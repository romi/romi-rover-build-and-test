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
#ifndef __OQUAM_OQUAM_STEPPER_H_
#define __OQUAM_OQUAM_STEPPER_H

#include <r.h>
#include "RomiSerialClient.h" 
#include "RSerial.h" 
#include "IBlockController.h" 

namespace romi {

        class OquamStepper : public IBlockController
        {
        public:
        
                OquamStepper(RomiSerialClient &romi_serial);
                virtual ~OquamStepper();

                bool get_position(int32_t *pos) override;
                bool homing() override;
                
                bool move(int16_t millis, int16_t steps_x,
                          int16_t steps_y, int16_t steps_z,
                          int16_t id = 0) override;
                
                bool synchronize(double timeout) override;
        
        protected:

                int encode(block_t &block, char *buffer, int len);
                int send_command(const char *cmd);
                int update_status();

                RomiSerialClient &_romi_serial;

                int _status;
                int _available;
                int _block_id;
                int _block_ms;
                int _milliseconds;
                int _interrupts;
                int32_t _stepper_position[3];
                int32_t _encoder_position[3];
                uint32_t _millis;
        };
}

#endif // __OQUAM_OQUAM_STEPPER_H
