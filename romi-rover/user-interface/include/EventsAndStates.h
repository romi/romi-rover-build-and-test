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

        enum AxisID {
                axis_direction = 0,          // left stick
                axis_backward_speed = 2,     // L2
                axis_forward_speed = 5,      // R2
                axis_menu_navigation = 7,    // up/down buttons
                axis_last = 8,
        };
        
        enum ButtonID {
                button_select = 0,            // cross (X)
                button_navigation_mode = 1,   // circle (O)
                button_menu_mode = 2,         // triangle (/\)
                button_stop = 3,              // square ([])
                button_accurate_backward = 4, // L1
                button_accurate_forward = 5,  // R1
                button_backward_mode = 6,     // L2
                button_forward_mode = 7,      // R2
                button_spin_mode = 11,        // left stick
                button_last = 12,
        };

        enum EventID {
                event_none = 0,
                
                event_start = 1,
                event_stop,
                event_select,
                
                event_input_timeout,
                event_timer_timeout,
                
                event_navigation_mode_pressed,
                event_navigation_mode_released,
                
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

                event_menu_mode_pressed,
                event_menu_mode_released,

                event_next_menu,
                event_previous_menu,
                
                event_script_finished,
                event_script_error,
        };

        enum StateID {
                state_ready = 1,
                state_stopping,
                
                state_waiting_navigate_confirmation_1,
                state_waiting_navigate_confirmation_2,
                state_ready_to_navigate,
                state_moving_forward,
                state_moving_forward_accurately,
                state_moving_backward,
                state_moving_backward_accurately,
                state_spinning,
                
                state_menu_mode_waiting_confirmation_1,
                state_menu_mode_waiting_confirmation_2,
                state_menu,
                state_menu_selection_waiting_confirmation,

                state_executing_script,
        };

}

#endif // __ROMI_EVENTS_AND_STATES_H
