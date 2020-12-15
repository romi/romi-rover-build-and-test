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
#ifndef _ROMI_JOYSTICK_H_
#define _ROMI_JOYSTICK_H_

#include <string>
#include <vector>
#include "IJoystick.h"

namespace romi {
        
        class Joystick : public IJoystick
        {
        protected:
                int _fd;
                bool _debug;
                
                std::vector<bool> _buttons;
                std::vector<double> _axes;
                
                void close_device();
                bool open_device(const char *name);
                bool parse_event(JoystickEvent &e, struct js_event event);
                int count_axes();
                int count_buttons();
                
        public:
                Joystick(const char *device);
                virtual ~Joystick();

                bool update(JoystickEvent &e) override;
                
                double get_axis(int i) override {
                        double value = 0.0;
                        if (i >=0 && i < (int) _axes.size())
                                value = _axes[i];
                        return value;
                }
                
                bool get_button(int i) override {
                        bool value = false;
                        if (i >=0 && i < (int) _buttons.size())
                                value = _buttons[i];
                        return value;
                }
                        
                int num_axes() override {
                        return (int) _axes.size();
                }
                
                int num_buttons() override {
                        return (int) _buttons.size();
                }

                void set_debug(bool value) {
                        _debug = value;
                }

        };
}

#endif // _ROMI_I_JOYSTICK_H_
