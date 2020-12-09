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

#include "IJoystick.h"
#include "INavigation.h"
#include "StateTransition.h"
#include "EventMapper.h"
#include "NavigationEvents.h"

namespace romi {
 
        class NavigationReady : public StateTransitionHandler
        {
        protected:
                INavigation &_navigation;
                
        public:
                NavigationReady(INavigation &navigation)
                        : _navigation(navigation) {}
                
                void doTransition(unsigned long t) override {
                        r_debug("NavigationReady");
                        _navigation.stop();
                }
        };
        
        class StartDrivingForward : public StateTransitionHandler
        {
        protected:
                IJoystick &_joystick;
                INavigation &_navigation;
                
        public:
                StartDrivingForward(IJoystick &joystick, INavigation &navigation)
                        : _joystick(joystick), _navigation(navigation)
                        {}
                
                void doTransition(unsigned long t) override {
                        r_debug("start moving forward");
                }
        };
        
        class DriveForward : public StateTransitionHandler
        {
        protected:
                IJoystick &_joystick;
                INavigation &_navigation;
                
        public:
                DriveForward(IJoystick &joystick, INavigation &navigation)
                        : _joystick(joystick), _navigation(navigation)
                        {}
                
                void doTransition(unsigned long t) override {
                        r_debug("update forward speed and direction");
                        double speed = _joystick.get_axis(EventMapper::AxisForwardSpeed);
                        double direction = _joystick.get_axis(EventMapper::AxisDirection);
                        if (speed < 0.0)
                                speed = 0.0;
                        _navigation.drive_at(speed, direction);
                }
        };

        class StartDrivingBackward : public StateTransitionHandler
        {
        protected:
                IJoystick &_joystick;
                INavigation &_navigation;
                
        public:
                StartDrivingBackward(IJoystick &joystick, INavigation &navigation)
                        : _joystick(joystick), _navigation(navigation)
                        {}
                
                void doTransition(unsigned long t) override {
                        r_debug("start moving backward");
                }
        };
        
        class DriveBackward : public StateTransitionHandler
        {
        protected:
                IJoystick &_joystick;
                INavigation &_navigation;
                
        public:
                DriveBackward(IJoystick &joystick, INavigation &navigation)
                        : _joystick(joystick), _navigation(navigation)
                        {}
                
                void doTransition(unsigned long t) override {
                        r_debug("update backward speed and direction");
                        double speed = _joystick.get_axis(EventMapper::AxisBackwardSpeed);
                        double direction = _joystick.get_axis(EventMapper::AxisDirection);
                        if (speed < 0.0)
                                speed = 0.0;
                        _navigation.drive_at(-speed, direction);
                }
        };

        class StopDriving : public StateTransitionHandler
        {
        protected:
                INavigation &_navigation;
                
        public:
                StopDriving(INavigation &navigation) : _navigation(navigation) {}
                
                void doTransition(unsigned long t) override {
                        r_debug("stop");
                        _navigation.stop();
                }
        };
        
        class StartDrivingForwardAccurately : public StateTransitionHandler
        {
        protected:
                IJoystick &_joystick;
                INavigation &_navigation;
                
        public:
                StartDrivingForwardAccurately(IJoystick &joystick, INavigation &navigation)
                        : _joystick(joystick), _navigation(navigation)
                        {}
                
                void doTransition(unsigned long t) override {
                        r_debug("start moving forward accurately");
                }
        };
        
        class DriveForwardAccurately : public StateTransitionHandler
        {
        protected:
                IJoystick &_joystick;
                INavigation &_navigation;
                
        public:
                DriveForwardAccurately(IJoystick &joystick, INavigation &navigation)
                        : _joystick(joystick), _navigation(navigation)
                        {}
                
                void doTransition(unsigned long t) override {
                        r_debug("update accurate forward speed and direction");
                        double speed = _joystick.get_axis(EventMapper::AxisForwardSpeed);
                        double direction = _joystick.get_axis(EventMapper::AxisDirection);
                        if (speed < 0.0)
                                speed = 0.0;
                        _navigation.drive_accurately_at(speed, direction);
                }
        };
                
        
        class StartDrivingBackwardAccurately : public StateTransitionHandler
        {
        protected:
                IJoystick &_joystick;
                INavigation &_navigation;
                
        public:
                StartDrivingBackwardAccurately(IJoystick &joystick, INavigation &navigation)
                        : _joystick(joystick), _navigation(navigation)
                        {}
                
                void doTransition(unsigned long t) override {
                        r_debug("start moving backward accurately");
                }
        };
        
        class DriveBackwardAccurately : public StateTransitionHandler
        {
        protected:
                IJoystick &_joystick;
                INavigation &_navigation;
                
        public:
                DriveBackwardAccurately(IJoystick &joystick, INavigation &navigation)
                        : _joystick(joystick), _navigation(navigation)
                        {}
                
                void doTransition(unsigned long t) override {
                        r_debug("update accurate backward speed and direction");
                        double speed = _joystick.get_axis(EventMapper::AxisBackwardSpeed);
                        double direction = _joystick.get_axis(EventMapper::AxisDirection);
                        if (speed < 0.0)
                                speed = 0.0;
                        _navigation.drive_accurately_at(-speed, direction);
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
                        
        class Spinning : public StateTransitionHandler
        {
        protected:
                IJoystick &_joystick;
                INavigation &_navigation;
                
        public:
                Spinning(IJoystick &joystick, INavigation &navigation)
                        : _joystick(joystick), _navigation(navigation)
                        {}
                
                void doTransition(unsigned long t) override {
                        r_debug("spinning");
                        _navigation.spin(_joystick.get_axis(EventMapper::AxisDirection));
                }
        };

}

#endif // __ROMI_JOYSTICK_STATE_TRANSITION_H
