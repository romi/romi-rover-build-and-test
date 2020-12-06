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
//#include <stdio.h>
#include <string.h>
#include "MessageParser.h"
#include "RomiSerialErrors.h"
#include "Log.h"

enum parser_state_t {
        wait_opcode,
        wait_bracket_or_end,
        wait_value,
        wait_digit,
        wait_digits_or_comma_or_bracket,
        wait_end_message,
        wait_string,
        wait_comma_or_bracket,
        wait_id1, wait_id2,
        wait_crc1, wait_crc2
};

#define START_MESSAGE(_c)     ((_c) == '#')
#define END_MESSAGE(_c)       ((_c) == '\0')
#define MINUS(_c)             ((_c) == '-')
#define QUOTE(_c)             ((_c) == '"')
#define DIGIT(_c)             ((_c) >= '0' && (_c) <= '9')
#define OPENBRACKET(_c)       ((_c) == '[')
#define CLOSEBRACKET(_c)      ((_c) == ']')
#define CARRIAGE_RETURN(_c)   ((_c) == '\r')
#define COMMA(_c)             ((_c) == ',')
#define START_METADATA(_c)    ((_c) == ':')
#define VALUE(_c)             ((int)((_c) - '0'))
#define VALID_OPCODE(_c)      (('a' <= (_c) && (_c) <= 'z') \
                               || ('A' <= (_c) && (_c) <= 'Z')  \
                               || ('0' <= (_c) && (_c) <= '9')  \
                               || ((_c) == '?'))
#define VALID_STRING_CHAR(_c) (('a' <= (_c) && (_c) <= 'z') \
                               || ('A' <= (_c) && (_c) <= 'Z') \
                               || ('0' <= (_c) && (_c) <= '9') \
                               || strchr("-_ !?%()[]{}&=+*/.,;:'", (_c)) != NULL)
#define VALID_HEX_CHAR(_c)    (('a' <= (_c) && (_c) <= 'f') \
                               || ('0' <= (_c) && (_c) <= '9'))

void MessageParser::set_error(char character, char what)
{
        _error = what;
        _state = wait_opcode;
}

void MessageParser::append_char(char c)
{
        if (VALID_STRING_CHAR(c)) {
                if (_string_length < PARSER_MAXIMUM_STRING_LENGTH) {
                        _string[_string_length++] = c;
                        // Assure that the buffer is always zero-terminted
                        _string[_string_length] = 0;
                } else {
                        set_error(c, romiserial_string_too_long);
                }
        } else {
                set_error(c, romiserial_invalid_string);
        }
}

void MessageParser::init_value(char c, int16_t sign)
{
        _tmpval = VALUE(c);
        _sign = sign;
}

int MessageParser::append_value(int32_t v)
{
        int r = -1;
        if (_length < PARSER_MAXIMUM_ARGUMENTS) {
                _value[_length++] = v;
                r = 0;
        } else {
                set_error('?', romiserial_vector_too_long);
                r = -1;
        }
        return r;
}

int MessageParser::append_digit(char c)
{
        int r = 0;
        _tmpval = 10 * _tmpval + _sign * VALUE(c);
        if (_tmpval > 32767 || _tmpval < -32768) {
                log_print("Out of range");
                log_print(_tmpval);
                set_error(c, romiserial_value_out_of_range);
                r = -1;
        }
        return r;
}

void MessageParser::reset_values()
{
        _length = 0;
        for (unsigned int i = 0; i < PARSER_MAXIMUM_ARGUMENTS; i++)
                _value[i] = 0;
}

void MessageParser::reset_string()
{
        _has_string = 0;
        _string_length = 0;
        for (unsigned int i = 0; i < PARSER_MAXIMUM_STRING_LENGTH+1; i++)
                _string[i] = 0;
}

void MessageParser::reset()
{
        _state = wait_opcode;
        _error = romiserial_error_none;
        _opcode = 0;
        reset_values();
        reset_string();
}

bool MessageParser::parse(const char *s, int len)
{
        bool success = false;
        reset();
        for (int i = 0; i < len-1; i++) {
                process(s[i]);
                if (_error != 0)
                        break;
        }
        if (_error == 0)
                success = process(s[len-1]);
        return success;
}

bool MessageParser::process(char c)
{
        bool has_request = false;

        _error = 0;
        
        switch(_state) {
        case wait_opcode:
                if (VALID_OPCODE(c)) {
                        _opcode = c;
                        _state = wait_bracket_or_end;
                } else {
                        set_error(c, romiserial_invalid_opcode);
                }
                break;
                
        case wait_bracket_or_end:
                if (OPENBRACKET(c)) {
                        _state = wait_value;
                } else if (END_MESSAGE(c)) {
                        has_request = true;
                } else {
                        set_error(c, romiserial_unexpected_char);
                }
                break;

        case wait_end_message:
                if (END_MESSAGE(c)) {
                        _state = wait_opcode;
                        has_request = true;
                } else {
                        set_error(c, romiserial_unexpected_char);
                }
                break;
                                                
        case wait_value:
                if (MINUS(c)) {
                        init_value('0', -1);
                        _state = wait_digit;
                } else if (DIGIT(c)) {
                        init_value(c, 1);
                        _state = wait_digits_or_comma_or_bracket;
                } else if (QUOTE(c)) {
                        if (_has_string) {
                                set_error(c, romiserial_too_many_strings);
                        } else {
                                _state = wait_string;
                        }
                } else {
                        set_error(c, romiserial_unexpected_char);
                }
                break;
                
        case wait_digit:
                if (DIGIT(c)) {
                        if (append_digit(c) == 0)
                                _state = wait_digits_or_comma_or_bracket;
                } else {
                        set_error(c, romiserial_unexpected_char);
                }
                break;
                
        case wait_digits_or_comma_or_bracket:
                if (DIGIT(c)) {
                        if (append_digit(c) == 0)
                                _state = wait_digits_or_comma_or_bracket;
                } else if (COMMA(c)) {
                        if (append_value(_tmpval) == 0)
                                _state = wait_value;
                } else if (CLOSEBRACKET(c)) {
                        if (append_value(_tmpval) == 0)
                                _state = wait_end_message;
                } else {
                        set_error(c, romiserial_unexpected_char);
                }
                break;
                                
        case wait_string:
                if (QUOTE(c)) {
                        _has_string = 1;
                        _state = wait_comma_or_bracket;                        
                } else if (END_MESSAGE(c)) {
                        set_error(c, romiserial_invalid_string);
                } else {
                        append_char(c);
                }
                break;
                
        case wait_comma_or_bracket:
                if (COMMA(c)) {
                        _state = wait_value;                        
                } else if (CLOSEBRACKET(c)) {
                        _state = wait_end_message;                        
                } else {
                        set_error(c, romiserial_unexpected_char);
                }
                break;
        }
        
        return has_request;
}
