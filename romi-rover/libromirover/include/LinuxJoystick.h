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
#ifndef _ROMI_LINUX_JOYSTICK_H_
#define _ROMI_LINUX_JOYSTICK_H_

#include <string>
#include <vector>
#include "api/Joystick.h"
#include "ILinux.h"

namespace romi {
        
        class LinuxJoystick : public Joystick
        {
        protected:
                rpp::ILinux &_linux;
                int _fd;
                bool _debug;
                JoystickEvent _event;
                
                std::vector<bool> _buttons;
                std::vector<double> _axes;

                void try_initialize();
                void initialize();

                void try_open_device(const char *name);
                void open_device(const char *name);
                void close_device();
                
                void handle_input_event();
                bool has_event(double timeout);
                void read_event(struct js_event& linux_event);
                void parse_event(struct js_event& linux_event);
                void parse_axis_event(struct js_event& linux_event);
                void parse_button_event(struct js_event& linux_event);
                
                int try_count_axes();
                int count_axes();                
                int try_count_buttons();
                int count_buttons();
                
        public:
                
                LinuxJoystick(rpp::ILinux &linux, const char *device);
                virtual ~LinuxJoystick();
                
                JoystickEvent& get_next_event() override;
                
                double get_axis(int i) override {
                        double value = 0.0;
                        if (i >=0 && i < (int) _axes.size())
                                value = _axes[i];
                        return value;
                }
                
                bool is_button_pressed(int i) {
                        bool value = false;
                        if (i >=0 && i < (int) _buttons.size())
                                value = _buttons[i];
                        return value;
                }
                        
                int get_num_axes() override {
                        return (int) _axes.size();
                }
                
                int get_num_buttons() override {
                        return (int) _buttons.size();
                }

                void set_debug(bool value) {
                        _debug = value;
                }

        };
}

#endif // _ROMI_LINUX_JOYSTICK_H_
