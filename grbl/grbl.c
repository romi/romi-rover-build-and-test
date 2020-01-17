/*
  romi-rover

  Copyright (C) 2019 Sony Computer Science Laboratories
  Author(s) Peter Hanappe

  romi-rover is collection of applications for the Romi Rover.

  romi-rover is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see
  <http://www.gnu.org/licenses/>.

 */
#include <r.h>
#include "grbl.h"

static int initialized = 0;
static char *device = NULL;
static mutex_t *mutex = NULL;
static serial_t *serial = NULL;
static int serial_errors = 0;
static membuf_t *reply = NULL;

static void broadcast_log(void *userdata, const char* s)
{
        messagelink_t *get_messagelink_logger();
        messagelink_t *log = get_messagelink_logger();
        if (log)
                messagelink_send_str(log, s);
}

static int open_serial(const char *dev)
{
        r_info("Trying to open the serial connection on %s.", dev);
	
        mutex_lock(mutex);        
        serial = new_serial(dev, 115200, 1);
        mutex_unlock(mutex);        
	
        if (serial == NULL)
                r_info("Open failed.");
        else 
                r_info("Serial connection opened.");

        return (serial == NULL);
}

static void close_serial()
{
        mutex_lock(mutex);        
        if (serial) delete_serial(serial);
        serial = NULL;
        initialized = 0;
        mutex_unlock(mutex);        
}

static int send_command_unlocked(const char *cmd, membuf_t *message)
{
        int err = 0;
        const char *r;

        r = serial_command_send(serial, message, cmd);
        r_debug("%s -> %s", cmd, membuf_data(message));

        // FIXME
        /* if (r == NULL) { */
        /*         err = -1; */
        /*         rprintf(message, len, "%s", "Unknown error"); */
        /* } else if (strncmp(r, "ERR", 3) == 0) { */
        /*         err = -1; */
        /* }  */
        
        return err;
}

static int send_command(const char *cmd, membuf_t *message)
{
        int err;
        mutex_lock(mutex);        
        err = send_command_unlocked(cmd, message);
        mutex_unlock(mutex);
        return err;
}

static int reset_cnc()
{
        r_debug("Resetting CNC");
	
	serial_println(serial, "");
	serial_println(serial, "");
	clock_sleep(2);
        serial_flush(serial);

        r_debug("Homing");
        if (send_command("$H", reply) != 0) {
                r_err("Homing failed");
                return -1;
        }

        r_debug("Making sure spindle is off");
        if (send_command("M5", reply) != 0) {
                r_err("Spindle off failed");
                return -1;
        }

        r_debug("Setting origin");
        if (send_command("g92 x0 y0 z0", reply) != 0) {
                r_err("g91 failed");
                return -1;
        }

        r_debug("Setting absolute positioning");
        if (send_command("g90", reply) != 0) {
                r_err("g90 failed");
                return -1;
        }

        r_debug("Set units to millimeters");
        if (send_command("g21", reply) != 0) {
                r_err("g21 failed");
                return -1;
        }
        r_debug("Resetting done");
        return 0;
}

static int set_device(const char *dev)
{
        close_serial();
        if (device != NULL) {
                r_free(device);
                device = NULL;
        }
        device = r_strdup(dev);
        if (device == NULL) {
                r_err("Out of memory");
                return -1;
        }
        return 0;
}

static int get_configuration()
{
        if (device != NULL)
                return 0;
        
        json_object_t dev = client_get("configuration", "grbl.device");
        if (!json_isstring(dev)) {
                r_err("the value of 'grbl.device' is not a string");
                json_unref(dev);
                return -1;
        }
        
        int err = set_device(json_string_value(dev));
        json_unref(dev);
        
        return err;
}

static int cnc_init()
{
        if (initialized)
                return 0;
        
        if (get_configuration() != 0)
                return -1;
        
        if (open_serial(device) != 0)
                return -1;
        
        if (reset_cnc() != 0)
                return -1;
        
        initialized = 1;
        return 0;
}

int grbl_init(int argc, char **argv)
{
        r_log_set_writer(broadcast_log, NULL);
        
        mutex = new_mutex();
        reply = new_membuf();
        if (mutex == NULL || reply == NULL)
                return -1;

        if (argc >= 2) {
                r_debug("using serial device given on command line: '%s'", argv[1]);
                if (set_device(argv[1]) != 0)
                        return -1;
                return 0;
        }

        for (int i = 0; i < 10; i++) {
                if (cnc_init() == 0)
                        return 0;
                r_err("cnc_init failed: attempt %d/10", i);
                clock_sleep(0.2);
        }

        r_err("failed to initialize the cnc");        
        return -1;
}

void grbl_cleanup()
{
        close_serial();
        if (mutex)
                delete_mutex(mutex);
        if (reply)
                delete_membuf(reply);
}

static int grbl_idle()
{
        int ret = -1;
        const char *r;
        
        r_debug("CNC: put '?'");
        serial_lock(serial);
        serial_put(serial, '?');
        r = serial_readline(serial, reply);
        serial_unlock(serial);
        
        if (r) r_debug("CNC status: '%s'", r);
        else r_debug("CNC status: NULL!");

        if (r == NULL)
                ret = -1;
        else if (r[0] != '<')
                ret = -1;
        else if (strlen(r) < 5)
                ret = -1;
        else if (strncmp(r+1, "Run", 3) == 0)
                ret = 0;
        else if (strncmp(r+1, "Idle", 4) == 0)
                ret = 1;
        else ret = 0;

        return ret;
}

static void wait_cnc()
{
        double start_time = clock_time();
        
        serial_lock(serial);
        serial_flush(serial);
        serial_unlock(serial);
        
        while (grbl_idle() != 1 && clock_time() - start_time < 600.0) {
                r_debug("Waiting for CNC to finish");
                clock_sleep(1);
        }
}

