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
#include "RoverStateMachine.h"

namespace romi {
        
        bool rover_ready(Rover& rover)
        {
                r_debug("rover_ready");
                // TODO: Turn power on
                rover.display.show(0, "Ready");
                rover.display.clear(1);
                rover.event_timer.reset(); 
                return true;
        }
        
        bool initialize_rover(Rover& rover)
        {
                r_debug("initialize_rover");
                rover_ready(rover);
                rover.notifications.notify(Notifications::startup);
                return true;
        }

        bool signal_system_failure(Rover& rover)
        {
                r_debug("signal_system_failure");
                // TODO: Cut power
                rover.display.show(0, "System failure");
                rover.display.show(1, "Please restart");
                rover.event_timer.reset(); 
                return true;
        }
        
        bool pause_rover(Rover& rover)
        {
                r_debug("pause_rover");
                return (rover.navigation.pause_activity()
                        && rover.weeder.pause_activity());
        }
        
        bool reset_rover(Rover& rover)
        {
                r_debug("reset_rover");
                rover.notifications.notify(Notifications::rover_reset);
                return (rover.navigation.reset_activity()
                        && rover.weeder.reset_activity());
        }
        
        bool continue_rover(Rover& rover)
        {
                r_debug("continue_rover");
                return (rover.navigation.continue_activity()
                        && !rover.weeder.continue_activity());
        }
        
        bool confirm_navigation_step_1(Rover& rover)
        {
                r_debug("confirm_navigation_step_1");
                rover.display.show(0, "Hold to navigate");
                rover.event_timer.set_timeout(3.0);
                return true;
        }

        bool confirm_navigation_step_2(Rover& rover)
        {
                r_debug("confirm_navigation_step_2");
                rover.display.show(0, "Navigate? (X)");
                rover.event_timer.set_timeout(2.0);
                rover.notifications.notify(Notifications::confirm_navigation_mode);
                return true;
        }

        bool initialize_navigation(Rover& rover)
        {
                r_debug("initialize_navigation");
                rover.speed_controller.stop();
                rover.display.show(0, "Navigating");
                // set timer to return to main mode
                rover.event_timer.set_timeout(10.0);
                return true;
        }

        bool leave_navigation_mode(Rover& rover)
        {
                r_debug("leave_navigation_mode");
                rover_ready(rover);
                rover.notifications.notify(Notifications::leave_navigation_mode);
                return true;
        }

        bool start_driving_forward(Rover& rover)
        {
                r_debug("start moving forward");
                rover.event_timer.set_timeout(10.0); // restart timer
                return true;
        }

        bool drive_forward(Rover& rover)
        {
                r_debug("update forward speed and direction");
                double speed = rover.input_device.get_forward_speed();
                double direction = rover.input_device.get_direction();
                if (speed < 0.0)
                        speed = 0.0;
                rover.speed_controller.drive_at(speed, direction);
                rover.event_timer.set_timeout(10.0); // restart timer
                return true;
        }

        bool start_driving_backward(Rover& rover)
        {
                r_debug("start moving backward");
                rover.event_timer.set_timeout(10.0); // restart timer
                return true;
        }
        
        bool drive_backward(Rover& rover)
        {
                r_debug("update backward speed and direction");
                double speed = rover.input_device.get_backward_speed();
                double direction = rover.input_device.get_direction();
                if (speed < 0.0)
                        speed = 0.0;
                rover.speed_controller.drive_at(-speed, direction);
                rover.event_timer.set_timeout(10.0); // restart timer
                return true;
        }

        bool stop_driving(Rover& rover)
        {
                r_debug("stop");
                rover.speed_controller.stop();
                rover.event_timer.set_timeout(10.0); // restart timer
                return true;
        }
        
        bool start_driving_forward_accurately(Rover& rover)
        {
                r_debug("start moving forward accurately");
                rover.event_timer.set_timeout(10.0); // restart timer
                return true;
        }
        
