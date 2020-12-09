/*
  romi-rover

  Copyright (C) 2019-2020 Sony Computer Science Laboratories
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
#ifndef __ROMI_NAVIGATION_EVENTS_H
#define __ROMI_NAVIGATION_EVENTS_H

namespace romi {
        
        enum {
                ROVER_EVENT_START = 1,
                
                ROVER_EVENT_DIRECTION,
                ROVER_EVENT_FORWARD_SPEED,
                ROVER_EVENT_BACKWARD_SPEED,
                
                ROVER_EVENT_FORWARD_START,
                ROVER_EVENT_FORWARD_STOP,
                
                ROVER_EVENT_BACKWARD_START,
                ROVER_EVENT_BACKWARD_STOP,
                
                ROVER_EVENT_ACCURATE_FORWARD_START,
                ROVER_EVENT_ACCURATE_FORWARD_STOP,
                
                ROVER_EVENT_ACCURATE_BACKWARD_START,
                ROVER_EVENT_ACCURATE_BACKWARD_STOP,
                
                ROVER_EVENT_SPINNING_START,
                ROVER_EVENT_SPINNING_STOP,
        };

        enum {
                ROVER_STOPPED = 1,
                ROVER_MOVING_FORWARD,
                ROVER_MOVING_FORWARD_ACCURATELY,
                ROVER_MOVING_BACKWARD,
                ROVER_MOVING_BACKWARD_ACCURATELY,
                ROVER_SPINNING,
        };

}

#endif // __ROMI_NAVIGATION_EVENTS_H
