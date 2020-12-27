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
#include <r.h>
#include "EventsAndStates.h"
#include "UIStateMachine.h"

namespace romi {
        
        void initialize_rover(Rover& rover)
        {
                r_debug("initialize_rover");
                rover.display.show(0, "Ready");
                rover.notifications.notify(NotifyStartup);
        }

        void stop_rover(Rover& rover)
        {
                r_debug("stop_rover");
                // rover.saved_state = rover.state_machine.get_state();
                // rover.display.show(0, "[] to reset");
                // rover.display.show(1, "X  to continue");
        }
        
        void reset_rover(Rover& rover)
        {
                r_debug("reset_rover");
        }
        
        void continue_rover(Rover& rover)
        {
                r_debug("continue_rover");
        }
        
        void confirm_navigation_step_1(Rover& rover)
        {
                r_debug("confirm_navigation_step_1");
                rover.display.show(0, "Hold to navigate");
                rover.event_timer.set_timeout(3.0);
        }

        void confirm_navigation_step_2(Rover& rover)
        {
                r_debug("confirm_navigation_step_2");
                rover.display.show(0, "Navigate? (X)");
                rover.event_timer.set_timeout(2.0);
                rover.notifications.notify(NotifyConfirmNavigationMode);
        }

        void initialize_navigation(Rover& rover)
        {
                r_debug("initialize_navigation");
                rover.speed_controller.stop();
                rover.display.show(0, "Navigating");
                // set timer to return to main mode
                rover.event_timer.set_timeout(10.0);
        }

        void leave_navigation_mode(Rover& rover)
        {
                r_debug("leave_navigation_mode");
                rover.display.show(0, "Ready");
                rover.event_timer.reset(); 
                rover.notifications.notify(NotifyLeaveNavigationMode);
        }

        void start_driving_forward(Rover& rover)
        {
                r_debug("start moving forward");
                rover.event_timer.set_timeout(10.0); // restart timer
        }

        void drive_forward(Rover& rover)
        {
                r_debug("update forward speed and direction");
                double speed = rover.input_device.get_forward_speed();
                double direction = rover.input_device.get_direction();
                if (speed < 0.0)
                        speed = 0.0;
                rover.speed_controller.drive_at(speed, direction);
                rover.event_timer.set_timeout(10.0); // restart timer
        }

        void start_driving_backward(Rover& rover)
        {
                r_debug("start moving backward");
                rover.event_timer.set_timeout(10.0); // restart timer
        }
        
        void drive_backward(Rover& rover)
        {
                r_debug("update backward speed and direction");
                double speed = rover.input_device.get_backward_speed();
                double direction = rover.input_device.get_direction();
                if (speed < 0.0)
                        speed = 0.0;
                rover.speed_controller.drive_at(-speed, direction);
                rover.event_timer.set_timeout(10.0); // restart timer
        }

        void stop_driving(Rover& rover)
        {
                r_debug("stop");
                rover.speed_controller.stop();
                rover.event_timer.set_timeout(10.0); // restart timer
        }
        
        void start_driving_forward_accurately(Rover& rover)
        {
                r_debug("start moving forward accurately");
                rover.event_timer.set_timeout(10.0); // restart timer
        }
        
        void drive_forward_accurately(Rover& rover)
        {
                r_debug("update accurate forward speed and direction");
                double speed = rover.input_device.get_forward_speed();
                double direction = rover.input_device.get_direction();
                if (speed < 0.0)
                        speed = 0.0;
                rover.speed_controller.drive_accurately_at(speed, direction);
                rover.event_timer.set_timeout(10.0); // restart timer
        }

        void start_driving_backward_accurately(Rover& rover)
        {
                r_debug("start moving backward accurately");
                rover.event_timer.set_timeout(10.0); // restart timer
        }
        
        void drive_backward_accurately(Rover& rover)
        {
                r_debug("update accurate backward speed and direction");
                double speed = rover.input_device.get_backward_speed();
                double direction = rover.input_device.get_direction();
                if (speed < 0.0)
                        speed = 0.0;
                rover.speed_controller.drive_accurately_at(-speed, direction);
                rover.event_timer.set_timeout(10.0); // restart timer
        }
        
        void start_spinning(Rover& rover)
        {
                r_debug("start spinning");
                rover.event_timer.set_timeout(10.0); // restart timer
        }
        
        void spin(Rover& rover)
        {
                r_debug("spinning");
                rover.speed_controller.spin(rover.input_device.get_direction());
                rover.event_timer.set_timeout(10.0); // restart timer
        }

        
        void confirm_menu_step_1(Rover& rover)
        {
                r_debug("confirm_menu_step_1");
                rover.display.show(0, "Hold for menu");
                rover.event_timer.set_timeout(3.0);
        }

        void confirm_menu_step_2(Rover& rover)
        {
                r_debug("confirm_menu_step_2");
                rover.display.show(0, "Menu? (X)");
                rover.event_timer.set_timeout(2.0);
                rover.notifications.notify(NotifyConfirmMenuMode);
        }

