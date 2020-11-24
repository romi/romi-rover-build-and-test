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
#include "OquamStepperController.hpp" 
#include "RomiSerialErrors.h" 

namespace oquam {

        enum {
                CONTROLLER_IDLE,
                CONTROLLER_EXECUTING,
                CONTROLLER_TRIGGER
        };

        OquamStepperController::OquamStepperController(const char *device,
                                                       double *xmax, double *vmax,
                                                       double *amax, double deviation,
                                                       double *scale, double interval)
                : StepperController(xmax, vmax, amax, deviation, scale, interval)
        {
                _serial = new RSerial(device, 115200, 1);
                _romi_serial = new RomiSerialClient(_serial, _serial);

                clock_sleep(2.0f);
                for (int i = 0; i < 3; i++) {
                        int r = update_status();
                        if (r == 0)
                                break;
                }
        }
        
        OquamStepperController:: ~OquamStepperController()
        {
                if (_romi_serial)
                        delete _romi_serial;
                if (_serial)
                        delete _serial;
        }

        int OquamStepperController::send_command(const char *command)
        {
                int r = -1;
                json_object_t response;

                /* The number of loops is a bit random but it avoids
                 * an infinite loop. The loop will take at the most 10
                 * seconds to complete. This gives the firmware 10
                 * seconds to free a slot in the command buffer. */
                for (int i = 0; i < 100; i++) {
                        
                        response = _romi_serial->send(command);

                        r = (int) json_array_getnum(response, 0);
                        if (r == 0) {
                                break;
                                
                        } else if (r == 1) {
                                // Error code 1 indicates that the
                                // command buffer in the firmware is
                                // full. Wait a bit and try again.
                                json_unref(response);
                                clock_sleep(0.1);
                                
                        } else {
                                r_err("OquamStepperController::execute: error: %s",
                                      json_array_getstr(response, 1));
                                break;
                        }
                }
                
                json_unref(response);
        
                return r;
        }

        int OquamStepperController::update_status()
        {
                json_object_t s = _romi_serial->send("S");

                int r = (int) json_array_getnum(s, 0);
                if (r == 0) {
                        if (json_array_length(s) == 15) {
                        
                                const char* status = json_array_getstr(s, 1);
                                if (status == NULL) {
                                        r_err("OquamStepperController::update_status: "
                                              "NULL status");
                                        r = -1;
                                } else if (rstreq(status, "e")) {
                                        _status = CONTROLLER_EXECUTING;
                                } else if (rstreq(status, "i")) {
                                        _status = CONTROLLER_IDLE;
                                } else if (rstreq(status, "t")) {
                                        _status = CONTROLLER_TRIGGER;
                                }

                                _available = (int) json_array_getnum(s, 2);
                                _block_id = (int) json_array_getnum(s, 3);
                                _block_ms = (int) json_array_getnum(s, 4);
                                _milliseconds = (int) json_array_getnum(s, 5);
                                _interrupts = (int) json_array_getnum(s, 6);
                                _trigger = (int) json_array_getnum(s, 7);
                                _stepper_position[0] = (int32_t) json_array_getnum(s, 8);
                                _stepper_position[1] = (int32_t) json_array_getnum(s, 9);
                                _stepper_position[2] = (int32_t) json_array_getnum(s, 10);
                                _encoder_position[0] = (int32_t) json_array_getnum(s, 11);
                                _encoder_position[1] = (int32_t) json_array_getnum(s, 12);
                                _encoder_position[2] = (int32_t) json_array_getnum(s, 13);
                                _millis = (uint32_t) json_array_getnum(s, 14);

                                // if (controller->available > 0
                                //     || controller->block_id >= 0) {
                                //         r_debug("Block %d, %d blocks avail., "
                                //                 "block dur. %d, block ms. %d, "
                                //                 "pos. [%d,%d,%d]s=[%.3f,%.3f,%.3f]m",
                                //                 controller->block_id,
                                //                 controller->available,
                                //                 controller->block_ms,
                                //                 controller->milliseconds,
                                //                 controller->stepper_position[0],
                                //                 controller->stepper_position[1],
                                //                 controller->stepper_position[2],
                                //                 (double) controller->stepper_position[0] / controller->stepper.scale[0],
                                //                 (double) controller->stepper_position[1] / controller->stepper.scale[1],
                                //                 (double) controller->stepper_position[2] / controller->stepper.scale[2]);
                                // }
                        } else {
                                r_err("OquamStepperController::update_status: error: "
                                      "invalid array length");
                        }
                } else {
                        r_err("OquamStepperController::update_status: error: %s",
                              json_array_getstr(s, 1));
                }

                json_unref(s);
                
                return r;
        }

