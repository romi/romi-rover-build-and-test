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
#include "oquam.hpp"
#include <r.h>
#include "VirtualStepperController.hpp"
#include "OquamStepperController.hpp"
#include "plotter.h"

static mutex_t *mutex = NULL;
static oquam::Controller *controller = 0;
static int use_virtual_controller = 1;

static double xmax[3] = { 0.6, 0.6, -0.3 };
static double vm[3];

// XCarve: values taken from grbl/defaults.h.  
// 
// XY: 500.0*60*60 mm/min² = 500 mm/s² = 0.5 m/s²
// Z:  50.0*60*60 mm/min² = 500 mm/s² = 0.05 m/s²
//double amax[3] = { 0.5, 0.5, 0.05 };
static double amax[3] = { 0.3, 0.3, 0.03 };
static double scale[3];

static inline double sign(double v)
{
        return (v < 0)? -1.0 : 1.0;
}

int oquam_init(int argc, char **argv)
{
        double steps[3] = { 200.0, 200.0, 200.0 };
        double gears[3] = { 1.0, 1.0, 1.0 };
        double rpm[3] = { 300.0, 300.0, 300.0 };
        double displacement[3] = { 0.04, 0.04, -0.002 };
        double microsteps[3] = { 8.0, 8.0, 1.0 };
        
        double scale[3];

        for (int i = 0; i < 3; i++) {
                vm[i] = sign(displacement[i]) * displacement[i] * rpm[i] / 60.0 / gears[i];
                scale[i] = gears[i] * microsteps[i] * steps[i] / displacement[i];
        }

        /* vdiv(amax, amax, gears); */
        
        double period = 0.014;

        if (use_virtual_controller) 
                controller = new oquam::VirtualStepperController(xmax, vm, amax,
                                                                 0.001, scale, period);
        else
                controller = new oquam::OquamStepperController("/dev/ttyACM0",
                                                               xmax, vm, amax,
                                                               0.001, scale, period);
        if (controller == 0) {
                r_err("Failed to create the virtual controller");
                return -1;
        }

        mutex = new_mutex();
        
        return 0;
}

void oquam_cleanup()
{
        if (controller)
                delete controller;
        if (mutex)
                delete_mutex(mutex);
}

int oquam_onmoveto(void *userdata,
                   messagelink_t *link,
                   json_object_t command,
                   membuf_t *message)
{
	r_debug("handle_moveto");

        if (controller == 0) {
                membuf_printf(message, "CNC not initialized");
                return 0;
        }

        int hasx = json_object_has(command, "x");
        int hasy = json_object_has(command, "y");
        int hasz = json_object_has(command, "z");
        int hasv = json_object_has(command, "v");
        double x = -1.0, y = -1.0, z = -1.0, v = 0.1;
        
        if (!hasx && !hasy && !hasz) {
                membuf_printf(message, "Missing coordinates");
                return -1;
        }
        if (hasx)
                x = json_object_getnum(command, "x");
        if (hasy)
                y = json_object_getnum(command, "y");
        if (hasz)
                z = json_object_getnum(command, "z");
        if (hasv)
                v = json_object_getnum(command, "v");
        
        if (isnan(x) || isnan(y) || isnan(z) || isnan(v)) {
                membuf_printf(message, "Invalid coordinates or speed");
                return -1;
        }
        if (hasx && !controller->valid_x(x)) {
                membuf_printf(message, "X value out of range: %f", x);
                return -1;
        }
        if (hasy && !controller->valid_y(y)) {
                membuf_printf(message, "Y value out of range: %f", y);
                return -1;
        }
        if (hasz && !controller->valid_z(z)) {
                membuf_printf(message, "Z value out of range %f", z);
                return -1;
        }
        if (v < 0 || v > 1.0) {
                membuf_printf(message, "Speed out of range: %f", v);
                return -1;
        }

        controller->moveto(x, y, z, v, hasx, hasy, hasz);

        return 0;
}

int oquam_ontravel(void *userdata,
                   messagelink_t *link,
                   json_object_t command,
                   membuf_t *message)
{
	r_debug("oquam_ontravel 1");
        
        if (controller == 0) {
                membuf_printf(message, "CNC not initialized");
                r_warn("oquam_ontravel: CNC not initialized");
                return 0;
        }

        r_debug("oquam_ontravel 2");

        json_object_t path = json_object_get(command, "path");
        if (!json_isarray(path)) {
                membuf_printf(message, "Expected an array for the path");
                r_warn("oquam_ontravel: Expected an array for the path");
                return -1;
        }

        r_debug("oquam_ontravel 3: path");
        
        r_debug("oquam_ontravel 4");
        
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
        
        script_t *script = new_script();
        
        for (int i = 0; i < json_array_length(path); i++) {
                json_object_t p = json_array_get(path, i);
                double x = json_array_getnum(p, 0);
                double y = json_array_getnum(p, 1);
                double z = json_array_getnum(p, 2);

                double v = 0.1;
                if (json_array_length(p) == 4)
                        v = json_array_getnum(p, 3);

                if (x < 0 || x > 2.0) {
                        membuf_printf(message, "X value out of range [0,2]: %f", x);
                        err = -1;
                        break;
                }
                if (y < 0 || y > 2.0) {
                        membuf_printf(message, "Y value out of range [0,2]: %f", y);
                        err = -1;
                        break;
                }
                if (z > 0 || z < -2.0) {
                        membuf_printf(message, "Z value out of range [-2,0]: %f", z);
                        err = -1;
                        break;
                }
                if (v < 0 || v > 2.0) {
                        membuf_printf(message, "Speed out of range [0,2]: %f", v);
                        err = -1;
                        break;
                }

                script_moveto(script, x, y, z, v, i);
        }
        
        controller->run(script);

        char buffer[32000];
        if (json_tostring(path, buffer, sizeof(buffer)) == 0)
                r_debug("path: %s", buffer);

        json_print(path, k_json_pretty);
        printf("\n\npath: %s\n\n", buffer);


        print_to_stdout(script, xmax, vm, amax, scale);
        
        plot_to_file("oquam.svg", script,
                     xmax, vm, amax, scale);

        delete_script(script);
        
        return err;
}

int oquam_onspindle(void *userdata,
                    messagelink_t *link,
                    json_object_t command,
                    membuf_t *message)
{
	r_debug("Spindle event");
        return 0;
}

int oquam_onhoming(void *userdata,
                   messagelink_t *link,
                   json_object_t command,
                   membuf_t *message)
{
	r_debug("Homing");
        return 0;
}
