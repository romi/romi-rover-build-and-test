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
#ifndef _ROMI_I_JOYSTICK_H_
#define _ROMI_I_JOYSTICK_H_

namespace romi {

        struct JoystickEvent
        {
                enum { Button, Axis };
                enum { Released = 0, Pressed = 1 };
                int type;
                int number;
        };
        
        class IJoystick
        {
        public:
                virtual ~IJoystick() = default;

                virtual bool update(JoystickEvent &e) = 0;
                virtual int num_axes() = 0;
                virtual double get_axis(int i) = 0;
                virtual int num_buttons() = 0;
                virtual bool get_button(int i) = 0;
        };
}

#endif // _ROMI_I_JOYSTICK_H_