        int OquamStepperController::get_position(double *pos)
        {
                if (update_status() != 0)
                        return -1;
                pos[0] = (double) _stepper_position[0] / _scale[0];
                pos[1] = (double) _stepper_position[1] / _scale[1];
                pos[2] = (double) _stepper_position[2] / _scale[2];
                return 0;
        }

        int OquamStepperController::encode(block_t *block, char *buffer, int len)
        {
                int r = 0;
                
                switch (block->type) {
                case BLOCK_WAIT:
                        rprintf(buffer, len, "W");
                        break;
                case BLOCK_MOVE:
                        if (block->data[0] <= 0)
                                return 0;
                        if (block->data[1] == 0
                            && block->data[2] == 0
                            && block->data[3] == 0)
                                return 0;
                        rprintf(buffer, len, "M[%d,%d,%d,%d,%d]",
                                block->data[0],
                                block->data[1],
                                block->data[2],
                                block->data[3],
                                block->id);
                        break;
                case BLOCK_DELAY:
                        if (block->data[0] <= 0)
                                return 0;
                        rprintf(buffer, len, "D%d", block->data[0]);
                        break;
                case BLOCK_TRIGGER:
                        rprintf(buffer, len, "T[%d,%d]",
                                block->data[0], block->data[1]);
                        break;
                default:
                        r_err("oquam_stepper_controller_send_block: Unknown block type");
                        r = -1;
                }
                
                return r;
        }
        
        int OquamStepperController::execute(block_t *block)
        {
                int r = -1;
                char cmd[64];
        
                if (encode(block, cmd, sizeof(cmd)) == 0) {
                        r = send_command(cmd);
                }

                return r;
        }

        int OquamStepperController::is_busy()
        {
                if (update_status() != 0)
                        return -1;
                return (_available != 0 || _block_id != -1);
        }

        int OquamStepperController::moveat(double *v)
        {
                if (_script) { // lock
                        r_err("OquamStepperController::moveto: running a script");
                        return -1;
                }
                char cmd[64];
                rprintf(cmd, sizeof(cmd), "V[%d,%d,%d,0]",
                        (int) (v[0] * _scale[0]),
                        (int) (v[2] * _scale[1]),
                        (int) (v[2] * _scale[2]));
                return send_command(cmd);
        }

        int OquamStepperController::moveto(double x, double y, double z, double v,
                                           int move_x, int move_y, int move_z)
        {
                if (_script) { // lock
                        r_err("OquamStepperController::moveto: running a script");
                        return -1;
                }
                if (move_x && !valid_x(x)){
                        r_err("VirtualStepperController::moveto: x out of boundary");
                        return -1;
                }
                if (move_y && !valid_y(y)){
                        r_err("VirtualStepperController::moveto: y out of boundary");
                        return -1;
                }
                if (move_z && !valid_z(z)){
                        r_err("VirtualStepperController::moveto: z out of boundary");
                        return -1;
                }
                if (v < 0 || v > 1.0) {
                        r_err("OquamStepperController::moveto: speed out of boundary");
                        return -1;
                }
        
                double vs[3]; // speed in steps/s
                vmul(vs, _scale, _vmax);
                smul(vs, vs, v); // v in [0,1]
                int v_steps = vmin(vs); 
        
                int xi = move_x? (int) (x * _scale[0]) : -1;
                int yi = move_y? (int) (y * _scale[1]) : -1;
                int zi = move_z? (int) (z * _scale[2]) : -1;
        
                char cmd[64];
                rprintf(cmd, sizeof(cmd), "m[%d,%d,%d,%d,0]", v_steps, xi, yi, zi);

                return send_command(cmd);
        }

        int OquamStepperController::continue_script()
        {
                return send_command("C");
        }
}
