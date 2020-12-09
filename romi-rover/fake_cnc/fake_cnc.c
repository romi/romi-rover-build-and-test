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
#include "fake_cnc.h"

static int _initialized = 0;
static char *_device = NULL;
static mutex_t *_mutex = NULL;

static double _x = 0.0;
static double _y = 0.0;
static double _z = 0.0;

static cnc_range_t _range = {{0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}};

static void broadcast_log(void *userdata, const char* s)
{
        messagelink_t *get_messagelink_logger();
        messagelink_t *log = get_messagelink_logger();
        if (log)
                messagelink_send_str(log, s);
}

////////////////////////////////////////////

static int open_serial(const char *dev)
{
        r_debug("Open serial %s", dev);
        return 0;
}

static void close_serial()
{
        r_debug("Close serial");
}

//////////////////////////////////////////////

static int set_spindle_speed(double speed, membuf_t *message)
{
        return 0;
}

static int do_homing(membuf_t *message)
{
        return 0;
}

static int reset_cnc()
{
        _x = 0.0;
        _y = 0.0;
        _z = 0.0;
        return 0;
}

static void wait_cnc()
{
        // Nothing to do
}

static int moveto(int hasx, int hasy, int hasz,
                  double x, double y, double z,
                  membuf_t *message)
{
        if (hasx)
                _x = x;
        if (hasy)
                _y = y;
        if (hasz)
                _z = z;
	r_debug("position[%.4f,%.4f,%.4f]", _x, _y, _z);
        return 0;
}

//////////////////////////////////////////////

static int set_device(const char *dev)
{
        close_serial();
        if (_device != NULL) {
                r_free(_device);
                _device = NULL;
        }
        _device = r_strdup(dev);
        return 0;
}

static int get_device()
{
        return 0;
}

static int get_range()
{
        int err = 0;
        
        json_object_t config = client_get("configuration", "cnc");
        if (json_isobject(config)) {
                
                json_object_t r = json_object_get(config, "range");
                if (!json_isarray(r)) {
                        r_err("Invalid range in configuration");
                        err = -1;
                } else {
                        err = cnc_range_parse(&_range, r);
                        if (err == 0) {
                                r_info("range set to: x[%.3f,%.3f], y[%.3f,%.3f], z[%.3f,%.3f]",
                                       _range.x[0], _range.x[1],
                                       _range.y[0], _range.y[1],
                                       _range.z[0], _range.z[1]);
                        } // else: error message alreay printed by cnc_range_parse()
                }
        } else {
                r_err("Couldn't find CNC configuration");
                err = -1;
        }
        json_unref(config);
        return err;
}

static int get_configuration()
{
        int err;

        err = get_device();

        if (err == 0)
                err = get_range();


        return err;
}

static int try_cnc_init()
{
        if (_initialized)
                return 0;

        if (get_configuration() != 0)
                return -1;
        
        if (open_serial(_device) != 0)
                return -1;

        if (reset_cnc() != 0) {
                close_serial();
                return -1;
        }

        _initialized = 1;
        return 0;
}

static int cnc_init()
{
        int r = -1;
        for (int i = 0; i < 10; i++) {
                if (try_cnc_init() == 0) {
                        r = 0;
                        break;
                }
                r_err("cnc_init failed: attempt %d/10", i);
                clock_sleep(0.5);
        }
        return r;
}

int fake_cnc_init(int argc, char **argv)
{
        int r = 0;
        
        r_log_set_writer(broadcast_log, NULL);
        
        _mutex = new_mutex();

        if (argc >= 2) {
                r_debug("using serial device given on command line: '%s'", argv[1]);
                if (set_device(argv[1]) != 0)
                        r = -1;
        }

        if (r == 0)
                r = cnc_init();
        
        return r;
}

void fake_cnc_cleanup()
{
        close_serial();
        delete_mutex(_mutex);
}

static int cnc_move(int hasx, int hasy, int hasz,
                    double x, double y, double z,
                    membuf_t *message)
{
        int r;
        mutex_lock(_mutex); 
        r = moveto(hasx, hasy, hasz, x, y, z, message);
        mutex_unlock(_mutex);
        return r;
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
        if (hasx)
                x = json_object_getnum(command, "x");
        if (hasy)
                y = json_object_getnum(command, "y");
        if (hasz)
                z = json_object_getnum(command, "z");
        
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

        int err = cnc_move(hasx, hasy, hasz, x, y, z, message);
        if (err == 0)
                wait_cnc();

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
        
        int valid = cnc_range_is_valid(&_range, v[0], v[1], v[2]);
        if (!valid)
                membuf_printf(message, "Point %d is out of range: (%.3f, %.3f, %.3f)",
                              i, v[0], v[1], v[2]);
        return valid;
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
                if (!path_is_valid_point(path, i, message))
                        return 0;
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
        
        mutex_lock(_mutex);        
        
        for (int i = 0; i < json_array_length(path); i++) {
                json_object_t p = json_array_get(path, i);
                double x = json_array_getnum(p, 0);
                double y = json_array_getnum(p, 1);
                double z = json_array_getnum(p, 2);
                r_debug("point %d: %d %d %d", i, (int) (x * 1000.0), (int) (y * 1000.0), (int) (z * 1000.0));
                err = moveto(1, 1, 1, x, y, z, message);
                if (err)
                        break;
                clock_sleep(0.5);
        }
        
        mutex_unlock(_mutex);        

        if (err == 0)
                wait_cnc();

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

        return set_spindle_speed(speed, message);
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
        return do_homing(message);
}
