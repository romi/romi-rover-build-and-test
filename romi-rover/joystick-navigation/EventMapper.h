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
#ifndef __ROMI_EVENT_MAPPER_H
#define __ROMI_EVENT_MAPPER_H

#include "IEventMapper.h"
#include "IJoystick.h"
#include "JoystickStateTransitions.h"
#include "NavigationEvents.h"

namespace romi {

        class EventMapper : public IEventMapper
        {
        protected:
                IJoystick &_joystick;
                
        public:

                enum {
                        AxisForwardSpeed = 5,    // R2
                        AxisBackwardSpeed = 2,    // L2
                        AxisDirection = 0,       // left stick
                        ButtonForwardMode = 7,   // R2
                        ButtonBackwardMode = 6,   // L2
                        ButtonAccurateBackward = 4, // L1
                        ButtonAccurateForward = 5, // R1
                        ButtonSpinMode = 11,
                };
                
                EventMapper(IJoystick &joystick) : _joystick(joystick) {}
                
                virtual ~EventMapper() override = default;

                int16_t map_axis(JoystickEvent &event) {
                        int16_t retval = 0;
                        
                        if (event.number == AxisDirection) {
                                retval = ROVER_EVENT_DIRECTION;
                                        
                        } else if (event.number == AxisBackwardSpeed) {
                                retval = ROVER_EVENT_BACKWARD_SPEED;
                                        
                        } else if (event.number == AxisForwardSpeed) {
                                retval = ROVER_EVENT_FORWARD_SPEED;
                        }
                        
                        return retval;
                }

                int16_t map_button(JoystickEvent &event) {
                        int16_t retval = 0;
                        
                        if (event.number == ButtonForwardMode) {
                                if (_joystick.get_button(event.number)) {
                                        retval = ROVER_EVENT_FORWARD_START;
                                } else {
                                        retval = ROVER_EVENT_FORWARD_STOP;
                                }
                                
                        } else if (event.number == ButtonBackwardMode) {
                                if (_joystick.get_button(event.number)) {
                                        retval = ROVER_EVENT_BACKWARD_START;
                                } else {
                                        retval = ROVER_EVENT_BACKWARD_STOP;
                                }
                                
                        } else if (event.number == ButtonSpinMode) {
                                if (_joystick.get_button(event.number)) {
                                        retval = ROVER_EVENT_SPINNING_START;
                                } else {
                                        retval = ROVER_EVENT_SPINNING_STOP;
                                }
                                
                        } else if (event.number == ButtonAccurateForward) {
                                if (_joystick.get_button(event.number)) {
                                        retval = ROVER_EVENT_ACCURATE_FORWARD_START;
                                } else {
                                        retval = ROVER_EVENT_ACCURATE_FORWARD_STOP;
                                }
                                
                        } else if (event.number == ButtonAccurateBackward) {
                                if (_joystick.get_button(event.number)) {
                                        retval = ROVER_EVENT_ACCURATE_BACKWARD_START;
                                } else {
                                        retval = ROVER_EVENT_ACCURATE_BACKWARD_STOP;
                                }
                        }
                        
                        return retval;
                }
                
                int16_t map(JoystickEvent &event) {
                        int16_t retval = 0;
                        
                        if (event.type == JoystickEvent::Axis) {
                                retval = map_axis(event);
                                
                        } else if (event.type == JoystickEvent::Button) {
                                retval = map_button(event);
                        }
                        
                        return retval;
                }
        };
}

#endif // __ROMI_EVENT_MAPPER_H
