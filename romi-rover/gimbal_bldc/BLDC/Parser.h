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
#ifndef __PARSER_H
#define __PARSER_H

#include "IParser.h"

enum parser_error_t {
        parser_error_none = 0,
        parser_unexpected_char = 1,
        parser_vector_too_long = 2,
        parser_value_out_of_range = 3,
        parser_string_too_long = 4,
        parser_invalid_string = 5
};

class Parser : public IParser
{
protected:
        char _state;
        char _error;
        char _opcode;
        int16_t _value[32];
        int _length;
        char _has_string;
        char _string[32];
        unsigned char _string_length;
        int32_t _tmpval;
        int16_t _sign;
        const char* _opcodesWithValue;
        const char* _opcodesWithoutValue;

        void set_error(char what);
        void reset();
        void reset_string();
        void reset_values();
        int append_char(char c);
        int append_value(int32_t v);
        void init_value(char c, int16_t sign);
        int append_digit(char c);

public:
        
        Parser(const char* opcodesWithValue,
               const char* opcodesWithoutValue)
                : _opcodesWithValue(opcodesWithValue), 
                  _opcodesWithoutValue(opcodesWithoutValue) {
                reset();
        }
        
        virtual int16_t value(int index = 0) {
                return (index < _length)? _value[index] : 0;
        }

        virtual int length() {
                return _length;
        }
        
        virtual char has_string() {
                return _has_string;
        }

        virtual char *string() {
                return _string;
        }

        virtual char opcode() {
                return _opcode;
        }
        
        virtual char error() {
                return _error;
        }
        
        virtual bool process(char c);
};

#endif
