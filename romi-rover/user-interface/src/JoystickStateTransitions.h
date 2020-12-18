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
#ifndef __ROMI_JOYSTICK_STATE_TRANSITION_H
#define __ROMI_JOYSTICK_STATE_TRANSITION_H

#include <r.h>
#include "IJoystick.h"
#include "ISpeedController.h"
#include "StateTransition.h"
#include "EventsAndStates.h"
#include "StateMachine.h"
#include "IDisplay.h"

namespace romi {

        void init_state_transitions(StateMachine &state_machine);
        
        
        class NavigationReady : public StateTransitionHandler
        {
        protected:
                ISpeedController &_controller;
                IDisplay &_display;
                
        public:
                NavigationReady(ISpeedController &controller, IDisplay &display)
                        : _controller(controller), _display(display) {}
                
                void doTransition(unsigned long t) override {
                        r_debug("NavigationReady");
                        _controller.stop();
                        _display.show(0, "Ready");
                }
        };
        
        class StartDrivingForward : public StateTransitionHandler
        {
        protected:
                IJoystick &_joystick;
                ISpeedController &_controller;
                
        public:
                StartDrivingForward(IJoystick &joystick, ISpeedController &controller)
                        : _joystick(joystick), _controller(controller)
                        {}
                
                void doTransition(unsigned long t) override {
                        r_debug("start moving forward");
                }
        };
        
        class DriveForward : public StateTransitionHandler
        {
        protected:
                IJoystick &_joystick;
                ISpeedController &_controller;
                
        public:
                DriveForward(IJoystick &joystick, ISpeedController &controller)
                        : _joystick(joystick), _controller(controller)
                        {}
                
                void doTransition(unsigned long t) override {
                        r_debug("update forward speed and direction");
                        double speed = _joystick.get_axis(axis_forward_speed);
                        double direction = _joystick.get_axis(axis_direction);
                        if (speed < 0.0)
                                speed = 0.0;
                        _controller.drive_at(speed, direction);
                }
        };

        class StartDrivingBackward : public StateTransitionHandler
        {
        protected:
                IJoystick &_joystick;
                ISpeedController &_controller;
                
        public:
                StartDrivingBackward(IJoystick &joystick, ISpeedController &controller)
                        : _joystick(joystick), _controller(controller)
                        {}
                
                void doTransition(unsigned long t) override {
                        r_debug("start moving backward");
                }
        };
        
        class DriveBackward : public StateTransitionHandler
        {
        protected:
                IJoystick &_joystick;
                ISpeedController &_controller;
                
        public:
                DriveBackward(IJoystick &joystick, ISpeedController &controller)
                        : _joystick(joystick), _controller(controller)
                        {}
                
                void doTransition(unsigned long t) override {
                        r_debug("update backward speed and direction");
                        double speed = _joystick.get_axis(axis_backward_speed);
                        double direction = _joystick.get_axis(axis_direction);
                        if (speed < 0.0)
                                speed = 0.0;
                        _controller.drive_at(-speed, direction);
                }
        };

        class StopDriving : public StateTransitionHandler
        {
        protected:
                ISpeedController &_controller;
                
        public:
                StopDriving(ISpeedController &controller) : _controller(controller) {}
                
                void doTransition(unsigned long t) override {
                        r_debug("stop");
                        _controller.stop();
                }
        };
        
        class StartDrivingForwardAccurately : public StateTransitionHandler
        {
        protected:
                IJoystick &_joystick;
                ISpeedController &_controller;
                
        public:
                StartDrivingForwardAccurately(IJoystick &joystick,
                                              ISpeedController &controller)
                        : _joystick(joystick), _controller(controller)
                        {}
                
                void doTransition(unsigned long t) override {
                        r_debug("start moving forward accurately");
                }
        };
        
        class DriveForwardAccurately : public StateTransitionHandler
        {
        protected:
                IJoystick &_joystick;
                ISpeedController &_controller;
                
        public:
                DriveForwardAccurately(IJoystick &joystick, ISpeedController &controller)
                        : _joystick(joystick), _controller(controller)
                        {}
                
                void doTransition(unsigned long t) override {
                        r_debug("update accurate forward speed and direction");
                        double speed = _joystick.get_axis(axis_forward_speed);
                        double direction = _joystick.get_axis(axis_direction);
                        if (speed < 0.0)
                                speed = 0.0;
                        _controller.drive_accurately_at(speed, direction);
                }
        };
                
        
        class StartDrivingBackwardAccurately : public StateTransitionHandler
        {
        protected:
                IJoystick &_joystick;
                ISpeedController &_controller;
                
        public:
                StartDrivingBackwardAccurately(IJoystick &joystick,
                                               ISpeedController &controller)
                        : _joystick(joystick), _controller(controller)
                        {}
                
                void doTransition(unsigned long t) override {
                        r_debug("start moving backward accurately");
                }
        };
        
        class DriveBackwardAccurately : public StateTransitionHandler
        {
        protected:
                IJoystick &_joystick;
                ISpeedController &_controller;
                
        public:
                DriveBackwardAccurately(IJoystick &joystick, ISpeedController &controller)
                        : _joystick(joystick), _controller(controller)
                        {}
                
                void doTransition(unsigned long t) override {
                        r_debug("update accurate backward speed and direction");
                        double speed = _joystick.get_axis(axis_backward_speed);
                        double direction = _joystick.get_axis(axis_direction);
                        if (speed < 0.0)
                                speed = 0.0;
                        _controller.drive_accurately_at(-speed, direction);
                }
        };
                
        class StartSpinning : public StateTransitionHandler
        {
        public:
                StartSpinning() {}
                
                void doTransition(unsigned long t) override {
                        r_debug("start spinning");
                }
        };
                        
        class Spin : public StateTransitionHandler
        {
        protected:
                IJoystick &_joystick;
                ISpeedController &_controller;
                
        public:
                Spin(IJoystick &joystick, ISpeedController &controller)
                        : _joystick(joystick), _controller(controller)
                        {}
                
                void doTransition(unsigned long t) override {
                        r_debug("spinning");
                        _controller.spin(_joystick.get_axis(axis_direction));
                }
        };

}

#endif // __ROMI_JOYSTICK_STATE_TRANSITION_H
