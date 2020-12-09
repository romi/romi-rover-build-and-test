#include <exception>
#include <stdexcept>
#include <string.h>
#include <rcom.h>

#include "Joystick.h"
#include "StateMachine.h"
#include "StateTransition.h"
#include "DebugMotorController.h"
#include "Navigation.h"
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

                DebugMotorController controller;
                Navigation navigation(controller);
                
                EventMapper eventMapper(joystick);
                
                StateMachine stateMachine;

                NavigationReady navReady(navigation);
                
                stateMachine.add(STATE_START,
                                 ROVER_EVENT_START,
                                 ROVER_STOPPED,
                                 navReady);
                
                StartDrivingForward startDrivingForward(joystick, navigation);
                
                stateMachine.add(ROVER_STOPPED,
                                 ROVER_EVENT_FORWARD_START,
                                 ROVER_MOVING_FORWARD,
                                 startDrivingForward);
                
                
                DriveForward driveForward(joystick, navigation);
                
                stateMachine.add(ROVER_MOVING_FORWARD,
                                 ROVER_EVENT_FORWARD_SPEED,
                                 ROVER_MOVING_FORWARD,
                                 driveForward);
                
                stateMachine.add(ROVER_MOVING_FORWARD,
                                 ROVER_EVENT_DIRECTION,
                                 ROVER_MOVING_FORWARD,
                                 driveForward);

                StopDriving stopDriving(navigation);
                
                stateMachine.add(ROVER_MOVING_FORWARD,
                                 ROVER_EVENT_FORWARD_STOP,
                                 ROVER_STOPPED,
                                 stopDriving);
                
                StartDrivingForwardAccurately startDrivingForwardAccurately(joystick,
                                                                            navigation);

                stateMachine.add(ROVER_STOPPED,
                                 ROVER_EVENT_ACCURATE_FORWARD_START,
                                 ROVER_MOVING_FORWARD_ACCURATELY,
                                 startDrivingForwardAccurately);
                
                DriveForwardAccurately driveForwardAccurately(joystick, navigation);


                stateMachine.add(ROVER_MOVING_FORWARD_ACCURATELY,
                                 ROVER_EVENT_FORWARD_SPEED,
                                 ROVER_MOVING_FORWARD_ACCURATELY,
                                 driveForwardAccurately);
                stateMachine.add(ROVER_MOVING_FORWARD_ACCURATELY,
                                 ROVER_EVENT_DIRECTION,
                                 ROVER_MOVING_FORWARD_ACCURATELY,
                                 driveForwardAccurately);

                stateMachine.add(ROVER_MOVING_FORWARD_ACCURATELY,
                                 ROVER_EVENT_ACCURATE_FORWARD_STOP,
                                 ROVER_STOPPED,
                                 stopDriving);
                
                StartDrivingBackward startDrivingBackward(joystick, navigation);
                
                stateMachine.add(ROVER_STOPPED,
                                 ROVER_EVENT_BACKWARD_START,
                                 ROVER_MOVING_BACKWARD,
                                 startDrivingBackward);
                
                
                DriveBackward driveBackward(joystick, navigation);
                
                stateMachine.add(ROVER_MOVING_BACKWARD,
                                 ROVER_EVENT_BACKWARD_SPEED,
                                 ROVER_MOVING_BACKWARD,
                                 driveBackward);
                
                stateMachine.add(ROVER_MOVING_BACKWARD,
                                 ROVER_EVENT_DIRECTION,
                                 ROVER_MOVING_BACKWARD,
                                 driveBackward);
                
                stateMachine.add(ROVER_MOVING_BACKWARD,
                                 ROVER_EVENT_BACKWARD_STOP,
                                 ROVER_STOPPED,
                                 stopDriving);

                StartDrivingBackwardAccurately startDrivingBackwardAccurately(joystick,
                                                                            navigation);

                stateMachine.add(ROVER_STOPPED,
                                 ROVER_EVENT_ACCURATE_BACKWARD_START,
                                 ROVER_MOVING_BACKWARD_ACCURATELY,
                                 startDrivingBackwardAccurately);
                
                DriveBackwardAccurately driveBackwardAccurately(joystick, navigation);


                stateMachine.add(ROVER_MOVING_BACKWARD_ACCURATELY,
                                 ROVER_EVENT_BACKWARD_SPEED,
                                 ROVER_MOVING_BACKWARD_ACCURATELY,
                                 driveBackwardAccurately);
                stateMachine.add(ROVER_MOVING_BACKWARD_ACCURATELY,
                                 ROVER_EVENT_DIRECTION,
                                 ROVER_MOVING_BACKWARD_ACCURATELY,
                                 driveBackwardAccurately);

                stateMachine.add(ROVER_MOVING_BACKWARD_ACCURATELY,
                                 ROVER_EVENT_ACCURATE_BACKWARD_STOP,
                                 ROVER_STOPPED,
                                 stopDriving);
                
                
                StartSpinning startSpinning;
                Spinning spinning(joystick, navigation);
                
                stateMachine.add(ROVER_STOPPED,
                                 ROVER_EVENT_SPINNING_START,
                                 ROVER_SPINNING,
                                 startSpinning);
                 
                stateMachine.add(ROVER_SPINNING,
                                 ROVER_EVENT_DIRECTION,
                                 ROVER_SPINNING,
                                 spinning);

                stateMachine.add(ROVER_SPINNING,
                                 ROVER_EVENT_SPINNING_STOP,
                                 ROVER_STOPPED,
                                 stopDriving);

                  
                stateMachine.handleEvent(ROVER_EVENT_START, 0);
                  
                while (!app_quit()) {
                        if (joystick.update(event)) {
                                int16_t state_event = eventMapper.map(event);
                                if (state_event > 0)
                                        stateMachine.handleEvent(state_event, 0);
                        } else {
                                clock_sleep(0.010);
                        }
                }
                
        } catch (std::runtime_error e) {
                r_err("main: std::runtime_error: %s", e.what());
                
        } catch (std::exception e) {
                r_err("main: std::exception: %s", e.what());
        }

        return 0;
}
