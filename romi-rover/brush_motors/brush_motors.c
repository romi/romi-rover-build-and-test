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
#include <string.h>
#include <r.h>
#include "brush_motors.h"

static char *device = NULL;
static mutex_t *mutex = NULL;
static serial_t *serial = NULL;
static membuf_t *encoders = NULL;
static membuf_t *status = NULL;

static int initialized = 0;
static double wheel_diameter = 0.0;
static double encoder_steps = 0.0;
static double max_speed = 0.0;
static double max_signal = 0.0;
static char with_pid = 0;
static char with_rc = 0;
static char with_homing = 0;
static double k[] = { 0.0, 0.0, 0.0 };

datahub_t *get_datahub_encoders();
messagehub_t *get_messagehub_motorstatus();

static void broadcast_log(void *userdata, const char* s)
{
        messagelink_t *get_messagelink_logger();
        static int _inside_log_writer = 0;
        messagelink_t *log = get_messagelink_logger();
        if (log != NULL && _inside_log_writer == 0) {
                _inside_log_writer = 1;
                messagelink_send_str(log, s);
                _inside_log_writer = 0;
        }
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
        mutex_unlock(mutex);        
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

static int send_command(const char *cmd, membuf_t *message)
{
        int err = 0;
        const char *r;

        if (cmd[0] != 'e' && cmd[0] != 'S') r_debug("send_command @1: cmd=%s", cmd);
        
        mutex_lock(mutex);        

        membuf_clear(message);
        
        if (cmd[0] != 'e' && cmd[0] != 'S') r_debug("send_command @2");
        
        if (serial == NULL) {
                membuf_printf(message, "No serial");
                err = -1;
                goto unlock;
        }        

        if (cmd[0] != 'e' && cmd[0] != 'S') r_debug("send_command @3");
        
        r = serial_command_send(serial, message, cmd);
        if (r == NULL) {
                err = -1;
                membuf_printf(message, "Unknown error");
        } else if (strncmp(r, "ERR", 3) == 0) {
                err = -1;
        } 

        if (cmd[0] != 'e' && cmd[0] != 'S') r_debug("send_command @4: r=%s", r);
        
unlock:
        mutex_unlock(mutex);

        if (cmd[0] != 'e' && cmd[0] != 'S') r_debug("send_command @5");
        
        return err;
}

static int reset_controller(serial_t *serial)
{
        char cmd[256];
        
        r_info("Resetting the motor controller.");

        membuf_t *reply = new_membuf();
        if (reply == NULL)
                return -1;
        
        if (send_command("X", reply) != 0) {
                r_err("Reset failed: %s", membuf_data(reply));
                return -1;
        }

        double max_rpm = max_speed / (M_PI * wheel_diameter);
        double steps_per_revolution = encoder_steps;
        rprintf(cmd, sizeof(cmd), "I[%d,%d,%d,%d,%d,%d,%d,%d,%d]",
                (int) steps_per_revolution,
                (int) (max_rpm * 100.0),
                (int) max_signal,
                with_pid? 1 : 0,
                with_pid? (int) (k[0] * 1000.0) : 0,
                with_pid? (int) (k[1] * 1000.0) : 0,
                with_pid? (int) (k[2] * 1000.0) : 0,
                with_rc? 1 : 0,
                with_homing? 1 : 0);
        if (send_command(cmd, reply) != 0) {
                r_err("Initialization failed: %s", membuf_data(reply));
                return -1;
        }
        
        if (send_command("E1", reply) != 0) {
                r_err("Enable failed: %s", membuf_data(reply));
                return -1;
        }

        delete_membuf(reply);
        return 0;
}

int get_rover_configuration()
{
        json_object_t rover = client_get("configuration", "rover");
        if (!json_isobject(rover)) {
                r_err("unexpected value for 'rover'");
                json_unref(rover);
                return -1;
        }

        double diam = json_object_getnum(rover, "wheel_diameter");
        double steps = json_object_getnum(rover, "encoder_steps");
        if (isnan(diam) || isnan(steps)) {
                r_err("invalid wheel diameter or encoder steps");
                json_unref(rover);
                return -1;
        }
        if (diam < 0.02 || diam > 2.0) {
                r_err("invalid wheel diameter: %f !in [0.02,2.00]", diam);
                json_unref(rover);
                return -1;
        }
        if (steps < 100 || steps > 1000000) {
                r_err("invalid encoder steps: %f !in [100,1000000]", steps);
                json_unref(rover);
                return -1;
        }
        wheel_diameter = diam;
        encoder_steps = steps;

        r_info("wheel_diameter %f", wheel_diameter);
        r_info("encoder_steps  %f", encoder_steps);
        json_unref(rover);
        return 0;
}

int get_controller_configuration()
{
        json_object_t controller = client_get("configuration", "brush_motors");
        if (!json_isobject(controller)) {
                r_err("unexpected value for 'brush_motors'");
                json_unref(controller);
                return -1;
        }

        double v = json_object_getnum(controller, "max_speed");
        double sig = json_object_getnum(controller, "max_signal");
        int pid = json_object_getbool(controller, "pid");
        int rc = json_object_getbool(controller, "rc");
        int h = json_object_getbool(controller, "homing");
        json_object_t ka = json_object_get(controller, "k");

        if (isnan(v) || isnan(sig) || pid == -1
            || rc == -1 || h == -1 || !json_isarray(ka)) {
                r_err("invalid controller settings");
                json_unref(controller);
                return -1;
        }

        double kp = json_array_getnum(ka, 0);
        double ki = json_array_getnum(ka, 1);
        double kd = json_array_getnum(ka, 2);
        if (isnan(kp) || isnan(ki) || isnan(kd)
            || kp < 0.0 || kp > 100.0
            || ki < 0.0 || ki > 100.0
            || kd < 0.0 || kd > 100.0) {
                r_err("invalid PID settings");
                json_unref(controller);
                return -1;
        }
        json_unref(controller);

        if (device == NULL) {
                const char *dev = NULL;
                json_object_t port = client_get("configuration", "ports/brush_motors/port");
                if (!json_isstring(port)) {
                    r_err("missing value for 'ports' in the configuration");
                    json_unref(port);
                    return -1;
                } else {
                    dev = json_string_value(port);
                }

                if (set_device(dev) != 0) {
                        json_unref(port);
                        return -1;
                }
            json_unref(port);
        }


        max_speed = v;
        max_signal = sig;
        with_pid = pid;
        with_rc = rc;
        with_homing = h;
        k[0] = kp;
        k[1] = ki;
        k[2] = kd;

        r_info("max_speed      %.1f", max_speed);
        r_info("max_signal     %.0f", max_signal);
        r_info("with_pid       %s", with_pid? "yes" : "no");
        r_info("with_rc        %s", with_rc? "yes" : "no");
        r_info("with_homing    %s", with_homing? "yes" : "no");
        r_info("PID            [%.4f,%.4f,%.4f]", k[0], k[1], k[2]);

        return 0;
}

int get_configuration()
{
        if (get_rover_configuration() != 0)
                return -1;
        if (get_controller_configuration() != 0)
                return -1;
        return 0;
}

int motorcontroller_init()
{
        if (initialized)
                return 0;
        
        if (get_configuration() != 0)
                return -1;

        if (open_serial(device) != 0)
                return -1;
        
        if (reset_controller(serial) != 0) {
                close_serial();
                return -1;
        }
        
        initialized = 1;
        return 0;
}

int brush_motors_init(int argc, char **argv)
{
        r_log_set_writer(broadcast_log, NULL);
        
        mutex = new_mutex();
        encoders = new_membuf();
        status = new_membuf();

        if (argc <= 1 ) {
            r_debug("no arguements");
                return -1;
        }

        if (argc > 1) {
                r_debug("using serial device '%s'", argv[1]);
                if (set_device(argv[1]) != 0)
                        return -1;
        }

        for (int i = 0; i < 10; i++) {
                if (motorcontroller_init() == 0)
                        return 0;
                r_err("motorcontroller_init failed: attempt %d/10", i);
                clock_sleep(0.2);
        }

        r_err("failed to initialize the motorcontroller");
        return -1;
}

void brush_motors_cleanup()
{
        close_serial();
        delete_mutex(mutex);
        delete_membuf(status);
        delete_membuf(encoders);
        r_free(device);
}

int motorcontroller_onmoveat(void *userdata,
                             messagelink_t *link,
                             json_object_t command,
                             membuf_t *message)
{
        r_debug("motorcontroller_onmoveat");

        if (motorcontroller_init() != 0) {
                membuf_printf(message, "Motor controller not initialized");
                return -1;
        }
        
        json_object_t speed = json_object_get(command, "speed");
        double left, right;
        if (json_isarray(speed)) {
                left = json_array_getnum(speed, 0);
                right = json_array_getnum(speed, 1);
        } else if (json_isnumber(speed)) {
                left = json_number_value(speed);
                right = left;
        } else {
            membuf_printf(message, "Missing or invalid speed value");
            return -1;
        }
        
        if (isnan(left) || isnan(right)) {
                r_warn("invalid left|right values: %f, %f", left, right);
                membuf_printf(message, "Invalid left|right values");
                return -1;
        }

        if (left < -1000.0 || left > 1000.0
            || right < -1000.0 || right > 1000.0) {
                r_warn("Speed is out of bounds [-1000, 1000].");
                membuf_printf(message, "Speed is out of bounds [-1000, 1000].");
                return -1;
        }

        char cmd[64];
        rprintf(cmd, sizeof(cmd), "M[%d,%d]", (int) left, (int) right);

        return send_command(cmd, message);
}

int motorcontroller_onenable(void *userdata,
                             messagelink_t *link,
                             json_object_t command,
                             membuf_t *message)
{
        r_debug("motorcontroller_onenable.");

        if (motorcontroller_init() != 0) {
                membuf_printf(message, "Motor controller not initialized");
                return -1;
        }
        
        double value = json_object_getnum(command, "value");
        if (isnan(value)) {
                r_warn("missing value for enable");
                membuf_printf(message, "missing value for enable");
                return -1;
        }
        return send_command((value == 0)? "E0" : "E1", message);
}

int motorcontroller_onreset(void *userdata,
                            messagelink_t *link,
                            json_object_t command,
                            membuf_t *message)
{
        r_info("Resetting the motor controller.");
        if (motorcontroller_init() != 0) {
                membuf_printf(message, "Motor controller not initialized");
                return -1;
        }        
        return send_command("X", message);
}

int motorcontroller_onhoming(void *userdata,
                             messagelink_t *link,
                             json_object_t command,
                             membuf_t *message)
{
	r_debug("Homing");
        return send_command("H", message);
}

int broadcast_encoders(void *userdata, datahub_t* hub)
{
        static double offset = 0.0;
                              
        if (motorcontroller_init() != 0) {
                clock_sleep(0.2);
                return -1;
        }
        
        if (send_command("e", encoders) != 0) {
                r_err("brush_motors: 'e' command returned: %s",
                        membuf_data(encoders));
                return -1;
        }
                
        int encL, encR;
        unsigned int millis;
        const char *r = membuf_data(encoders);
        sscanf(r, "e[%d,%d,%u]", &encL, &encR, &millis);

        double boottime = millis / 1000.0;
        if (offset == 0.0) {
                double now = clock_time();
                offset = now - boottime;
        }

        //r_debug("broadcast_encoders: %d, %d", encL, encR);
        
        datahub_broadcast_f(get_datahub_encoders(), NULL, 
                            "{\"encoders\":[%d,%d], \"timestamp\": %f}",
                            encL, encR, offset + boottime);
        clock_sleep(0.100);

	return 0;
}

void broadcast_status()
{
        if (motorcontroller_init() != 0) {
                clock_sleep(1);
                return;
        }

        if (send_command("S", encoders) != 0) {
                r_err("brush_motors: 'S' command returned: %s",
                        membuf_data(encoders));
                return;
        }

        const char *r = membuf_data(encoders);
        messagehub_broadcast_f(get_messagehub_motorstatus(), NULL, r + 1);
        clock_sleep(1);
}

void watchdog_onmessage(void *userdata,
                        messagelink_t *link,
                        json_object_t message)
{
        const char *r = json_object_getstr(message, "request");
        if (r == NULL)
                r_warn("watchdog_onmessage: invalid message: empty request");
        else if (rstreq(r, "ready?"))
                messagelink_send_f(link,
                                   "{\"request\": \"set-ready\", "
                                   "\"name\": \"brush_motors\"}");
}