        void open_menu(Rover& rover)
        {
                r_debug("open_menu");
                std::string name;
                rover.menu.first_menu_item(name);
                rover.display.show(0, name.c_str());
                rover.display.clear(1);
                // set timer to return to main mode
                rover.event_timer.set_timeout(10.0);  
        }

        void show_next_menu(Rover& rover)
        {
                r_debug("show_next_menu");
                std::string name;
                rover.menu.next_menu_item(name);
                rover.display.show(0, name.c_str());
                rover.event_timer.set_timeout(10.0);  // restart timer
                rover.notifications.notify(NotifyChangeMenu);
        }

        void show_previous_menu(Rover& rover)
        {
                r_debug("show_previous_menu");
                std::string name;
                rover.menu.previous_menu_item(name);
                rover.display.show(0, name.c_str());
                rover.event_timer.set_timeout(10.0); // restart timer
                rover.notifications.notify(NotifyChangeMenu);
        }

        void show_current_menu(Rover& rover)
        {
                r_debug("show_current_menu");
                std::string name;
                rover.menu.get_current_menu(name);
                rover.display.show(0, name.c_str());
                rover.display.clear(1);
        }

        void signal_end_and_show_current_menu(Rover& rover)
        {
                r_debug("signal_end_and_show_current_menu");
                rover.notifications.notify(NotifyMenuFinished);
                show_current_menu(rover);
                rover.event_timer.set_timeout(10.0); // restart timer
        }

        void select_menu(Rover& rover)
        {
                r_debug("select_menu");
                rover.display.show(1, "X to confirm");
                rover.event_timer.set_timeout(1.0);
        }
        
        void execute_menu(Rover& rover)
        {
                r_debug("execute_menu");
                rover.notifications.notify(NotifyMenuConfirmed);
                int index = rover.menu.get_current_index();
                rover.script_engine.execute_script(rover, index);
                rover.display.show(1, "Running");
        }
        
        void leave_menu_mode(Rover& rover)
        {
                r_debug("leave_menu_mode");
                rover.display.show(0, "Ready");
                rover.event_timer.reset(); 
                rover.notifications.notify(NotifyLeaveMenuMode);
        }

        void UIStateMachine::init_state_transitions()
        {
                init_start_transition();
                init_navigation_transitions();
                init_menu_transitions();
        }

        void UIStateMachine::init_start_transition()
        {
                add(STATE_START,
                    event_start,
                    initialize_rover,
                    state_ready);
        }

        void UIStateMachine::init_stop_transitions()
        {
                add(state_stopping,
                    event_stop,
                    reset_rover,
                    state_ready);
                
                add(state_stopping,
                    event_select,
                    continue_rover,
                    saved_state);

                add(state_stopping,
                    event_timer_timeout,
                    reset_rover,
                    state_ready);
        }
        
        void UIStateMachine::init_navigation_transitions()
        {
                init_navigation_mode_transition();
                init_forward_driving_transitions();
                init_accurate_forward_driving_transitions();
                init_backward_driving_transitions();
                init_accurate_backward_driving_transitions();
                init_spinning_transitions();
        }

        void UIStateMachine::init_navigation_mode_transition()
        {
                // The user pressed the "O" button. In the first step,
                // she must hold the button for a certain time to
                // confirm entering the navigation. This avoids
                // spurious buttons presses.
                add(state_ready,
                    event_navigation_mode_pressed,
                    confirm_navigation_step_1,
                    state_waiting_navigate_confirmation_1);

                // The event timer sends the confirmation event when
                // the button is held long enough.
                add(state_waiting_navigate_confirmation_1,
                    event_timer_timeout,
                    confirm_navigation_step_2,
                    state_waiting_navigate_confirmation_2);
                
                // If the button is released before the timeout, the
                // "released" event will bring the state machine back
                // to the main state.
                add(state_waiting_navigate_confirmation_1,
                    event_navigation_mode_released,
                    leave_navigation_mode,
                    state_ready);

                // Once the button has been held for long enough,
                // there is a second confirmation. In the second step,
                // the user must confirm entering the navigation mode
                // by pressing the X/select button.
                add(state_waiting_navigate_confirmation_2,
                    event_select,
                    initialize_navigation,
                    state_ready_to_navigate);

                // There's a timeout also for the second confirmation
                // step. If the user doesn't press confirm/select
                // within the given time, the state goes back to the
                // main state.
                add(state_waiting_navigate_confirmation_2,
                    event_timer_timeout,
                    leave_navigation_mode,
                    state_ready);

                // In any case, when the user interface is in the
                // navigation mode, when no user inputs have been
                // received for a given time, the input device
                // generates a "input delay passed" event. In this
                // case, the user interface quits the navigation mode
                // and returns to the main mode.
                add(state_ready_to_navigate,
                    event_timer_timeout,
                    leave_navigation_mode,
                    state_ready);
                
                add(state_ready_to_navigate,
                    event_stop,
                    leave_navigation_mode,
                    state_ready);                
        }
        