        bool drive_forward_accurately(Rover& rover)
        {
                r_debug("update accurate forward speed and direction");
                double speed = rover.input_device.get_forward_speed();
                double direction = rover.input_device.get_direction();
                if (speed < 0.0)
                        speed = 0.0;
                rover.speed_controller.drive_accurately_at(speed, direction);
                rover.event_timer.set_timeout(10.0); // restart timer
                return true;
        }

        bool start_driving_backward_accurately(Rover& rover)
        {
                r_debug("start moving backward accurately");
                rover.event_timer.set_timeout(10.0); // restart timer
                return true;
        }
        
        bool drive_backward_accurately(Rover& rover)
        {
                r_debug("update accurate backward speed and direction");
                double speed = rover.input_device.get_backward_speed();
                double direction = rover.input_device.get_direction();
                if (speed < 0.0)
                        speed = 0.0;
                rover.speed_controller.drive_accurately_at(-speed, direction);
                rover.event_timer.set_timeout(10.0); // restart timer
                return true;
        }
        
        bool start_spinning(Rover& rover)
        {
                r_debug("start spinning");
                rover.event_timer.set_timeout(10.0); // restart timer
                return true;
        }
        
        bool spin(Rover& rover)
        {
                r_debug("spinning");
                rover.speed_controller.spin(rover.input_device.get_direction());
                rover.event_timer.set_timeout(10.0); // restart timer
                return true;
        }

        
        bool confirm_menu_step_1(Rover& rover)
        {
                r_debug("confirm_menu_step_1");
                rover.display.show(0, "Hold for menu");
                rover.event_timer.set_timeout(3.0);
                return true;
        }

        bool confirm_menu_step_2(Rover& rover)
        {
                r_debug("confirm_menu_step_2");
                rover.display.show(0, "Menu? (X)");
                rover.event_timer.set_timeout(2.0);
                rover.notifications.notify(Notifications::confirm_menu_mode);
                return true;
        }

        bool open_menu(Rover& rover)
        {
                r_debug("open_menu");
                std::string name;
                rover.menu.first_menu_item(name);
                rover.display.show(0, name.c_str());
                rover.display.clear(1);
                // set timer to return to main mode
                rover.event_timer.set_timeout(10.0);  
                return true;
        }

        bool show_next_menu(Rover& rover)
        {
                r_debug("show_next_menu");
                std::string name;
                rover.menu.next_menu_item(name);
                rover.display.show(0, name.c_str());
                rover.event_timer.set_timeout(10.0);  // restart timer
                rover.notifications.notify(Notifications::change_menu);
                return true;
        }

        bool show_previous_menu(Rover& rover)
        {
                r_debug("show_previous_menu");
                std::string name;
                rover.menu.previous_menu_item(name);
                rover.display.show(0, name.c_str());
                rover.event_timer.set_timeout(10.0); // restart timer
                rover.notifications.notify(Notifications::change_menu);
                return true;
        }

        bool show_current_menu(Rover& rover)
        {
                r_debug("show_current_menu");
                std::string name;
                rover.menu.get_current_menu(name);
                rover.display.show(0, name.c_str());
                rover.display.clear(1);
                rover.event_timer.set_timeout(10.0); // restart timer
                return true;
        }

        bool select_menu(Rover& rover)
        {
                r_debug("select_menu");
                rover.display.show(1, "X to confirm");
                rover.event_timer.set_timeout(1.0);
                return true;
        }
        
        bool execute_script(Rover& rover)
        {
                r_debug("execute_script");
                rover.notifications.notify(Notifications::menu_confirmed);
                int index = rover.menu.get_current_index();
                rover.script_engine.execute_script(rover, index);
                rover.display.show(1, "Running");
                return true;
        }

        bool pause_script(Rover& rover)
        {
                r_debug("pause_script");
                bool success = pause_rover(rover);
                rover.display.show(0, "[] to reset");
                rover.display.show(1, "X  to continue");
                return success;
        }

