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

#include "UIEventMapper.h"

namespace romi {

        int UIEventMapper::map_axis(Joystick& joystick, JoystickEvent &event)
        {
                int retval = 0;
                        
                if (event.number == axis_direction) {
                        retval = event_direction;
                                        
                } else if (event.number == axis_backward_speed) {
                        retval = event_backward_speed;
                                        
                } else if (event.number == axis_forward_speed) {
                        retval = event_forward_speed;
                }
                        
                return retval;
        }

        int UIEventMapper::map_button(Joystick& joystick, JoystickEvent &event)
        {
                int retval = 0;
                        
                if (event.number == button_forward_mode) {
                        if (joystick.is_button_pressed(event.number)) {
                                retval = event_forward_start;
                        } else {
                                retval = event_forward_stop;
                        }
                                
                } else if (event.number == button_backward_mode) {
                        if (joystick.is_button_pressed(event.number)) {
                                retval = event_backward_start;
                        } else {
                                retval = event_backward_stop;
                        }
                                
                } else if (event.number == button_spin_mode) {
                        if (joystick.is_button_pressed(event.number)) {
                                retval = event_spinning_start;
                        } else {
                                retval = event_spinning_stop;
                        }
                                
                } else if (event.number == button_accurate_forward) {
                        if (joystick.is_button_pressed(event.number)) {
                                retval = event_accurate_forward_start;
                        } else {
                                retval = event_accurate_forward_stop;
                        }
                                
                } else if (event.number == button_accurate_backward) {
                        if (joystick.is_button_pressed(event.number)) {
                                retval = event_accurate_backward_start;
                        } else {
                                retval = event_accurate_backward_stop;
                        }
                }
                        
                return retval;
        }
                
        int UIEventMapper::map(Joystick& joystick, JoystickEvent &event)
        {
                int retval = 0;
                        
                if (event.type == JoystickEvent::Axis) {
                        retval = map_axis(joystick, event);
                                
                } else if (event.type == JoystickEvent::Button) {
                        retval = map_button(joystick, event);
                }
                        
                return retval;
        }
}
