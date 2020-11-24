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

#include "RomiSerial.h"
#include "RomiSerialErrors.h"
#include <stdio.h>

void RomiSerial::handle_input()
{
        if (_in == 0) 
                _in = get_default_input();
        
        while (_in->available()) {
                int c = _in->read();
                if (c > 0) {
                        bool has_message = _parser.process((char) c);
                        if (has_message) {
                                if (_parser.has_id()
                                    && _parser.id() != _last_id) {
                                        handle_message();
                                } else {
                                        send_error(romiserial_duplicate, 0);
                                }
                        } else if (_parser.error() != 0) {
                                send_error(_parser.error(), 0);
                        }
                }
        }
}

void RomiSerial::handle_message()
{
        bool handled = false;
        _sent_response = false;
        for (uint8_t i = 0; i < _num_handlers; i++) {
                if (_parser.opcode() == _handlers[i].opcode) {
                        if (_parser.length() == _handlers[i].number_arguments) {
                                if (_parser.has_string() == _handlers[i].requires_string) {
                                        _handlers[i].callback(this,
                                                              _parser.values(),
                                                              _parser.string());
                                } else {
                                        if (_handlers[i].requires_string)
                                                send_error(romiserial_missing_string, 0);
                                        else
                                                send_error(romiserial_bad_string, 0);
                                }
                        } else {
                                send_error(romiserial_bad_number_of_arguments, 0);                       
                        }
                        handled = true;
                        break;
                }
        }
        if (!handled) {
                send_error(romiserial_unknown_opcode, 0);
        } else if (!_sent_response) {
                send_error(romiserial_bad_handler, 0);
        }
}

void RomiSerial::append_char(const char c)
{
        if (_out == 0)
                _out = get_default_output();
        _crc.update(c);
        _out->write(c);
}

void RomiSerial::append_message(const char *s)
{
        while (*s != '\0')
                append_char(*s++);
}

void RomiSerial::start_message()
{
        _crc.start();
        append_char('#');
        if (_parser.opcode()) 
                append_char(_parser.opcode());
        else
                append_char('_');
}

void RomiSerial::finalize_message()
{
        append_char(':');
        append_id();
        append_crc();
        append_char('\r');
        append_char('\n');
}

void RomiSerial::send_error(int code, const char *message)
{
        char buffer[128];
        if (message) 
                snprintf(buffer, sizeof(buffer), "[%d,\"%s\"]", code, message);
        else
                snprintf(buffer, sizeof(buffer), "[%d]", code);
        start_message();
        append_message(buffer);
        finalize_message();
        _sent_response = true;
}

void RomiSerial::send_ok()
{
        start_message();
        append_message("[0]");
        finalize_message();
        _sent_response = true;
        _last_id = _parser.id();
}

void RomiSerial::send(const char *message)
{
        start_message();
        append_message(message);
        finalize_message();
        _sent_response = true;
        _last_id = _parser.id();
}

void RomiSerial::log(const char *message)
{
        if (_out == 0)
                _out = get_default_output();
        _out->write('!');
        _out->print(message);
        _out->write('\r');
        _out->write('\n');
}
