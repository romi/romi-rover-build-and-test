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
#include "UserInterface.h"
#include "EventsAndStates.h"
#include "UIStateTransitions.h"

namespace romi {
        
        UserInterface::UserInterface(InputDevice &input_device_,
                                     Display &display_,
                                     SpeedController &speed_controller_,
                                     EventTimer &event_timer_,
                                     Menu &menu_,
                                     ScriptEngine& script_engine_)
                : input_device(input_device_),
                  display(display_),
                  speed_controller(speed_controller_),
                  event_timer(event_timer_),
                  menu(menu_),
                  script_engine(script_engine_),
                  state_machine(*this)
        {
                init_state_machine();
                state_machine.handle_event(event_start);
        }

        void UserInterface::handle_events()
        {
                handle_input_events();
                handle_timer_events();
                handle_script_events();
        }
        
        void UserInterface::handle_input_events()
        {
                while (true) {
                        int event = input_device.get_next_event();
                        if (event == 0)
                                break;
                        state_machine.handle_event(event);
                }
        }
        
        void UserInterface::handle_timer_events()
        {
                int event = event_timer.get_next_event();
                if (event != 0)
                        state_machine.handle_event(event);
        }

        void UserInterface::handle_script_events()
        {
                int event = script_engine.get_next_event();
                if (event != 0)
                        state_machine.handle_event(event);
        }

        void UserInterface::init_state_machine()
        {
                init_start_transition();
                init_navigation_transitions();
                init_menu_transitions();
        }

        void UserInterface::init_start_transition()
        {
                state_machine.add(STATE_START,
                                  event_start,
                                  initialize_rover,
                                  state_ready);
        }

        void UserInterface::init_stop_transitions()
        {
                state_machine.add(state_stopping,
                                  event_stop,
                                  reset_rover,
                                  state_ready);
                
                state_machine.add(state_stopping,
                                  event_select,
                                  continue_rover,
                                  saved_state);

                state_machine.add(state_stopping,
                                  event_timer_timeout,
                                  reset_rover,
                                  state_ready);
        }
        
        void UserInterface::init_navigation_transitions()
        {
                init_navigation_mode_transition();
                init_forward_driving_transitions();
                init_accurate_forward_driving_transitions();
                init_backward_driving_transitions();
                init_accurate_backward_driving_transitions();
                init_spinning_transitions();
        }

        void UserInterface::init_navigation_mode_transition()
        {
                // The user pressed the "O" button. In the first step,
                // she must hold the button for a certain time to
                // confirm entering the navigation. This avoids
                // spurious buttons presses.
                state_machine.add(state_ready,
                                  event_navigation_mode_pressed,
                                  confirm_navigation_step_1,
                                  state_waiting_navigate_confirmation_1);

                // The event timer sends the confirmation event when
                // the button is held long enough.
                state_machine.add(state_waiting_navigate_confirmation_1,
                                  event_timer_timeout,
                                  confirm_navigation_step_2,
                                  state_waiting_navigate_confirmation_2);
                
                // If the button is released before the timeout, the
                // "released" event will bring the state machine back
                // to the main state.
                state_machine.add(state_waiting_navigate_confirmation_1,
                                  event_navigation_mode_released,
                                  leave_navigation_mode,
                                  state_ready);

                // Once the button has been held for long enough,
                // there is a second confirmation. In the second step,
                // the user must confirm entering the navigation mode
                // by pressing the X/select button.
                state_machine.add(state_waiting_navigate_confirmation_2,
                                  event_select,
                                  initialize_navigation,
                                  state_ready_to_navigate);

                // There's a timeout also for the second confirmation
                // step. If the user doesn't press confirm/select
                // within the given time, the state goes back to the
                // main state.
                state_machine.add(state_waiting_navigate_confirmation_2,
                                  event_timer_timeout,
                                  leave_navigation_mode,
                                  state_ready);

                // In any case, when the user interface is in the
                // navigation mode, when no user inputs have been
                // received for a given time, the input device
                // generates a "input delay passed" event. In this
                // case, the user interface quits the navigation mode
                // and returns to the main mode.
                state_machine.add(state_ready_to_navigate,
                                  event_timer_timeout,
                                  leave_navigation_mode,
                                  state_ready);
                
                state_machine.add(state_ready_to_navigate,
                                  event_stop,
                                  leave_navigation_mode,
                                  state_ready);                
        }
        
        void UserInterface::init_forward_driving_transitions()
        {
                state_machine.add(state_ready_to_navigate,
                                  event_forward_start,
                                  start_driving_forward,
                                  state_moving_forward);
                
                state_machine.add(state_moving_forward,
                                  event_forward_speed,
                                  drive_forward,
                                  state_moving_forward);
                
                state_machine.add(state_moving_forward,
                                  event_direction,
                                  drive_forward,
                                  state_moving_forward);

                state_machine.add(state_moving_forward,
                                  event_forward_stop,
                                  stop_driving,
                                  state_ready_to_navigate);
        }
        
