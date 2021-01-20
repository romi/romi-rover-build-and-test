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
#ifndef __ROMI_POWER_DEVICE_H
#define __ROMI_POWER_DEVICE_H

namespace romi {
        
        class PowerDevice
        {
        public:
                virtual ~PowerDevice() = default;

                // Do the initial power-up sequence once the power has
                // been supplied after a fresh reboot.
                virtual bool power_up() = 0;
                
                // Do the power-down sequence before shutting down.
                virtual bool power_down() = 0;

                // Go into a lower-power state due to inactivity.
                virtual bool stand_by() = 0;

                // Exit the lower-power state and prepare for action.
                virtual bool wake_up() = 0;
        };
}

#endif // __ROMI_POWER_DEVICE_H
