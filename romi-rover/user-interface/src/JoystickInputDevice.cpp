/*
  romi-rover

  Copyright (C) 2019 Sony Computer Science Laboratories
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
#include "JoystickInputDevice.h"
#include "EventsAndStates.h"

namespace romi {
        
        int JoystickInputDevice::get_next_event()
        {
                int event = event_none;
                JoystickEvent& joystick_event = _joystick.get_next_event();
                if (joystick_event.type != JoystickEvent::None) {
                        event = _event_mapper.map(_joystick, joystick_event);
                }
                return event;
        }

        double JoystickInputDevice::get_forward_speed()
        {
                return _joystick.get_axis(axis_forward_speed);
        }

        double JoystickInputDevice::get_backward_speed()
        {
                return _joystick.get_axis(axis_backward_speed);
        }
        
        double JoystickInputDevice::get_direction()
        {
                return _joystick.get_axis(axis_direction);
        }
}
