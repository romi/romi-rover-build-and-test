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
#include <stdexcept>
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <linux/joystick.h>
#include <r.h>
#include "Joystick.h"

namespace romi {
        
        Joystick::Joystick(const char *device) : _fd(-1), _debug(false) {
                if (!open_device(device)) {
                        std::string s = "Failed to open joystick device: ";
                        s += device;
                        throw std::runtime_error(s.c_str());
                }
                _axes.resize(count_axes(), 0.0);
                _buttons.resize(count_buttons(), false);
        }
        
        Joystick::~Joystick() {
                close_device();
        }

        void Joystick::close_device() {
                if (_fd != -1) {
                        ::close(_fd);
                        _fd = -1;
                }
        }
        
        bool Joystick::open_device(const char *name) {
                close_device();
                _fd = ::open(name, O_RDONLY);
                return (_fd >= 0);
        }

        bool Joystick::parse_event(JoystickEvent &e, struct js_event event)
        {
                bool has_event = false;
                event.type &= ~JS_EVENT_INIT;

                switch (event.type) {
                        
                case JS_EVENT_BUTTON:
                        e.type = JoystickEvent::Button;
                        e.number = event.number;
                        _buttons[event.number] = event.value != 0;
                        has_event = true;
                        
                        if (_debug)
                                r_debug("button[%d]=%s", event.number,
                                        _buttons[event.number]? "pressed" : "released");
                
                        break;
                        
                case JS_EVENT_AXIS:
                        e.type = JoystickEvent::Axis;
                        e.number = event.number;
                        _axes[event.number] = event.value / 32768.0;
                        has_event = true;

                        if (_debug)
                                r_debug("axis[%d]=%0.3f", event.number,
                                        _axes[event.number]);
                        break;
                }
                return has_event;
        }

        bool Joystick::update(JoystickEvent &e)
        {
                bool has_event = false;
                struct js_event event;
                
                ssize_t bytes = read(_fd, &event, sizeof(js_event));
                if (bytes == sizeof(js_event)) {
                        has_event = parse_event(e, event);
                }
                
                return has_event;
        }

        int Joystick::count_axes()
        {
                __u8 count;
                if (ioctl(_fd, JSIOCGAXES, &count) == -1)
                        throw std::runtime_error("Joystick: Failed to obtain the "
                                                 "number of axes.");
                return (int) count;
        }

        int Joystick::count_buttons()
        {
                __u8 count;
                if (ioctl(_fd, JSIOCGBUTTONS, &count) == -1)
                        throw std::runtime_error("Joystick: Failed to obtain the "
                                                 "number of buttons.");
                return (int) count;
        }
}
