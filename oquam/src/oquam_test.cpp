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
#include "script.h"
#include "VirtualStepperController.hpp"
#include "OquamStepperController.hpp"
#include "plotter.h"
#include "v.h"
 
int use_virtual_controller = 0;

static void trigger(void *userdata, int16_t arg)
{
        printf("*** trigger %d ***\n", (int) arg);
}

static inline double sign(double v)
{
        return (v < 0)? -1.0 : 1.0;
}

int main(int argc, char **argv)
{
        Controller *controller;
        
        r_init(0);

        double xmax[3] = { 0.6, 0.6, -0.3 };
        double steps[3] = { 200.0, 200.0, 200.0 };
        double gears[3] = { 1.0, 1.0, 1.0 };
        double displacement[3] = { 0.04, 0.04, -0.002 };
        // double gears[3] = { 15.0, 15.0, 15.0 };
        // double displacement[3] = { 0.6, 0.6, -0.02 };
        double microsteps[3] = { 8.0, 8.0, 1.0 };
        double rpm[3] = { 150.0, 150.0, 150.0 };
        
        double vmax[3];
        double scale[3];

        for (int i = 0; i < 3; i++) {
                vmax[i] = sign(displacement[i]) * displacement[i] * rpm[i] / 60.0 / gears[i];
                scale[i] = gears[i] * microsteps[i] * steps[i] / displacement[i];
        }

        // XCarve: values taken from grbl/defaults.h.  
        // 
        // XY: 500.0*60*60 mm/min² = 500 mm/s² = 0.5 m/s²
        // Z:  50.0*60*60 mm/min² = 500 mm/s² = 0.05 m/s²
        //double amax[3] = { 0.5, 0.5, 0.05 };
        double amax[3] = { 0.3, 0.3, 0.03 };

        /* vdiv(amax, amax, gears); */
        
        double period = 0.014;
        double deviation = 0.010;

        printf("xmax[%.3f,%.3f,%.3f]\n", xmax[0], xmax[1], xmax[2]);
        printf("vmax[%.3f,%.3f,%.3f]\n", vmax[0], vmax[1], vmax[2]);
        printf("amax[%.3f,%.3f,%.3f]\n", amax[0], amax[1], amax[2]);
        printf("scale[%.3f,%.3f,%.3f]\n", scale[0], scale[1], scale[2]);
        
        if (use_virtual_controller) 
                controller = new VirtualStepperController(xmax, vmax, amax, deviation,
                                                          scale, period);
        else
                controller = new OquamStepperController("/dev/ttyACM0",
                                                        xmax, vmax, amax, deviation,
                                                        scale, period);
        if (controller == NULL) {
                r_err("Failed to create the virtual controller");
                return 1;
        }

        ///
        
        if (0) {
                script_t *script = new_script();
                double xc = xmax[0] / 2.0;
                double yc = xmax[1] / 2.0;
                script_moveto(script, xc, yc, 0.0, 0.1, 0);
                script_moveto(script, 2 * xc, 0.0, 0.0, 0.1, 1);
                controller->run(script);
                delete_script(script);
        }
        

        ///
        
        if (0) {
                script_t *script = new_script();
                double dx = 0.2 * xmax[0];
                double dy = 0.2 * xmax[1];
                script_moveto(script, dx, 0.0, 0.0, 0.1, 0);
                script_moveto(script, dx,  dy, 0.0, 0.1, 1);
                script_moveto(script, 0.0, dy, 0.0, 0.1, 2);
                controller->run(script);
                delete_script(script);
        }
        
        ///

        // Circle around the center of the workspace
        if (0) {
                script_t *script = new_script();
                double xc = xmax[0] / 2.0;
                double yc = xmax[1] / 2.0;
                double r = xc * 0.6;
                int segments = 36;
                //printf("xc=%f, yc=%f, r=%f\n", xc, yc, r);
                script_moveto(script, 0, 0, 0, 0.15, 0);
                for (int i = 0; i < segments; i++) {
                        double angle = 2.0 * M_PI * i / segments;
                        double x = xc - r * cosf(angle);
                        double y = xc + r * sinf(angle);
                        script_moveto(script, x, y, 0, 0.15, i + 1);
                        //printf("%f, %f\n", x, y);
                        script_trigger(script, trigger, NULL, i, 0.0);
                }
                script_moveto(script, 0, 0, 0, 0.15, segments + 1);
                script_trigger(script, trigger, NULL, 36, 0.0);
                controller->run(script);                

                plot_to_file("oquam_test.svg", script,
                             xmax, vmax, amax, scale);

                
                delete_script(script);
        }

        
        ///
                
        if (0) {
                script_t *script = new_script();
                double dx = 0.2 * xmax[0];
                double dy = 0.2 * xmax[1];
                script_moveto(script, dx, 0.0, 0.0, 0.1, 0);
                script_moveto(script, dx,  dy, 0.0, 0.1, 1);
                script_moveto(script, 0.0, dy, 0.0, 0.1, 2);
                //script_moveto(script, 0.0, 0.0, 0.0, 0.1);
                controller->run(script);                
                delete_script(script);
        }

        ///
                
        if (0) {
                script_t *script = new_script();
                for (int i = 1; i < 4; i++) {
                        script_moveto(script, i * 0.1, 0.0, 0.0, 0.1, 4 * i);
                        script_moveto(script, i * 0.1, i * 0.1, 0.0, 0.1, 4 * i + 1);
                        script_moveto(script, 0.0, i * 0.1, 0.0, 0.1, 4 * i + 2);
                        script_moveto(script, 0.0, 0.0, 0.0, 0.1, 4 * i + 3);
                }
                controller->run(script);
                delete_script(script);
        }

        
        ///
        
        if (0) {
                script_t *script = new_script();
                for (int i = 1; i < 7; i++) {
                        script_moveto(script, i * 0.1, i * 0.1, 0.0, 0.07, 2 * i - 1);
                        script_moveto(script, 0.0, 0.0, 0.0, 0.07, 2 * i);
                }
                controller->run(script);
                delete_script(script);
        }

        ///
                
        if (0) {
                script_t *script = new_script();
                double xc = xmax[0] / 2.0;
                double yc = xmax[1] / 2.0;
                script_moveto(script, xc, yc, 0.0, 0.1, 0);
                script_moveto(script, 2 * xc, 0.0, 0.0, 0.1, 1);
                controller->run(script);
                delete_script(script);
        }

        if (0) {
                script_t *script = new_script();
                double xc = xmax[0] / 2.0;
                double zc = xmax[2] / 10.0;
                double vx = vmax[0] / 2.0;
                double vz = vmax[2];
                script_moveto(script,  xc, 0.0, 0.0, vx, 0);
                script_moveto(script,  xc, 0.0,  zc, vz, 1);
                script_moveto(script, 0.0, 0.0,  zc, vx, 2);
                script_moveto(script, 0.0, 0.0, 0.0, vz, 3);
                controller->run(script);

                plot_to_file("oquam_test.svg", script,
                             xmax, vmax, amax, scale);
                
                delete_script(script);
        }

        if (1) {
                double p[][3] = {{0, 0.025000, -0.1},
                                 {0, 0.066100, -0.1},
                                 {0.047500, 0.060400, -0.1},
                                 {0.092200, 0.077400, -0.1},
                                 {0.123900, 0.113200, -0.1},
                                 {0.135400, 0.159600, -0.1},
                                 {0.123900, 0.206100, -0.1},
                                 {0.092200, 0.241900, -0.1},
                                 {0.047500, 0.258900, -0.1},
                                 {0, 0.253200, -0.1},
                                 {0, 0.365900, -0.1},
                                 {0.047300, 0.360500, -0.1},
                                 {0.091800, 0.377600, -0.1},
                                 {0.123300, 0.413400, -0.1},
                                 {0.134600, 0.459700, -0.1},
                                 {0.123300, 0.505900, -0.1},
                                 {0.091800, 0.541700, -0.1},
                                 {0.047300, 0.558900, -0.1},
                                 {0, 0.553500, -0.1},
                                 {0, 0.575000, -0.1},
                                 {0.050000, 0.575000, -0.1},
                                 {0.050000, 0.558500, -0.1},
                                 {0.093300, 0.540700, -0.1},
                                 {0.123700, 0.505100, -0.1},
                                 {0.134600, 0.459700, -0.1},
                                 {0.123700, 0.414200, -0.1},
                                 {0.093300, 0.378700, -0.1},
                                 {0.050000, 0.360900, -0.1},
                                 {0.050000, 0.258600, -0.1},
                                 {0.093600, 0.240900, -0.1},
                                 {0.124300, 0.205300, -0.1},
                                 {0.135400, 0.159600, -0.1},
                                 {0.124300, 0.113900, -0.1},
                                 {0.093600, 0.078300, -0.1},
                                 {0.050000, 0.060700, -0.1},
                                 {0.050000, 0.025000, -0.1},
                                 {0.100000, 0.025000, -0.1},
                                 {0.100000, 0.083300, -0.1},
                                 {0.126100, 0.117600, -0.1},
                                 {0.135400, 0.159600, -0.1},
                                 {0.126100, 0.201700, -0.1},
                                 {0.100000, 0.235900, -0.1},
                                 {0.100000, 0.384000, -0.1},
                                 {0.125600, 0.418100, -0.1},
                                 {0.134600, 0.459700, -0.1},
                                 {0.125600, 0.501300, -0.1},
                                 {0.100000, 0.535300, -0.1},
                                 {0.100000, 0.575000, -0.1},
                                 {0.150000, 0.575000, -0.1},
                                 {0.150000, 0.316700, -0.1},
                                 {0.143700, 0.281800, -0.1},
                                 {0.150000, 0.246800, -0.1},
                                 {0.150000, 0.073900, -0.1},
                                 {0.150000, 0.073900, -0.1},
                                 {0.171800, 0.097700, -0.1},
                                 {0.200000, 0.113300, -0.1},
                                 {0.200000, 0.191800, -0.1},
                                 {0.165400, 0.219600, -0.1},
                                 {0.146200, 0.259600, -0.1},
                                 {0.146200, 0.303900, -0.1},
                                 {0.165400, 0.344000, -0.1},
                                 {0.200000, 0.371700, -0.1},
                                 {0.200000, 0.500900, -0.1},
                                 {0.200000, 0.500900, -0.1},
                                 {0.223700, 0.488100, -0.1},
                                 {0.250000, 0.482100, -0.1},
                                 {0.250000, 0.381600, -0.1},
                                 {0.297300, 0.366200, -0.1},
                                 {0.331300, 0.329900, -0.1},
                                 {0.343700, 0.281800, -0.1},
                                 {0.331300, 0.233600, -0.1},
                                 {0.297300, 0.197300, -0.1},
                                 {0.250000, 0.182000, -0.1},
                                 {0.250000, 0.117800, -0.1},
                                 {0.276700, 0.109500, -0.1},
                                 {0.300000, 0.094000, -0.1},
                                 {0.300000, 0.199100, -0.1},
                                 {0.332100, 0.235000, -0.1},
                                 {0.343700, 0.281800, -0.1},
                                 {0.332100, 0.328500, -0.1},
                                 {0.300000, 0.364400, -0.1},
                                 {0.300000, 0.490600, -0.1},
                                 {0.300000, 0.490600, -0.1},
                                 {0.329600, 0.511000, -0.1},
                                 {0.350000, 0.540500, -0.1},
                                 {0.350000, 0.178600, -0.1},
                                 {0.343200, 0.142200, -0.1},
                                 {0.350000, 0.105800, -0.1},
                                 {0.350000, 0.025000, -0.1},
                                 {0.400000, 0.025000, -0.1},
                                 {0.400000, 0.052000, -0.1},
                                 {0.365100, 0.079700, -0.1},
                                 {0.345700, 0.119900, -0.1},
                                 {0.345700, 0.164500, -0.1},
                                 {0.365100, 0.204700, -0.1},
                                 {0.400000, 0.232400, -0.1},
                                 {0.400000, 0.372900, -0.1},
                                 {0.366400, 0.409100, -0.1},
                                 {0.354200, 0.456900, -0.1},
                                 {0.366400, 0.504800, -0.1},
                                 {0.400000, 0.541000, -0.1},
                                 {0.400000, 0.575000, -0.1},
                                 {0.450000, 0.575000, -0.1},
                                 {0.450000, 0.556800, -0.1},
                                 {0.401800, 0.542100, -0.1},
                                 {0.366900, 0.505700, -0.1},
                                 {0.354200, 0.456900, -0.1},
                                 {0.366900, 0.408200, -0.1},
                                 {0.401800, 0.371800, -0.1},
                                 {0.450000, 0.357000, -0.1},
                                 {0.450000, 0.242000, -0.1},
                                 {0.497100, 0.226400, -0.1},
                                 {0.530900, 0.190200, -0.1},
                                 {0.543200, 0.142200, -0.1},
                                 {0.530900, 0.094200, -0.1},
                                 {0.497100, 0.058000, -0.1},
                                 {0.450000, 0.042400, -0.1},
                                 {0.450000, 0.025000, -0.1},
                                 {0.500000, 0.025000, -0.1},
                                 {0.500000, 0.059900, -0.1},
                                 {0.531700, 0.095700, -0.1},
                                 {0.543200, 0.142200, -0.1},
                                 {0.531700, 0.188600, -0.1},
                                 {0.500000, 0.224500, -0.1},
                                 {0.500000, 0.368000, -0.1},
                                 {0.533400, 0.395900, -0.1},
                                 {0.551800, 0.435200, -0.1},
                                 {0.551800, 0.478700, -0.1},
                                 {0.533400, 0.518000, -0.1},
                                 {0.500000, 0.545800, -0.1},
                                 {0.500000, 0.575000, -0.1},
                                 {0.550000, 0.575000, -0.1},
                                 {0.550000, 0.485700, -0.1},
                                 {0.554200, 0.456900, -0.1},
                                 {0.550000, 0.428200, -0.1},
                                 {0.550000, 0.025000, -0.1},
                                 {0.600000, 0.025000, -0.1},
                                 {0.600000, 0.575000, -0.1}};

                int n = sizeof(p) / (3 * sizeof(double));
                
                controller->moveto(0, 0, -0.1, 0.1);
                
                script_t *script = new_script();
                
                for (int i = 0; i < n; i++)
                        script_moveto(script, p[i][0], p[i][1], p[i][2], 0.15, i);

                controller->run(script);                

                plot_to_file("oquam_test.svg", script,
                             xmax, vmax, amax, scale);
                
                delete_script(script);
        }
        
        delete controller;
        
        r_cleanup();
        return 0;
}
