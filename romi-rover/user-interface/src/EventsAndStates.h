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
#ifndef __ROMI_EVENTS_AND_STATES_H
#define __ROMI_EVENTS_AND_STATES_H

namespace romi {

        // The joystick axes and buttons that are used.
        enum {
                axis_direction = 0,          // left stick
                axis_backward_speed = 2,     // l2
                axis_forward_speed = 5,      // r2
                axis_last = 6,               // l2
                
                button_accurate_backward = 4, // l1
                button_accurate_forward = 5,  // r1
                button_backward_mode = 6,     // l2
                button_forward_mode = 7,      // r2
                button_spin_mode = 11,        // left stick
                button_last = 12,
        };

        // the navigation events
        enum {
                event_none = 0,
                
                event_start = 1,
                
                event_direction,
                event_forward_speed,
                event_backward_speed,
                
                event_forward_start,
                event_forward_stop,
                
                event_backward_start,
                event_backward_stop,
                
                event_accurate_forward_start,
                event_accurate_forward_stop,
                
                event_accurate_backward_start,
                event_accurate_backward_stop,
                
                event_spinning_start,
                event_spinning_stop,
        };

        // the navigation state
        enum {
                state_stopped = 1,
                state_moving_forward,
                state_moving_forward_accurately,
                state_moving_backward,
                state_moving_backward_accurately,
                state_spinning,
        };

}

#endif // __ROMI_EVENTS_AND_STATES_H
