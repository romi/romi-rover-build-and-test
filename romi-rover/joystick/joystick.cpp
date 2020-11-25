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
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <linux/joystick.h>
#include <r.h>
#include <rcom.h>
#include "joystick.h"

messagehub_t *get_messagehub_joystick();
datahub_t *get_datahub_joystick();


class Joystick
{
protected:
        int _fd;
        std::string _device;
        membuf_t *_message;
        
        void close() {
                if (_fd != -1) {
                        ::close(_fd);
                        _fd = -1;
                }
        }
        
        bool open() {
                close();
                _fd = ::open(_device.c_str(), O_RDONLY);
                return (_fd >= 0);
        }
        
        bool convert_event(struct js_event &event, std::string &message) {
                bool success = true;
                event.type &= ~JS_EVENT_INIT;
                        
                membuf_clear(_message);
                
                switch (event.type) {
                case JS_EVENT_BUTTON:
                        membuf_printf(_message,
                                      "{\"time\": %0.6f, "
                                      "\"type\": \"button\", "
                                      "\"button\": %d, "
                                      "\"state\": \"%s\"}",
                                      (double) event.time / 1000.0,
                                      event.number,
                                      event.value ? "pressed" : "released");
                        membuf_append_zero(_message);
                        message = membuf_data(_message);
                        break;
                        
                case JS_EVENT_AXIS:
                        membuf_printf(_message,
                                      "{\"time\": %0.6f, "
                                      "\"type\": \"axis\", "
                                      "\"axis\": %d, "
                                      "\"value\": %f }",
                                      (double) event.time / 1000.0,
                                      event.number,
                                      (double) event.value / 32767.0);
                        membuf_append_zero(_message);
                        message = membuf_data(_message);
                        break;
                        
                default: // NOP
                        success = false;
                }
                
                return success;
        }
        
public:
        Joystick() : _fd(-1) {
                _message = new_membuf();
                membuf_append_zero(_message);
        }
        
        virtual ~Joystick() {
                close();
                if (_message)
                        delete_membuf(_message);
        }
        
        bool set_device(const char *dev) {
                _device = dev;
                return open();
        }

        bool is_open() {
                return _fd != -1;
        }

        // Retuns a JSON string representing the event.
        bool get_event(std::string &message) {
                bool has_event = false;
                struct js_event event;
                
                ssize_t bytes = read(_fd, &event, sizeof(js_event));
                if (bytes == sizeof(js_event))
                        has_event = convert_event(event, message);

                return has_event;
        }
};

Joystick joystick;
        
bool get_configuration()
{
        int r = false;
        
        json_object_t port = client_get("configuration", "ports/joystick/port");
        
        if (json_isstring(port)) {
                if (joystick.set_device(json_string_value(port))) {
                        r = true;
                } else {
                        r_err("bad value for 'ports' in the configuration");
                }
        } else {
                r_err("missing value for 'ports' in the configuration");
        }
        
        json_unref(port);

        return r;
}

int joystick_init(int argc, char **argv)
{
        int r = -1;
        
        if (argc > 1 && joystick.set_device(argv[1])) {
                r = 0;
        } else if (get_configuration()) {
                r = 0;
        } else {
                r_err("no device given");
        }
 
        return r;
}

void joystick_cleanup()
{
}

void joystick_handle_events()
{
        if (joystick.is_open()) {
                
                std::string message;
                messagehub_t *messagehub = get_messagehub_joystick();
                datahub_t *datahub = get_datahub_joystick();
                
                if (joystick.get_event(message)) { 
                        messagehub_broadcast_text(messagehub, 0,
                                                  message.c_str(), message.size());
                        datahub_broadcast_bin(datahub, 0,
                                              message.c_str(), message.size());
                }
        } else {
                clock_sleep(0.1);
        }
}