int cnc_onmoveto(void *userdata,
                 messagelink_t *link,
                 json_object_t command,
                 membuf_t *message)
{
	r_debug("handle_moveto");

        if (cnc_init() != 0) {
                membuf_printf(message, "CNC not initialized");
                return 0;
        }
        
        const char *r;
        int hasx = json_object_has(command, "x");
        int hasy = json_object_has(command, "y");
        int hasz = json_object_has(command, "z");
        double x = 0.0, y = 0.0, z = 0.0;
        char reply[64];
        
        if (!hasx && !hasy && !hasz) {
                membuf_printf(message, "No coordinates given");
                return -1;
        }
        if (hasx) x = json_object_getnum(command, "x");
        if (hasy) y = json_object_getnum(command, "y");
        if (hasz) z = json_object_getnum(command, "z");
        if (isnan(x) || isnan(y) || isnan(z)) {
                membuf_printf(message, "Invalid coordinates");
                return -1;
        }
        if (hasx && (x < 0 || x > 2.0)) {
                membuf_printf(message, "X value out of range [0,2]: %f", x);
                return -1;
        }
        if (hasy && (y < 0 || y > 2.0)) {
                membuf_printf(message, "Y value out of range [0,2]: %f", y);
                return -1;
        }
        if (hasz && (z > 0 || z < -2.0)) {
                membuf_printf(message, "Z value out of range [-2,0]: %f", z);
                return -1;
        }

        membuf_t *buf = new_membuf();
        membuf_printf(buf, "g0");
        // Convert meters to millimeters
        if (hasx) membuf_printf(buf, " x%d", (int) (x * 1000.0));
        if (hasy) membuf_printf(buf, " y%d", (int) (y * 1000.0));
        if (hasz) membuf_printf(buf, " z%d", (int) (z * 1000.0));
        membuf_append_zero(buf);

        mutex_lock(mutex);        
        
        int err = send_command_unlocked(membuf_data(buf), message);
        if (err == 0)
                wait_cnc();
        
        mutex_unlock(mutex);        
        delete_membuf(buf);

        return err;
}

int cnc_ontravel(void *userdata,
                 messagelink_t *link,
                 json_object_t command,
                 membuf_t *message)
{
	r_debug("cnc_ontravel");
        
        if (cnc_init() != 0) {
                membuf_printf(message, "CNC not initialized");
                return 0;
        }
        
        json_object_t path = json_object_get(command, "path");
        if (!json_isarray(path)) {
                membuf_printf(message, "Expected an array for the path");
                return -1;
        }

        char buffer[2048];
        json_tostring(path, buffer, sizeof(buffer));
        r_debug("path: %s", buffer);
        
        // Check the path
        for (int i = 0; i < json_array_length(path); i++) {
                json_object_t p = json_array_get(path, i);
                if (!json_isarray(p) || json_array_length(p) < 3) {
                        membuf_printf(message, "Point %d is not a valid", i);
                        return -1;
                }
                for (int j = 0; j < 3; j++) {
                        json_object_t v;
                        v = json_array_get(p, j);
                        if (!json_isnumber(v)) {
                                membuf_printf(message, "Coordinate (%d,%d) is not a valid", i, j);
                                return -1;
                        }
                        // TODO: check against CNC dimensions
                }
        }

        int err = 0;
        membuf_t *buf = new_membuf();
        
        mutex_lock(mutex);        
        
        for (int i = 0; i < json_array_length(path); i++) {
                json_object_t p = json_array_get(path, i);
                double x = json_array_getnum(p, 0);
                double y = json_array_getnum(p, 1);
                double z = json_array_getnum(p, 2);
                if (x < 0 || x > 2.0) {
                        membuf_printf(message, "X value out of range [0,2]: %f", x);
                        break;
                }
                if (y < 0 || y > 2.0) {
                        membuf_printf(message, "Y value out of range [0,2]: %f", y);
                        break;
                }
                if (z > 0 || z < -2.0) {
                        membuf_printf(message, "Z value out of range [-2,0]: %f", z);
                        break;
                }

                r_debug("point %d: %d %d %d", i, (int) (x * 1000.0), (int) (y * 1000.0), (int) (z * 1000.0));
                membuf_clear(buf);
                membuf_printf(buf, "g0 x%d y%d z%d", (int) (x * 1000.0), (int) (y * 1000.0), (int) (z * 1000.0));
                membuf_append_zero(buf);
                err = send_command_unlocked(membuf_data(buf), message);
                if (err) break;
                clock_sleep(1.0);
        }

        if (err == 0)
                wait_cnc();
        
        mutex_unlock(mutex);        
        delete_membuf(buf);

        return err;
}

int cnc_onspindle(void *userdata,
                  messagelink_t *link,
                  json_object_t command,
                  membuf_t *message)
{
	r_debug("Spindle event");

        if (cnc_init() != 0) {
                membuf_printf(message, "CNC not initialized");
                return 0;
        }
        
        double speed = json_object_getnum(command, "speed");
        if (isnan(speed) || speed < 0.0 || speed > 1.0) {
                membuf_printf(message, "Invalid speed");
                return -1;
        }

        const char *cmd = (speed == 0.0)? "M5" : "M3 S12000";
        return send_command(cmd, message);
}

int cnc_onhoming(void *userdata,
                 messagelink_t *link,
                 json_object_t command,
                 membuf_t *message)
{
	r_debug("Homing");

        if (cnc_init() != 0) {
                membuf_printf(message, "CNC not initialized");
                return 0;
        }
        return send_command("$H", message);
}