        void UIStateMachine::init_forward_driving_transitions()
        {
                add(state_ready_to_navigate,
                    event_forward_start,
                    start_driving_forward,
                    state_moving_forward);
                
                add(state_moving_forward,
                    event_forward_speed,
                    drive_forward,
                    state_moving_forward);
                
                add(state_moving_forward,
                    event_direction,
                    drive_forward,
                    state_moving_forward);

                add(state_moving_forward,
                    event_forward_stop,
                    stop_driving,
                    state_ready_to_navigate);
        }
        
        void UIStateMachine::init_accurate_forward_driving_transitions()
        {
                add(state_ready_to_navigate,
                    event_accurate_forward_start,
                    start_driving_forward_accurately,
                    state_moving_forward_accurately);
                
                add(state_moving_forward_accurately,
                    event_forward_speed,
                    drive_forward_accurately,
                    state_moving_forward_accurately);
                
                add(state_moving_forward_accurately,
                    event_direction,
                    drive_forward_accurately,
                    state_moving_forward_accurately);

                add(state_moving_forward_accurately,
                    event_accurate_forward_stop,
                    stop_driving,
                    state_ready_to_navigate);
        }
        
        void UIStateMachine::init_backward_driving_transitions()
        {
                add(state_ready_to_navigate,
                    event_backward_start,
                    start_driving_backward,
                    state_moving_backward);
                
                add(state_moving_backward,
                    event_backward_speed,
                    drive_backward,
                    state_moving_backward);
                
                add(state_moving_backward,
                    event_direction,
                    drive_backward,
                    state_moving_backward);
                
                add(state_moving_backward,
                    event_backward_stop,
                    stop_driving,
                    state_ready_to_navigate);
        }
        
        void UIStateMachine::init_accurate_backward_driving_transitions()
        {
                add(state_ready_to_navigate,
                    event_accurate_backward_start,
                    start_driving_backward_accurately,
                    state_moving_backward_accurately);
                
                add(state_moving_backward_accurately,
                    event_backward_speed,
                    drive_backward_accurately,
                    state_moving_backward_accurately);
                
                add(state_moving_backward_accurately,
                    event_direction,
                    drive_backward_accurately,
                    state_moving_backward_accurately);

                add(state_moving_backward_accurately,
                    event_accurate_backward_stop,
                    stop_driving,
                    state_ready_to_navigate);
        }
        
        void UIStateMachine::init_spinning_transitions()
        {
                add(state_ready_to_navigate,
                    event_spinning_start,
                    start_spinning,
                    state_spinning);
                 
                add(state_spinning,
                    event_direction,
                    spin,
                    state_spinning);

                add(state_spinning,
                    event_spinning_stop,
                    stop_driving,
                    state_ready_to_navigate);
        }

        void UIStateMachine::init_menu_transitions()
        {
                init_menu_mode_transition();
                init_menu_selection_transitions();
        }

        void UIStateMachine::init_menu_mode_transition()
        {
                // For an explanation, check the comments in
                // init_navigation_mode_transition(). To enter the
                // menu mode, the steps are similar. The "O" button is
                // replaced with the "triangle" button.
                add(state_ready,
                    event_menu_mode_pressed,
                    confirm_menu_step_1,
                    state_menu_mode_waiting_confirmation_1);
                
                add(state_menu_mode_waiting_confirmation_1,
                    event_timer_timeout,
                    confirm_menu_step_2,
                    state_menu_mode_waiting_confirmation_2);
                
                add(state_menu_mode_waiting_confirmation_1,
                    event_menu_mode_released,
                    leave_menu_mode,
                    state_ready);
                
                add(state_menu_mode_waiting_confirmation_2,
                    event_select,
                    open_menu,
                    state_menu);
                
                add(state_menu_mode_waiting_confirmation_2,
                    event_timer_timeout,
                    leave_menu_mode,
                    state_ready);
                
                add(state_menu,
                    event_timer_timeout,
                    leave_menu_mode,
                    state_ready);                

                add(state_menu,
                    event_stop,
                    leave_menu_mode,
                    state_ready);                
        }
        
        void UIStateMachine::init_menu_selection_transitions()
        {
                add(state_menu,
                    event_next_menu,
                    show_next_menu,
                    state_menu);
                
                add(state_menu,
                    event_previous_menu,
                    show_previous_menu,
                    state_menu);
                
                add(state_menu,
                    event_select,
                    select_menu,
                    state_menu_selection_waiting_confirmation);
                
                add(state_menu_selection_waiting_confirmation,
                    event_select,
                    execute_menu,
                    state_executing_script);
                
                add(state_menu_selection_waiting_confirmation,
                    event_timer_timeout,
                    show_current_menu,
                    state_menu);
                
                add(state_executing_script,
                    event_stop,
                    stop_rover,
                    state_stopping);
                
                add(state_executing_script,
                    event_script_finished,
                    signal_end_and_show_current_menu,
                    state_menu);
        }
}
