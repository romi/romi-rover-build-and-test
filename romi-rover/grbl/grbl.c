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
#include <romi.h>
#include "grbl.h"

static int _initialized = 0;
static char *_device = NULL;
static mutex_t *_mutex = NULL;
static serial_t *_serial = NULL;
//static int serial_errors = 0;
static membuf_t *_reply = NULL;

static cnc_range_t _range = {{0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}};


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

        mutex_lock(_mutex);
        _serial = new_serial(dev, 115200, 1);
        mutex_unlock(_mutex);

        if (_serial == NULL)
                r_info("Open failed.");
        else
                r_info("Serial connection opened.");

        return (_serial == NULL);
}

static void close_serial()
{
        mutex_lock(_mutex);
        if (_serial) delete_serial(_serial);
        _serial = NULL;
        _initialized = 0;
        mutex_unlock(_mutex);
}

static int send_command_unlocked(const char *cmd, membuf_t *message)
{
        int err = 0;
        const char *r;

        r = serial_command_send(_serial, message, cmd);
        r_debug("%s -> %s", cmd, membuf_data(message));

        // ToDo: This code is used in multiple places. Refactor.
        if (r == NULL) {
            membuf_printf(message, "Unknown error");
            return -1;
        } else if (strncmp(r, "ERR", 3) == 0) {
            return -1;
        }

        return err;
}

static int send_command(const char *cmd, membuf_t *message)
{
        int err;
        mutex_lock(_mutex);
        err = send_command_unlocked(cmd, message);
        mutex_unlock(_mutex);
        return err;
}

static int reset_cnc()
{
        r_debug("Resetting CNC");

	serial_println(_serial, "");
	serial_println(_serial, "");
	clock_sleep(2);
        serial_flush(_serial);

        r_debug("Homing");
        if (send_command("$H", _reply) != 0) {
                r_err("Homing failed");
                return -1;
        }

        r_debug("Making sure spindle is off");
        if (send_command("M5", _reply) != 0) {
                r_err("Spindle off failed");
                return -1;
        }

        r_debug("Setting origin");
        if (send_command("g92 x0 y0 z0", _reply) != 0) {
                r_err("g91 failed");
                return -1;
        }

        r_debug("Setting absolute positioning");
        if (send_command("g90", _reply) != 0) {
                r_err("g90 failed");
                return -1;
        }

        r_debug("Set units to millimeters");
        if (send_command("g21", _reply) != 0) {
                r_err("g21 failed");
                return -1;
        }
        r_debug("Resetting done");
        return 0;
}

static int set_device(const char *dev)
{
        close_serial();
        if (_device != NULL) {
                r_free(_device);
                _device = NULL;
        }
        _device = r_strdup(dev);
        if (_device == NULL) {
                r_err("Out of memory");
                return -1;
        }
        return 0;
}

static int get_device(json_object_t config)
{
        int err;
        if (_device == NULL) {
                // If device != NULL, it was given on the command
                // line. The command line has priority over the
                // configuration file.

                const char *device = json_object_getstr(config, "device");
                if (device == NULL) {
                        r_err("Missing value 'grbl.device' in configuration");
                        err = -1;
                } else {
                        err = set_device(device);
                }
        }
        return err;
}

static int get_range(json_object_t config)
{
        int err;
        json_object_t r = json_object_get(config, "range");
        if (!json_isarray(r)) {
                r_err("Invalid range in configuration");
                err = -1;
        } else {
                err = cnc_range_parse(&_range, r);
                r_info("range set to: x[%.3f,%.3f], y[%.3f,%.3f], z[%.3f,%.3f]",
                       _range.x[0], _range.x[1],
                       _range.y[0], _range.y[1],
                       _range.z[0], _range.z[1]);
        }
        return err;
}

static int get_configuration()
{
        int err;
        json_object_t config = client_get("configuration", "grbl");

        err = get_device(config);

        if (err == 0)
                err = get_range(config);

        json_unref(config);

        return err;
}

static int cnc_init()
{
        if (_initialized)
                return 0;

        if (get_configuration() != 0)
                return -1;

        if (open_serial(_device) != 0)
                return -1;

        if (reset_cnc() != 0)
                return -1;

        _initialized = 1;
        return 0;
}

