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
#include "LinuxJoystick.h"

namespace romi {
        
        LinuxJoystick::LinuxJoystick(const char *device) : _fd(-1), _debug(false) {
                if (!open_device(device)) {
                        std::string s = "Failed to open joystick device: ";
                        s += device;
                        throw std::runtime_error(s.c_str());
                }
                _axes.resize(count_axes(), 0.0);
                _buttons.resize(count_buttons(), false);
        }
        
        LinuxJoystick::~LinuxJoystick() {
                close_device();
        }

        void LinuxJoystick::close_device() {
                if (_fd != -1) {
                        ::close(_fd);
                        _fd = -1;
                }
        }
        
        bool LinuxJoystick::open_device(const char *name) {
                close_device();
                _fd = ::open(name, O_RDONLY);
                return (_fd >= 0);
        }

        void LinuxJoystick::parse_button_event(struct js_event linux_event)
        {
                _event.type = JoystickEvent::Button;
                _event.number = linux_event.number;
                _buttons[linux_event.number] = linux_event.value != 0;
                        
                if (_debug)
                        r_debug("button[%d]=%s", linux_event.number,
                                _buttons[linux_event.number]? "pressed" : "released");
                
        }

        void LinuxJoystick::parse_axis_event(struct js_event linux_event)
        {
                _event.type = JoystickEvent::Axis;
                _event.number = linux_event.number;
                _axes[linux_event.number] = linux_event.value / 32768.0;

                if (_debug)
                        r_debug("axis[%d]=%0.3f", linux_event.number,
                                _axes[linux_event.number]);
        }
        
        void LinuxJoystick::parse_event(struct js_event linux_event)
        {
                linux_event.type &= ~JS_EVENT_INIT;

                switch (linux_event.type) {
                case JS_EVENT_BUTTON:
                        parse_button_event(linux_event);
                        break;
                        
                case JS_EVENT_AXIS:
                        parse_axis_event(linux_event);
                        break;
                }
        }

        void LinuxJoystick::try_read_event()
        {
                struct js_event linux_event;
                ssize_t bytes = read(_fd, &linux_event, sizeof(js_event));
                if (bytes == sizeof(js_event)) {
                        parse_event(linux_event);
                }
        }

        JoystickEvent &LinuxJoystick::get_next_event()
        {
                r_debug("LinuxJoystick::get_next_event");
                _event.type = JoystickEvent::None;
                try_read_event();
                return _event;
        }
        
        int LinuxJoystick::count_axes()
        {
                __u8 count;
                if (ioctl(_fd, JSIOCGAXES, &count) == -1)
                        throw std::runtime_error("Joystick: Failed to obtain the "
                                                 "number of axes.");
                return (int) count;
        }

        int LinuxJoystick::count_buttons()
        {
                __u8 count;
                if (ioctl(_fd, JSIOCGBUTTONS, &count) == -1)
                        throw std::runtime_error("Joystick: Failed to obtain the "
                                                 "number of buttons.");
                return (int) count;
        }
}
