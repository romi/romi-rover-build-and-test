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
#ifndef __ROMI_ROVER_NAVIGATION_H
#define __ROMI_ROVER_NAVIGATION_H

#include <mutex>
#include "Navigation.h" 
#include "RoverConfiguration.h"
#include "MotorDriver.h"
#include "WheelOdometry.h" 

namespace romi {

        using SynchronizedCodeBlock = std::lock_guard<std::mutex>;

        class RoverNavigation : public Navigation
        {
        protected:

                enum { ROVER_MOVEAT_CAPABLE, ROVER_MOVING };
                
                MotorDriver &_driver;
                RoverConfiguration &_rover;
                std::mutex _mutex;
                int _status;
                int _stop;

                bool do_move(double distance, double speed);
                bool do_move2(double distance, double speed);
                bool wait_travel(WheelOdometry &odometry, double distance, double timeout);
                double compute_timeout(double distance, double speed);
                                        
        public:
                
                RoverNavigation(MotorDriver &driver, RoverConfiguration &rover) :
                        _driver(driver), _rover(rover), _status(ROVER_MOVEAT_CAPABLE) {
                }
                
                virtual ~RoverNavigation() override = default;

                bool moveat(double left, double right) override;
                bool move(double distance, double speed) override;
                bool stop() override;
                
                bool pause_activity() override;
                bool continue_activity() override;
                bool reset_activity() override;
        };
}

#endif // __ROMI_ROVER_NAVIGATION_H