int grbl_init(int argc, char **argv)
{
        r_log_set_writer(broadcast_log, NULL);

        _mutex = new_mutex();
        _reply = new_membuf();
        if (_mutex == NULL || _reply == NULL)
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
        delete_mutex(_mutex);
        delete_membuf(_reply);
}

static int grbl_idle()
{
        int ret = -1;
        const char *r;

        r_debug("CNC: put '?'");
        serial_lock(_serial);
        serial_put(_serial, '?');
        r = serial_readline(_serial, _reply);
        serial_unlock(_serial);

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

        serial_lock(_serial);
        serial_flush(_serial);
        serial_unlock(_serial);

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

        int hasx = json_object_has(command, "x");
        int hasy = json_object_has(command, "y");
        int hasz = json_object_has(command, "z");
        double x = 0.0, y = 0.0, z = 0.0;

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
        if (hasx && (x < _range.x[0] || x > _range.x[1])) {
                membuf_printf(message, "X value out of range [%.3f,%.3f]: %f",
                              _range.x[0], _range.x[1], x);
                return -1;
        }
        if (hasy && (y < _range.y[0] || y > _range.y[1])) {
                membuf_printf(message, "Y value out of range [%.3f,%.3f]: %f",
                              _range.y[0], _range.y[1], y);
                return -1;
        }
        if (hasz && (z < _range.z[0] || z > _range.z[1])) {
                membuf_printf(message, "Z value out of range [%.3f,%.3f]: %f",
                              _range.z[0], _range.z[1], z);
                return -1;
        }

        membuf_t *buf = new_membuf();
        membuf_printf(buf, "g0");
        // Convert meters to millimeters
        if (hasx) membuf_printf(buf, " x%d", (int) (x * 1000.0));
        if (hasy) membuf_printf(buf, " y%d", (int) (y * 1000.0));
        if (hasz) membuf_printf(buf, " z%d", (int) (z * 1000.0));
        membuf_append_zero(buf);

        mutex_lock(_mutex);

        int err = send_command_unlocked(membuf_data(buf), message);
        if (err == 0)
                wait_cnc();

        mutex_unlock(_mutex);
        delete_membuf(buf);

        return err;
}

static int path_is_valid_point(json_object_t path, int i, membuf_t *message)
{
        json_object_t p = json_array_get(path, i);
        if (!json_isarray(p) || json_array_length(p) < 3) {
                membuf_printf(message, "Point %d is not valid", i);
                return 0;
        }

        double v[3];
        for (int j = 0; j < 3; j++) {
                json_object_t n = json_array_get(p, j);
                if (!json_isnumber(n)) {
                        membuf_printf(message, "Coordinate (%d,%d) is not valid", i, j);
                        return 0;
                }
                v[j] = json_number_value(n);
        }
        return cnc_range_is_valid(&_range, v[0], v[1], v[2]);
}

static int path_is_valid(json_object_t path, membuf_t *message)
{
        if (!json_isarray(path)) {
                membuf_printf(message, "Expected an array for the path");
                return 0;
        }

        if (json_array_length(path) == 0) {
                membuf_printf(message, "Zero-length path");
                return 0;
        }

        if (0) { // Debug
                char buffer[2048];
                json_tostring(path, buffer, sizeof(buffer));
                r_debug("path: %s", buffer);
        }

        for (int i = 0; i < json_array_length(path); i++) {
                if (!path_is_valid_point(path, i, message)) {
                        membuf_printf(message, "Point %d is out of range", i);
                        return 0;
                }
        }

        return 1;
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
        if (!path_is_valid(path, message))
                return -1;

        int err = 0;
        membuf_t *buf = new_membuf();

        mutex_lock(_mutex);

        for (int i = 0; i < json_array_length(path); i++) {
                json_object_t p = json_array_get(path, i);
                double x = json_array_getnum(p, 0);
                double y = json_array_getnum(p, 1);
                double z = json_array_getnum(p, 2);
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

        mutex_unlock(_mutex);
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
