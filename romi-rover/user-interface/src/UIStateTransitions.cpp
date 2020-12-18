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

#include "UIStateTransitions.h"

namespace romi {
        
        void navigation_ready(UserInterface& ui, unsigned long t)
        {
                r_debug("NavigationReady");
                ui.speed_controller.stop();
                ui.display.show(0, "Ready");
        }

        void start_driving_forward(UserInterface& ui, unsigned long t)
        {
                r_debug("start moving forward");
        }

        void drive_forward(UserInterface& ui, unsigned long t)
        {
                r_debug("update forward speed and direction");
                double speed = ui.joystick.get_axis(axis_forward_speed);
                double direction = ui.joystick.get_axis(axis_direction);
                if (speed < 0.0)
                        speed = 0.0;
                ui.speed_controller.drive_at(speed, direction);
        }

        void start_driving_backward(UserInterface& ui, unsigned long t)
        {
                r_debug("start moving backward");
        }
        
        void drive_backward(UserInterface& ui, unsigned long t)
        {
                r_debug("update backward speed and direction");
                double speed = ui.joystick.get_axis(axis_backward_speed);
                double direction = ui.joystick.get_axis(axis_direction);
                if (speed < 0.0)
                        speed = 0.0;
                ui.speed_controller.drive_at(-speed, direction);
        }

        void stop_driving(UserInterface& ui, unsigned long t)
        {
                r_debug("stop");
                ui.speed_controller.stop();
        }
        
        void start_driving_forward_accurately(UserInterface& ui, unsigned long t)
        {
                r_debug("start moving forward accurately");
        }
        
        void drive_forward_accurately(UserInterface& ui, unsigned long t)
        {
                r_debug("update accurate forward speed and direction");
                double speed = ui.joystick.get_axis(axis_forward_speed);
                double direction = ui.joystick.get_axis(axis_direction);
                if (speed < 0.0)
                        speed = 0.0;
                ui.speed_controller.drive_accurately_at(speed, direction);
        }

        void start_driving_backward_accurately(UserInterface& ui, unsigned long t)
        {
                r_debug("start moving backward accurately");
        }
        
        void drive_backward_accurately(UserInterface& ui, unsigned long t)
        {
                r_debug("update accurate backward speed and direction");
                double speed = ui.joystick.get_axis(axis_backward_speed);
                double direction = ui.joystick.get_axis(axis_direction);
                if (speed < 0.0)
                        speed = 0.0;
                ui.speed_controller.drive_accurately_at(-speed, direction);
        }
        
        void start_spinning(UserInterface& ui, unsigned long t)
        {
                r_debug("start spinning");
        }
        
        void spin(UserInterface& ui, unsigned long t)
        {
                r_debug("spinning");
                ui.speed_controller.spin(ui.joystick.get_axis(axis_direction));
        }
        
        void init_state_transitions(StateMachine& state_machine)
        {
                state_machine.add(STATE_START,
                                  event_start,
                                  state_stopped,
                                  navigation_ready);
                
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
