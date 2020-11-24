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
#include "CRC8.h"

#define PARSER_MAXIMUM_ARGUMENTS 12
#define PARSER_MAXIMUM_STRING_LENGTH 32

class Parser : public IParser
{
protected:
        uint8_t _state;
        uint8_t _count;
        int8_t _error;
        uint8_t _opcode;
        uint8_t _id;
        uint8_t _has_id;
        CRC8 _crc;
        uint8_t _crc_metadata;
        int16_t _value[PARSER_MAXIMUM_ARGUMENTS];
        uint8_t _length;
        bool _has_string;
        char _string[PARSER_MAXIMUM_STRING_LENGTH+1];
        uint8_t _string_length;
        int32_t _tmpval;
        int16_t _sign;

        void set_error(char character, char what);
        void reset();
        void reset_string();
        void reset_values();
        void append_char(char c);
        int append_value(int32_t v);
        void init_value(char c, int16_t sign);
        int append_digit(char c);
        uint8_t hex_to_int(char c);
        void crc8_update(uint8_t c);

public:
        
        Parser() {
                reset();
        }
        
        virtual int16_t value(int index = 0) {
                return (index < _length)? _value[index] : 0;
        }
        
        virtual int16_t *values() {
                return _value;
        }

        virtual uint8_t length() {
                return _length;
        }
        
        virtual bool has_string() {
                return _has_string;
        }

        virtual char *string() {
                return _string;
        }

        virtual uint8_t opcode() {
                return _opcode;
        }

        virtual uint8_t id() {
                return _id;
        }

        virtual uint8_t has_id() {
                return _has_id;
        }

        virtual uint8_t crc() {
                return _crc.get();
        }
        
        virtual int8_t error() {
                return _error;
        }
        
        virtual bool process(char c);
};

#endif
