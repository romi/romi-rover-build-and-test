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
#include <RomiSerialClient.h> 
#include <RSerial.h>

#include "oquam/CNCController.h" 
#include "ActivityHelper.h" 


namespace romi {
                
        class StepperController : public CNCController
        {
        protected:


                RomiSerialClient &_romi_serial;
                std::mutex _mutex;
                std::condition_variable _continue_condition;
                ActivityHelper _activity_helper;
                
        public:
                
                static constexpr const char *ClassName = "stepper-controller";
        
                StepperController(RomiSerialClient &romi_serial)
                        : _romi_serial(romi_serial) {}
                
                virtual ~StepperController() override = default;

                bool configure_homing(AxisIndex axis1, AxisIndex axis2,
                                      AxisIndex axis3) override;
                bool get_position(int32_t *pos) override;
                bool homing() override;
                
                bool move(int16_t millis, int16_t steps_x,
                          int16_t steps_y, int16_t steps_z) override;
                
                bool synchronize(double timeout) override;

                bool pause_activity() override;
                bool continue_activity() override;
                bool reset_activity() override;
                
                bool enable() override;
                bool disable() override;

        protected:

                int send_command(const char *cmd);
                int is_idle();                
                bool response_ok(JsonCpp& response);
                bool send_command_without_interruption(const char *command);
        };
}

#endif // __OQUAM_OQUAM_STEPPER_H
