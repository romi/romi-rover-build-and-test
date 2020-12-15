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
#include <exception>
#include <stdexcept>
#include <string.h>
#include <rcom.h>

#include "Joystick.h"
#include "StateMachine.h"
#include "StateTransition.h"
#include "DebugNavigation.h"
#include "RPCNavigationClientAdaptor.h"
#include "RPCClient.h"
#include "SpeedController.h"
#include "JoystickStateTransitions.h"
#include "EventMapper.h"

using namespace romi;
        
int main(int argc, char** argv)
{
        app_init(&argc, argv);
        app_set_name("oquam");
        
        try {
                JoystickEvent event;
                Joystick joystick("/dev/input/js0");
                //joystick.set_debug(true);

                //DebugNavigation navigation;
                rcom::RPCClient rpc("navigation", "navigation");
                RPCNavigationClientAdaptor navigation(rpc);
                        
                SpeedController speed_controller(navigation);
                
                EventMapper eventMapper(joystick);
                
                StateMachine state_machine;

                NavigationReady navigation_ready(speed_controller);
                
                state_machine.add(STATE_START,
                                  event_start,
                                  state_stopped,
                                  navigation_ready);
                
                StartDrivingForward start_driving_forward(joystick, speed_controller);
                
                state_machine.add(state_stopped,
                                  event_forward_start,
                                  state_moving_forward,
                                  start_driving_forward);
                
                
                DriveForward drive_forward(joystick, speed_controller);
                
                state_machine.add(state_moving_forward,
                                  event_forward_speed,
                                  state_moving_forward,
                                  drive_forward);
                
                state_machine.add(state_moving_forward,
                                  event_direction,
                                  state_moving_forward,
                                  drive_forward);

                StopDriving stop_driving(speed_controller);
                
                state_machine.add(state_moving_forward,
                                  event_forward_stop,
                                  state_stopped,
                                  stop_driving);
                
                StartDrivingForwardAccurately start_driving_forward_accurately(joystick,
                                                                               speed_controller);

                state_machine.add(state_stopped,
                                  event_accurate_forward_start,
                                  state_moving_forward_accurately,
                                  start_driving_forward_accurately);
                
                DriveForwardAccurately drive_forward_accurately(joystick, speed_controller);


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
                
                StartDrivingBackward start_driving_backward(joystick, speed_controller);
                
                state_machine.add(state_stopped,
                                  event_backward_start,
                                  state_moving_backward,
                                  start_driving_backward);
                
                
                DriveBackward drive_backward(joystick, speed_controller);
                
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

                StartDrivingBackwardAccurately start_driving_backward_accurately(joystick,
                                                                                 speed_controller);

                state_machine.add(state_stopped,
                                  event_accurate_backward_start,
                                  state_moving_backward_accurately,
                                  start_driving_backward_accurately);
                
                DriveBackwardAccurately drive_backward_accurately(joystick,
                                                                  speed_controller);


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
                
                
                StartSpinning start_spinning;
                Spin spin(joystick, speed_controller);
                
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

                  
                state_machine.handleEvent(event_start, 0);
                  
                while (!app_quit()) {
                        if (joystick.update(event)) {
                                int16_t state_event = eventMapper.map(event);
                                if (state_event > 0)
                                        state_machine.handleEvent(state_event, 0);
                        } else {
                                clock_sleep(0.010);
                        }
                }
                
        } catch (std::runtime_error& e) {
                r_err("main: std::runtime_error: %s", e.what());
                
        } catch (std::exception& e) {
                r_err("main: std::exception: %s", e.what());
        }

        return 0;
}
