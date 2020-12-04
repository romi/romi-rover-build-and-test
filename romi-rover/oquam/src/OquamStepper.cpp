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
#include "OquamStepper.h" 
#include "RomiSerialErrors.h" 

namespace romi {

        OquamStepper::OquamStepper(RomiSerialClient &romi_serial)
                : _romi_serial(romi_serial)
        {
                for (int i = 0; i < 3; i++) {
                        int r = update_status();
                        if (r == 0)
                                break;
                        clock_sleep(1.0);
                }
        }
        
        OquamStepper:: ~OquamStepper()
        {
        }

        int OquamStepper::send_command(const char *command)
        {
                int r = -1;
                json_object_t response;

                /* The number of loops is a bit random but it avoids
                 * an infinite loop. The loop will take at the most 10
                 * seconds to complete. This gives the firmware 10
                 * seconds to free a slot in the command buffer. */
                for (int i = 0; i < 1000; i++) {

                        //r_debug("OquamStepper::send_command: %s", command);
                        
                        response = _romi_serial.send(command);

                        // {
                        //         char buffer[256];
                        //         json_tostring(response, buffer, 256);
                        //         r_debug("OquamStepper::send_command: response: %s", buffer);
                        // }
                        
                        r = (int) json_array_getnum(response, 0);
                        
                        if (r == 0) {
                                json_unref(response);
                                break;
                                
                        } else if (r == 1) {
                                // Error code 1 indicates that the
                                // command buffer in the firmware is
                                // full. Wait a bit and try again.
                                json_unref(response);
                                clock_sleep(0.1);
                                
                        } else {
                                r_err("OquamStepper::execute: error: %s",
                                      json_array_getstr(response, 1));
                                json_unref(response);
                                break;
                        }
                }
                        
                return r;
        }

        bool OquamStepper::homing()
        {
                return (send_command("H") == 0 && synchronize(60.0));
        }

        bool OquamStepper::move(int16_t dt, int16_t dx, int16_t dy, int16_t dz, int16_t id)
        {
                char buffer[64];
                rprintf(buffer, 64, "M[%d,%d,%d,%d,%d]", dt, dx, dy, dz, id);
                return (send_command(buffer) == 0);
        }

        int OquamStepper::update_status()
        {
                // FIXME: RomiSerial should use JSON.h!
                json_object_t obj = _romi_serial.send("S");
                JSON s = JSON::moveto(obj);

                int r = (int) s.num(0);
                if (r == 0) {
                        if (s.length() == 14) {
                        
                                const char* status = s.str(1);
                                if (status == NULL) {
                                        r_err("OquamStepper::update_status: "
                                              "NULL status");
                                        r = -1;
                                } else if (rstreq(status, "r")) {
                                        _status = RUNNING;
                                } else if (rstreq(status, "h")) {
                                        _status = HOMING;
                                } else if (rstreq(status, "e")) {
                                        _status = ERROR;
                                } 

                                _available = (int) s.num(2);
                                _block_id = (int) s.num(3);
                                _block_ms = (int) s.num(4);
                                _milliseconds = (int) s.num(5);
                                _interrupts = (int) s.num(6);
                                _stepper_position[0] = (int32_t) s.num(7);
                                _stepper_position[1] = (int32_t) s.num(8);
                                _stepper_position[2] = (int32_t) s.num(9);
                                _encoder_position[0] = (int32_t) s.num(10);
                                _encoder_position[1] = (int32_t) s.num(11);
                                _encoder_position[2] = (int32_t) s.num(12);
                                _millis = (uint32_t) s.num(13);
                        } else {
                                r_err("OquamStepper::update_status: error: "
                                      "invalid array length");
                                r = -1;
                        }
                } else {
                        r_err("OquamStepper::update_status: error: %s", s.str(1));
                }
                
                return r;
        }

        bool OquamStepper::get_position(int32_t *pos)
        {
                bool success = false;
                if (update_status() == 0) {
                        pos[0] = _stepper_position[0];
                        pos[1] = _stepper_position[1];
                        pos[2] = _stepper_position[2];
                        success = true;
                }
                return success;
        }

        int OquamStepper::encode(block_t &block, char *buffer, int len)
        {
                int r = 0;
                
                switch (block.type) {
                case BLOCK_MOVE:
                        if (block.data[0] <= 0)
                                return 0;
                        if (block.data[1] == 0
                            && block.data[2] == 0
                            && block.data[3] == 0)
                                return 0;
                        rprintf(buffer, len, "M[%d,%d,%d,%d,%d]",
                                block.data[0],
                                block.data[1],
                                block.data[2],
                                block.data[3],
                                block.id);
                        break;
                default:
                        r_err("oquam_stepper_controller_send_block: Unknown block type");
                        r = -1;
                }
                
                return r;
        }

        bool OquamStepper::synchronize(double timeout)
        {
                bool success = false;
                double start_time = clock_time();
                
                while (true) {
                        
                        if (update_status() == 0) {
                                
                                if (_status == RUNNING
                                    && _available == 0
                                    && _block_id == -1) {
                                        success = true;
                                        break;
                                } else {
                                        clock_sleep(0.1);
                                }
                                
                        } else {
                                break;
                        }

                        double now = clock_time();
                        if (timeout >= 0.0 && (now - start_time) >= timeout)
                                break;
                }
                return success;
        }
}
