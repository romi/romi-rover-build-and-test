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
#include <stdexcept>
#include "JSON.h" 
#include "StepperController.h" 
#include "RomiSerialErrors.h" 

namespace romi {

        int StepperController::send_command(const char *command)
        {
                int r = -1;
                json_object_t response;

                /* The number of loops is a bit random but it avoids
                 * an infinite loop. The loop will take at the most 10
                 * seconds to complete. This gives the firmware 10
                 * seconds to free a slot in the command buffer. */
                for (int i = 0; i < 1000; i++) {

                        //r_debug("StepperController::send_command: %s", command);
                        
                        response = _romi_serial.send(command);

                        if (json_isarray(response)
                            && json_isnumber(json_array_get(response, 0))) {
                                
                                r = (int) json_array_getnum(response, 0);
                                
                                        
                                if (r == 0) {
                                        json_unref(response);
                                        break;
                                
                                } else if (r == 1) {
                                        // Error code 1 indicates that the
                                        // command buffer in the firmware is
                                        // full. Wait a bit and try again.
                                        json_unref(response);
                                        clock_sleep(0.2);
                                                
                                } else {
                                        r_err("StepperController::execute: "
                                              "error: %s",
                                              json_array_getstr(response, 1));
                                        json_unref(response);
                                        break;
                                }
                                
                        } else {
                                char buffer[256];
                                json_tostring(response, buffer, 256);
                                r_debug("StepperController::send_command: "
                                        "invalid response: %s", buffer);
                        }
                }
                        
                return r;
        }

        bool StepperController::homing()
        {
                return (send_command("H") == 0 && synchronize(60.0));
        }

        bool StepperController::move(int16_t dt, int16_t dx, int16_t dy, int16_t dz)
        {
                char buffer[64];
                rprintf(buffer, 64, "M[%d,%d,%d,%d]", dt, dx, dy, dz);
                return (send_command(buffer) == 0);
        }

        int StepperController::is_idle()
        {
                int idle = -1;
                int state = '?';
                
                // FIXME: RomiSerial should use JSON.h!
                json_object_t obj = _romi_serial.send("I");
                JSON s = JSON::moveto(obj);

                int r = (int) s.num(0);
                if (r == 0) {
                        if (s.length() == 3) {
                                // This is the answer to "is idle?".
                                idle = (int) s.num(1);

                                const char *t = s.str(2);
                                state = t[0];

                                // TODO
                                // If the driver is in an error state,
                                // report it.
                                if (state == 'e') {
                                        // FIXME: what shall we do?
                                        r_err("StepperController::is_idle: "
                                              "Firmware in error state");
                                        idle = -1;
                                }
                                
                        } else {
                                r_err("StepperController::is_idle: error: "
                                      "invalid array length");
                        }
                        
                } else {
                        r_err("StepperController::is_idle: error: %s", s.str(1));
                }
                
                return idle;
        }
        
        bool StepperController::get_position(int32_t *pos)
        {
                bool success = false;
                
                // FIXME: RomiSerial should use JSON.h!
                json_object_t obj = _romi_serial.send("P");
                JSON s = JSON::moveto(obj);

                int r = (int) s.num(0);
                if (r == 0) {
                        if (s.length() == 4) {
                                pos[0] = (int32_t) s.num(1);
                                pos[1] = (int32_t) s.num(2);
                                pos[2] = (int32_t) s.num(3);
                                success = true;
                        } else {
                                r_err("StepperController::get_position: error: "
                                      "invalid array length");
                                r = -1;
                        }
                } else {
                        r_err("StepperController::get_position: error: %s", s.str(1));
                }
                
                return success;
        }

        bool StepperController::synchronize(double timeout)
        {
                bool success = false;
                double start_time = clock_time();
                
                while (true) {

                        int idle = is_idle();
                        
                        if (idle == 1) {
                                success = true;
                                break;
                        } else if (idle == -1) {
                                break;
                        } else {
                                clock_sleep(0.2);
                        }

                        double now = clock_time();
                        if (timeout >= 0.0 && (now - start_time) >= timeout)
                                break;
                }
                return success;
        }

        bool StepperController::stop_execution()
        {
                return (send_command("p") == 0);
        }
        
        bool StepperController::continue_execution()
        {
                return (send_command("c") == 0);
        }
        
        bool StepperController::reset()
        {
                return homing();
        }
}