        bool reset_script(Rover& rover)
        {
                r_debug("reset_script");
                bool success = reset_rover(rover);
                show_current_menu(rover);
                return success;
        }

        bool continue_script(Rover& rover)
        {
                r_debug("continue_script");
                show_current_menu(rover);
                rover.display.show(1, "Running");
                return continue_rover(rover);
        }

        bool signal_end_and_show_current_menu(Rover& rover)
        {
                r_debug("signal_end_and_show_current_menu");
                rover.notifications.notify(Notifications::script_finished);
                show_current_menu(rover);
                return true;
        }

        bool signal_script_failure(Rover& rover)
        {
                r_debug("signal_script_failure");
                rover.notifications.notify(Notifications::script_failed);
                show_current_menu(rover);
                rover.display.show(1, "Failure! []");
                return true;
        }

        bool recover_from_script_failure(Rover& rover)
        {
                r_debug("recover_from_script_failure");
                return (reset_rover(rover)
                        && rover_ready(rover));
        }
        
        bool leave_menu_mode(Rover& rover)
        {
                r_debug("leave_menu_mode");
                rover_ready(rover);
                rover.notifications.notify(Notifications::leave_menu_mode);
                return true;
        }

        void RoverStateMachine::init_state_transitions()
        {
                init_system_transitions();
                init_navigation_transitions();
                init_menu_transitions();
        }

        void RoverStateMachine::init_system_transitions()
        {
                add(STATE_START,
                    event_start,
                    initialize_rover,
                    state_ready);
                
                add(ALL_STATES,
                    event_system_failure,
                    signal_system_failure,
                    state_system_failure);
        }
        
        void RoverStateMachine::init_navigation_transitions()
        {
                init_navigation_mode_transition();
                init_forward_driving_transitions();
                init_accurate_forward_driving_transitions();
                init_backward_driving_transitions();
                init_accurate_backward_driving_transitions();
                init_spinning_transitions();
        }

        void RoverStateMachine::init_navigation_mode_transition()
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
        
        void RoverStateMachine::init_forward_driving_transitions()
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
        
        void RoverStateMachine::init_accurate_forward_driving_transitions()
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
        
        void RoverStateMachine::init_backward_driving_transitions()
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
        
        void RoverStateMachine::init_accurate_backward_driving_transitions()
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
        
        void RoverStateMachine::init_spinning_transitions()
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

        void RoverStateMachine::init_menu_transitions()
        {
                init_menu_mode_transition();
                init_menu_selection_transitions();
                init_script_execution_transitions();
        }

        void RoverStateMachine::init_menu_mode_transition()
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
        
        void RoverStateMachine::init_menu_selection_transitions()
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
                    execute_script,
                    state_executing_script);
                
                add(state_menu_selection_waiting_confirmation,
                    event_timer_timeout,
                    show_current_menu,
                    state_menu);
        }
        
        void RoverStateMachine::init_script_execution_transitions()
        {
                add(state_executing_script,
                    event_stop,
                    pause_script,
                    state_script_paused);
                
                add(state_script_paused,
                    event_stop,
                    reset_script,
                    state_menu);
                
                add(state_script_paused,
                    event_select,
                    continue_script,
                    state_executing_script);
                
                add(state_executing_script,
                    event_script_finished,
                    signal_end_and_show_current_menu,
                    state_menu);
                
                add(state_executing_script,
                    event_script_error,
                    signal_script_failure,
                    state_script_show_failure);
                
                add(state_script_show_failure,
                    event_stop,
                    recover_from_script_failure,
                    state_ready);
        }

        const char *RoverStateMachine::get_event_name(int id)
        {
                return event_name((EventID) id);
        }
        
        const char *RoverStateMachine::get_state_name(int id)
        {
                return state_name((StateID) id);
        }
}
