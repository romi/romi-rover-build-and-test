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

#include "rover/EventsAndStates.h"

namespace romi {

        const char *event_name(EventID id)
        {
                switch (id) {
                case event_none:
                        return "event_none";
                case event_start:
                        return "event_start";
                case event_stop:
                        return "event_stop";
                case event_select:
                        return "event_select";
                case event_input_timeout:
                        return "event_input_timeout";
                case event_navigation_mode_pressed:
                        return "event_navigation_mode_pressed";
                case event_navigation_mode_released:
                        return "event_navigation_mode_released";
                case event_direction:
                        return "event_direction";
                case event_forward_speed:
                        return "event_forward_speed";
                case event_backward_speed:
                        return "event_backward_speed";
                case event_forward_start:
                        return "event_forward_start";
                case event_forward_stop:
                        return "event_forward_stop";
                case event_backward_start:
                        return "event_backward_start";
                case event_backward_stop:
                        return "event_backward_stop";
                case event_accurate_forward_start:
                        return "event_accurate_forward_start";
                case event_accurate_forward_stop:
                        return "event_accurate_forward_stop";
                case event_accurate_backward_start:
                        return "event_accurate_backward_start";
                case event_accurate_backward_stop:
                        return "event_accurate_backward_stop";
                case event_spinning_start:
                        return "event_spinning_start";
                case event_spinning_stop:
                        return "event_spinning_stop";
                case event_menu_mode_pressed:
                        return "event_menu_mode_pressed";
                case event_menu_mode_released:
                        return "event_menu_mode_released";
                case event_next_menu:
                        return "event_next_menu";
                case event_previous_menu:
                        return "event_previous_menu";
                case event_script_finished:
                        return "event_script_finished";
                case event_script_error:
                        return "event_script_error";
                default:
                        return "unknown event";
                }
        }

        const char *state_name(StateID id)
        {
                switch (id) {
                case state_ready:
                        return "state_ready";
                case state_stopping:
                        return "state_stopping";
                case state_waiting_navigate_confirmation_1:
                        return "state_waiting_navigate_confirmation_1";
                case state_waiting_navigate_confirmation_2:
                        return "state_waiting_navigate_confirmation_2";
                case state_ready_to_navigate:
                        return "state_ready_to_navigate";
                case state_moving_forward:
                        return "state_moving_forward";
                case state_moving_forward_accurately:
                        return "state_moving_forward_accurately";
                case state_moving_backward:
                        return "state_moving_backward";
                case state_moving_backward_accurately:
                        return "state_moving_backward_accurately";
                case state_spinning:
                        return "state_spinning";                
                case state_menu_mode_waiting_confirmation_1:
                        return "state_menu_mode_waiting_confircase mation_1:";
                case state_menu_mode_waiting_confirmation_2:
                        return "state_menu_mode_waiting_confirmation_2";
                case state_menu:
                        return "state_menu";
                case state_menu_selection_waiting_confirmation:
                        return "state_menu_selection_waiting_confirmation";
                case state_executing_script:
                        return "state_executing_script";
                case state_script_paused:
                        return "state_script_paused";
                default:
                        return "unkown state";
                }
        }
}
