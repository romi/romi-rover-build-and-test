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
#ifndef _ROMI_JOYSTICK_INPUT_DEVICE_H
#define _ROMI_JOYSTICK_INPUT_DEVICE_H

#include <string>
#include <vector>
#include "api/InputDevice.h"
#include "api/Joystick.h"
#include "JoystickEventMapper.h"

namespace romi {
        
        class JoystickInputDevice : public InputDevice
        {
        public:
                static constexpr const char *ClassName = "joystick";
                
        protected:
                Joystick &_joystick;
                JoystickEventMapper &_event_mapper;
                
                void assure_number_of_axes(int minimum);
                void assure_number_of_buttons(int minimum);

        public:
                JoystickInputDevice(Joystick &joystick,
                                    JoystickEventMapper &event_mapper);                
                virtual ~JoystickInputDevice() override = default;
                
                int get_next_event() override;
                double get_forward_speed() override;
                double get_backward_speed() override;
                double get_direction() override;
        };
}

#endif // _ROMI_JOYSTICK_INPUT_DEVICE_H
