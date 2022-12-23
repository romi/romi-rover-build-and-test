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
#ifndef __ROMISERIAL_MESSAGEPARSER_H
#define __ROMISERIAL_MESSAGEPARSER_H

#include <stdint.h>

namespace romiserial {

#define PARSER_MAXIMUM_ARGUMENTS 12
#define PARSER_MAXIMUM_STRING_LENGTH 32

        enum message_parser_state_t {
                wait_opcode,
                wait_bracket_or_end,
                wait_value,
                wait_digit,
                wait_digits_or_comma_or_bracket,
                wait_end_message,
                wait_string,
                wait_comma_or_bracket
        };

        class MessageParser
        {
        protected:
                message_parser_state_t _state;
                int8_t _error;
                char _opcode;
                int16_t _value[PARSER_MAXIMUM_ARGUMENTS];
                uint8_t _length;
                bool _has_string;
                char _string[PARSER_MAXIMUM_STRING_LENGTH+1];
                uint8_t _string_length;
                int16_t _tmpval;
                int16_t _sign;

                void set_error(char character, int8_t what);
                void reset_string();
                void reset_values();
                void reset();
                void append_char(char c);
                int append_value(int16_t v);
                void init_value(char c, int16_t sign);
                int append_digit(char c);
                uint8_t hex_to_int(char c);
                void crc8_update(uint8_t c);

                bool process(char c);

        public:
        
                MessageParser();        
                ~MessageParser() = default;
                
                int16_t value(int index = 0) {
                        return (index < _length)? _value[index] : 0;
                }
        
                int16_t *values() {
                        return _value;
                }

                uint8_t length() {
                        return _length;
                }
        
                bool has_string() {
                        return _has_string;
                }

                char *string() {
                        return _string;
                }

                char opcode() {
                        return _opcode;
                }
        
                int8_t error() {
                        return _error;
                }
        
                bool parse(const char *s, size_t len);
        };
}

#endif // __ROMISERIAL_MESSAGEPARSER_H
