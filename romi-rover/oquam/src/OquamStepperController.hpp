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
#ifndef _OQUAM_OQUAMSTEPPERCONTROLLER_HPP_
#define _OQUAM_OQUAMSTEPPERCONTROLLER_HPP_

#include <r.h>
#include "RomiSerialClient.h" 
#include "RSerial.h" 
#include "StepperController.hpp" 

namespace oquam {

        class OquamStepperController : public StepperController
        {
        public:
        
                OquamStepperController(const char *device,
                                       double *xmax, double *vmax, double *amax,
                                       double deviation, double *scale, double interval);
        
                virtual ~OquamStepperController();

                virtual int get_position(double *pos);
                virtual int execute(block_t *block);
                virtual int is_busy();
                virtual int moveat(double *v);
                virtual int moveto(double x, double y, double z, double v,
                                   int move_x = 1, int move_y = 1, int move_z = 1);
                virtual int continue_script();

        
        protected:

                int encode(block_t *block, char *buffer, int len);
                int send_command(const char *cmd);
                int update_status();

                RSerial *_serial;
                RomiSerialClient *_romi_serial;
                // membuf_t *_buffer;
                // serial_t *_serial;

                int _status;
                int _available;
                int _block_id;
                int _block_ms;
                int _milliseconds;
                int _interrupts;
                int _trigger;
                int32_t _stepper_position[3];
                int32_t _encoder_position[3];
                uint32_t _millis;
        };
}

#endif // _OQUAM_OQUAMSTEPPERCONTROLLER_HPP_
