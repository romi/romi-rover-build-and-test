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
#include "VirtualStepperController.hpp" 

namespace oquam {

        int VirtualStepperController::get_position(double *pos)
        {
                pos[0] = (double) _stepper_position[0] / _scale[0];
                pos[1] = (double) _stepper_position[1] / _scale[1];
                pos[2] = (double) _stepper_position[2] / _scale[2];
                return 0;
        }

        int VirtualStepperController::delay()
        {
                double t0 = clock_time();
                while (1) {
                        double t = clock_time();
                        if (t - t0 > _delay)
                                break;
                        clock_sleep(0.1);
                }
                return 0;
        }

        int VirtualStepperController::execute(block_t *block)
        {
                double dt;
        
                switch (block->type) {
                case BLOCK_WAIT:
                        printf("W\n");
                        _delay = 1e10;
                        delay();
                        break;
                        
                case BLOCK_MOVE:
                        printf("M[%d,%d,%d,%d]\n",
                               block->data[0],
                               block->data[1],
                               block->data[2],
                               block->data[3]);
                        dt = (double) block->data[0] / 1000.0;
                        clock_sleep(dt);
                        _stepper_position[0] += block->data[1];
                        _stepper_position[1] += block->data[2];
                        _stepper_position[2] += block->data[3];
                        break;
                        
                case BLOCK_DELAY:
                        printf("D%d\n", block->data[0]);
                        _delay = (double) block->data[0] / 1000.0;
                        delay();
                        break;
                        
                case BLOCK_TRIGGER:
                        printf("T[%d,%d]\n", block->data[0], block->data[1]);
                        _delay = (double) block->data[1] / 1000.0;
                        script_do_trigger(_script, block->data[0]);
                        delay();
                        break;
                        
                default:
                        printf("ERR Unknown block type\n");
                        break;
                }
        
                return 0;
        }

        int VirtualStepperController::is_busy()
        {
                return 0;
        }

        int VirtualStepperController::moveat(double *v)
        {
                if (_script) { // lock
                        r_err("VirtualStepperController::moveto: running a script");
                        return -1;
                }
                return -1; // TODO: implement...
        }

        int VirtualStepperController::moveto(double x, double y, double z, double v,
                                             int move_x, int move_y, int move_z)
        {
                if (_script) { // lock
                        r_err("VirtualStepperController::moveto: running a script");
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
                        r_err("VirtualStepperController::moveto: speed out of boundary");
                        return -1;
                }
                
                if (move_x)
                        _stepper_position[0] = x * _scale[0];        
                if (move_y)
                        _stepper_position[1] = y * _scale[1];        
                if (move_z)
                        _stepper_position[2] = z * _scale[2];        
                return 0;
        }

        int VirtualStepperController::continue_script()
        {
                _delay = 0.0;
                return 0;
        }
}
