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
#include <string>
#include <linux/joystick.h>
#include <r.h>
#include "LinuxJoystick.h"

namespace romi {
        
        LinuxJoystick::LinuxJoystick(rpp::ILinux &linux, const char *device)
                : _linux(linux), _fd(-1), _debug(false) {
                
                try_open_device(device);
                try_initialize();
        }
        
        LinuxJoystick::~LinuxJoystick() {
                close_device();
        }

        void LinuxJoystick::try_initialize()
        {
                try {
                        initialize();
                        
                } catch (std::runtime_error& e) {
                        close_device();
                        throw e;
                }
        }

        void LinuxJoystick::initialize()
        {
                int num_axes = try_count_axes();
                _axes.resize(num_axes, 0.0);
                
                int num_buttons = try_count_buttons();
                _buttons.resize(num_buttons, false);
        }

        void LinuxJoystick::try_open_device(const char *name) {
                close_device();
                open_device(name);
                if (_fd < 0) {
                        r_err("Failed to open joystick device: '%s'", name);
                        throw std::runtime_error("Failed to open joystick device");
                }
        }
        
        void LinuxJoystick::open_device(const char *name) {
                r_info("LinuxJoystick: opening device '%s'", name);
                _fd = _linux.open(name, O_RDONLY);
        }

        void LinuxJoystick::close_device() {
                if (_fd != -1) {
                        _linux.close(_fd);
                        _fd = -1;
                }
        }

        void LinuxJoystick::parse_button_event(struct js_event& linux_event)
        {
                _event.type = JoystickEvent::Button;
                _event.number = linux_event.number;
                _buttons[linux_event.number] = linux_event.value != 0;
                        
                if (_debug) {
                        r_debug("button[%d]=%s", linux_event.number,
                                _buttons[linux_event.number]? "pressed" : "released");
                }
        }

        void LinuxJoystick::parse_axis_event(struct js_event& linux_event)
        {
                _event.type = JoystickEvent::Axis;
                _event.number = linux_event.number;
                _axes[linux_event.number] = linux_event.value / 32768.0;

                if (_debug) {
                        r_debug("axis[%d]=%0.3f", linux_event.number,
                                _axes[linux_event.number]);
                }
        }
        
        void LinuxJoystick::parse_event(struct js_event& linux_event)
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
        
        bool LinuxJoystick::has_event(double timeout)
        {
                bool retval = false;
                struct pollfd fds[1];
                fds[0].fd = _fd;
                fds[0].events = POLLIN;
                
                int timeout_ms = (int) (timeout * 1000.0f);
                
                int pollrc = _linux.poll(fds, 1, timeout_ms);
                if (pollrc < 0) {
                        r_err("LinuxJoystick::has_event: poll error %d", errno);
                        throw std::runtime_error("Joystick: poll failed");
                
                } else if ((pollrc > 0) && (fds[0].revents & POLLIN)) {
                        retval = true;
                } 
        
                return retval;
        }
        
        void LinuxJoystick::read_event(struct js_event& linux_event)
        {
                ssize_t bytes = _linux.read(_fd, &linux_event, sizeof(js_event));
                if (bytes != sizeof(js_event)) {
                        r_err("LinuxJoystick::read_event: read failed");
                        throw std::runtime_error("Joystick: read failed");
                }
        }

        void LinuxJoystick::read_and_parse_event()
        {
                struct js_event linux_event;
                if (has_event(0.100)) {
                        read_event(linux_event);
                        parse_event(linux_event);
                }
        }

        JoystickEvent& LinuxJoystick::get_next_event()
        {
                _event.type = JoystickEvent::None;
                read_and_parse_event();
                return _event;
        }
        
        int LinuxJoystick::try_count_axes()
        {
                int count = count_axes();
                if (count == -1) {
                        r_err("LinuxJoystick::try_count_axes: ioctl failed");
                        throw std::runtime_error("Joystick: Failed to obtain the "
                                                 "number of axes.");
                }
                r_info("LinuxJoystick::count_axes: %d", count);
                return count;
        }
        
        int LinuxJoystick::count_axes()
        {
                int retval = -1;
                __u8 count = 0;
                if (_linux.ioctl(_fd, JSIOCGAXES, &count) == 0) {
                        retval = (int) count;
                }
                return retval;
        }
        
        int LinuxJoystick::try_count_buttons()
        {
                int count = count_buttons();
                if (count == -1) {
                        r_err("LinuxJoystick::count_buttons: ioctl failed");
                        throw std::runtime_error("Joystick: Failed to obtain the "
                                                 "number of buttons.");
                }
                r_info("LinuxJoystick::count_buttons: %d", count);
                return count;
        }

        int LinuxJoystick::count_buttons()
        {
                int retval = -1;
                __u8 count = 0;
                if (_linux.ioctl(_fd, JSIOCGBUTTONS, &count) == 0) {
                        retval = (int) count;
                }
                return retval;
        }
}