        void UserInterface::init_accurate_forward_driving_transitions()
        {
                state_machine.add(state_ready_to_navigate,
                                  event_accurate_forward_start,
                                  start_driving_forward_accurately,
                                  state_moving_forward_accurately);
                
                state_machine.add(state_moving_forward_accurately,
                                  event_forward_speed,
                                  drive_forward_accurately,
                                  state_moving_forward_accurately);
                
                state_machine.add(state_moving_forward_accurately,
                                  event_direction,
                                  drive_forward_accurately,
                                  state_moving_forward_accurately);

                state_machine.add(state_moving_forward_accurately,
                                  event_accurate_forward_stop,
                                  stop_driving,
                                  state_ready_to_navigate);
        }
        
        void UserInterface::init_backward_driving_transitions()
        {
                state_machine.add(state_ready_to_navigate,
                                  event_backward_start,
                                  start_driving_backward,
                                  state_moving_backward);
                
                state_machine.add(state_moving_backward,
                                  event_backward_speed,
                                  drive_backward,
                                  state_moving_backward);
                
                state_machine.add(state_moving_backward,
                                  event_direction,
                                  drive_backward,
                                  state_moving_backward);
                
                state_machine.add(state_moving_backward,
                                  event_backward_stop,
                                  stop_driving,
                                  state_ready_to_navigate);
        }
        
        void UserInterface::init_accurate_backward_driving_transitions()
        {
                state_machine.add(state_ready_to_navigate,
                                  event_accurate_backward_start,
                                  start_driving_backward_accurately,
                                  state_moving_backward_accurately);
                
                state_machine.add(state_moving_backward_accurately,
                                  event_backward_speed,
                                  drive_backward_accurately,
                                  state_moving_backward_accurately);
                
                state_machine.add(state_moving_backward_accurately,
                                  event_direction,
                                  drive_backward_accurately,
                                  state_moving_backward_accurately);

                state_machine.add(state_moving_backward_accurately,
                                  event_accurate_backward_stop,
                                  stop_driving,
                                  state_ready_to_navigate);
        }
        
        void UserInterface::init_spinning_transitions()
        {
                state_machine.add(state_ready_to_navigate,
                                  event_spinning_start,
                                  start_spinning,
                                  state_spinning);
                 
                state_machine.add(state_spinning,
                                  event_direction,
                                  spin,
                                  state_spinning);

                state_machine.add(state_spinning,
                                  event_spinning_stop,
                                  stop_driving,
                                  state_ready_to_navigate);
        }

        
        void UserInterface::init_menu_transitions()
        {
                init_menu_mode_transition();
                init_menu_selection_transitions();
        }

        void UserInterface::init_menu_mode_transition()
        {
                // For an explanation, check the comments in
                // init_navigation_mode_transition(). To enter the
                // menu mode, the steps are similar. The "O" button is
                // replaced with the "triangle" button.
                state_machine.add(state_ready,
                                  event_menu_mode_pressed,
                                  confirm_menu_step_1,
                                  state_menu_mode_waiting_confirmation_1);
                
                state_machine.add(state_menu_mode_waiting_confirmation_1,
                                  event_timer_timeout,
                                  confirm_menu_step_2,
                                  state_menu_mode_waiting_confirmation_2);
                
                state_machine.add(state_menu_mode_waiting_confirmation_1,
                                  event_menu_mode_released,
                                  leave_menu_mode,
                                  state_ready);
                
                state_machine.add(state_menu_mode_waiting_confirmation_2,
                                  event_select,
                                  open_menu,
                                  state_menu);
                
                state_machine.add(state_menu_mode_waiting_confirmation_2,
                                  event_timer_timeout,
                                  leave_menu_mode,
                                  state_ready);
                
                state_machine.add(state_menu,
                                  event_timer_timeout,
                                  leave_menu_mode,
                                  state_ready);                

                state_machine.add(state_menu,
                                  event_stop,
                                  leave_menu_mode,
                                  state_ready);                
        }
        
        void UserInterface::init_menu_selection_transitions()
        {
                state_machine.add(state_menu,
                                  event_next_menu,
                                  show_next_menu,
                                  state_menu);
                
                state_machine.add(state_menu,
                                  event_previous_menu,
                                  show_previous_menu,
                                  state_menu);
                
                state_machine.add(state_menu,
                                  event_select,
                                  select_menu,
                                  state_menu_selection_waiting_confirmation);
                
                state_machine.add(state_menu_selection_waiting_confirmation,
                                  event_select,
                                  execute_menu,
                                  state_executing_script);
                
                state_machine.add(state_menu_selection_waiting_confirmation,
                                  event_timer_timeout,
                                  show_current_menu,
                                  state_menu);
                
                state_machine.add(state_executing_script,
                                  event_stop,
                                  stop_rover,
                                  state_stopping);
                
                state_machine.add(state_executing_script,
                                  event_script_finished,
                                  show_current_menu,
                                  state_menu);
        }
}
