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
                                     SpeedController &speed_controller_)
                : input_device(input_device_),
                  display(display_),
                  speed_controller(speed_controller_),
                  state_machine(*this)
        {
                init_state_machine();
                state_machine.handle_event(event_start);
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

        void UserInterface::handle_events()
        {
                handle_input_events();
        }
        
        void UserInterface::init_state_machine()
        {
                init_start_transition();
                init_navigation_transitions();
        }

        void UserInterface::init_start_transition()
        {
                state_machine.add(STATE_START,
                                  event_start,
                                  state_stopped,
                                  navigation_ready);
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
                // TODO
        }
        
        void UserInterface::init_forward_driving_transitions()
        {
                state_machine.add(state_stopped,
                                  event_forward_start,
                                  state_moving_forward,
                                  start_driving_forward);
                
                state_machine.add(state_moving_forward,
                                  event_forward_speed,
                                  state_moving_forward,
                                  drive_forward);
                
                state_machine.add(state_moving_forward,
                                  event_direction,
                                  state_moving_forward,
                                  drive_forward);

                state_machine.add(state_moving_forward,
                                  event_forward_stop,
                                  state_stopped,
                                  stop_driving);
        }
        
        void UserInterface::init_accurate_forward_driving_transitions()
        {
                state_machine.add(state_stopped,
                                  event_accurate_forward_start,
                                  state_moving_forward_accurately,
                                  start_driving_forward_accurately);
                
                state_machine.add(state_moving_forward_accurately,
                                  event_forward_speed,
                                  state_moving_forward_accurately,
                                  drive_forward_accurately);
                
                state_machine.add(state_moving_forward_accurately,
                                  event_direction,
                                  state_moving_forward_accurately,
                                  drive_forward_accurately);

                state_machine.add(state_moving_forward_accurately,
                                  event_accurate_forward_stop,
                                  state_stopped,
                                  stop_driving);
        }
        
        void UserInterface::init_backward_driving_transitions()
        {
                state_machine.add(state_stopped,
                                  event_backward_start,
                                  state_moving_backward,
                                  start_driving_backward);
                
                state_machine.add(state_moving_backward,
                                  event_backward_speed,
                                  state_moving_backward,
                                  drive_backward);
                
                state_machine.add(state_moving_backward,
                                  event_direction,
                                  state_moving_backward,
                                  drive_backward);
                
                state_machine.add(state_moving_backward,
                                  event_backward_stop,
                                  state_stopped,
                                  stop_driving);
        }
        
        void UserInterface::init_accurate_backward_driving_transitions()
        {
                state_machine.add(state_stopped,
                                  event_accurate_backward_start,
                                  state_moving_backward_accurately,
                                  start_driving_backward_accurately);
                
                state_machine.add(state_moving_backward_accurately,
                                  event_backward_speed,
                                  state_moving_backward_accurately,
                                  drive_backward_accurately);
                
                state_machine.add(state_moving_backward_accurately,
                                  event_direction,
                                  state_moving_backward_accurately,
                                  drive_backward_accurately);

                state_machine.add(state_moving_backward_accurately,
                                  event_accurate_backward_stop,
                                  state_stopped,
                                  stop_driving);
        }
        
        void UserInterface::init_spinning_transitions()
        {
                state_machine.add(state_stopped,
                                  event_spinning_start,
                                  state_spinning,
                                  start_spinning);
                 
                state_machine.add(state_spinning,
                                  event_direction,
                                  state_spinning,
                                  spin);

                state_machine.add(state_spinning,
                                  event_spinning_stop,
                                  state_stopped,
                                  stop_driving);
        }

}
