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
#include <string.h>
#include "Parser.h"

enum parser_state_t {
        wait_opcode = 0,
        wait_bracket_or_minus_or_digit_or_quote = 1,
        wait_minus_or_digit_scalar = 2,
        wait_minus_digit_or_quote_vector = 3,
        wait_digit_scalar = 4,
        wait_digit_vector = 5,
        wait_digits_or_cr = 6,
        wait_digits_or_comma_or_bracket = 7,
        wait_carriage_return = 8,
        wait_opcode_or_linefeed = 9,
        wait_single_string = 10,
        wait_string_in_vector = 11,
        wait_comma_or_bracket = 12
};

#define MINUS(_c)             ((_c) == '-')
#define QUOTE(_c)             ((_c) == '"')
#define DIGIT(_c)             ((_c) >= '0' && (_c) <= '9')
#define OPENBRACKET(_c)       ((_c) == '[')
#define CLOSEBRACKET(_c)      ((_c) == ']')
#define LINEFEED(_c)          ((_c) == '\n')
#define CARRIAGE_RETURN(_c)   ((_c) == '\r')
#define COMMA(_c)             ((_c) == ',')
#define VALUE(_c)             ((int)((_c) - '0'))
#define OPCODE_W_VALUE(_c)    (strchr(_opcodesWithValue, (_c)) != NULL)
#define OPCODE_WO_VALUE(_c)   (strchr(_opcodesWithoutValue, (_c)) != NULL)
#define VALID_STRING_CHAR(_c) (('a' <= (_c) && (_c) <= 'z') \
                               || ('A' <= (_c) && (_c) <= 'Z') \
                               || ('0' <= (_c) && (_c) <= '9') \
                               || strchr("-_ !?%#()[]{}&=+*/.,;:", (_c)) != NULL)

void Parser::set_error(char what)
{
        _state = wait_opcode;
        _error = what;
        _length = 0;
        _opcode = 0;
        _has_string = 0;
}

int Parser::append_char(char c)
{
        int r = -1;
        if (VALID_STRING_CHAR(c)) {
                if (_string_length < 31) {
                        _string[_string_length] = c;
                        // Assure that the buffer is always zero-terminted
                        _string[_string_length+1] = 0;
                        _string_length++;
                        r = 0;
                } else {
                        set_error(parser_string_too_long);
                        r = -1;
                }
        } else {
                set_error(parser_invalid_string);
                r = -1;
        }
        return r;
}

void Parser::init_value(char c, int16_t sign)
{
        _tmpval = VALUE(c);
        _sign = sign;
}

int Parser::append_value(int32_t v)
{
        int r = -1;
        if (_length < 32) {
                _value[_length++] = v;
                r = 0;
        } else {
                set_error(parser_vector_too_long);
                r = -1;
        }
        return r;
}

int Parser::append_digit(char c)
{
        int r = 0;
        _tmpval = 10 * _tmpval + _sign * VALUE(c);
        if (_tmpval > 32767 || _tmpval < -32768) {
                set_error(parser_value_out_of_range);
                r = -1;
        }
        return r;
}

void Parser::reset_values()
{
        _length = 0;
        unsigned int n = sizeof(_value) / sizeof(int16_t);
        for (unsigned int i = 0; i < n; i++)
                _value[i] = 0;
}

void Parser::reset_string()
{
        _has_string = 0;
        _string_length = 0;
        for (unsigned int i = 0; i < sizeof(_string); i++)
                _string[i] = 0;
}

void Parser::reset()
{
        _state = wait_opcode;
        _error = parser_error_none;
        _opcode = 0;
        reset_values();
        reset_string();
}

bool Parser::process(char c)
{
        bool r = false;
        
        switch(_state) {
        case wait_opcode:
                if (OPCODE_W_VALUE(c)) {
                        reset();
                        _opcode = c;
                        _state = wait_bracket_or_minus_or_digit_or_quote;
                } else if (OPCODE_WO_VALUE(c)) {
                        reset();
                        _opcode = c;
                        _state = wait_carriage_return;
                } else {
                        set_error(parser_unexpected_char);
                }
                break;
                
        case wait_opcode_or_linefeed:
                if (LINEFEED(c)) {
                        // The command was terminated by \r\n
                        _state = wait_opcode; 
                } else {
                        // The command was terminated by \r. The new
                        // character belongs to the next command. Call
                        // process again with the proper state.
                        _state = wait_opcode;
                        process(c);
                }
                break;
                
        case wait_carriage_return:
                if (CARRIAGE_RETURN(c)) {
                        // The command may be terminated by \n, or by \r\n
                        _state = wait_opcode_or_linefeed;
                        r = true; 
                } else if (LINEFEED(c)) {
                        _state = wait_opcode;
                        r = true; // The command was terminated by \n
                } else {
                        set_error(parser_unexpected_char);
                }
                break;

        case wait_bracket_or_minus_or_digit_or_quote:
                if (OPENBRACKET(c)) {
                        _state = wait_minus_digit_or_quote_vector;
              } else if (MINUS(c)) {
                        init_value('0', -1);
                        _state = wait_digit_scalar;
                } else if (DIGIT(c)) {
                        init_value(c, 1);
                        _state = wait_digits_or_cr;
                } else if (QUOTE(c)) {
                        reset_string();
                        _state = wait_single_string;
                } else {
                        set_error(parser_unexpected_char);
                }
                break;
                
        case wait_minus_or_digit_scalar:
                if (MINUS(c)) {
                        init_value('0', -1);
                        _state = wait_digit_scalar;
                } else if (DIGIT(c)) {
                        init_value(c, 1);
                        _state = wait_digits_or_cr;
                } else {
                        set_error(parser_unexpected_char);
                }
                break;
                
        case wait_digit_scalar:
                if (DIGIT(c)) {
                        if (append_digit(c) == 0)
                                _state = wait_digits_or_cr;
                } else {
                        set_error(parser_unexpected_char);
                }
                break;
                
        case wait_digits_or_cr:
                if (DIGIT(c)) {
                        if (append_digit(c) == 0)
                                _state = wait_digits_or_cr;
                } else if (CARRIAGE_RETURN(c)) {
                        if (append_value(_tmpval) == 0)
                                _state = wait_opcode_or_linefeed;
                        r = true;
                } else if (LINEFEED(c)) {
                        if (append_value(_tmpval) == 0)
                                _state = wait_opcode;
                        r = true;
                } else {
                        set_error(parser_unexpected_char);
                }
                break;
                
        case wait_minus_digit_or_quote_vector:
                if (MINUS(c)) {
                        init_value('0', -1);
                        _state = wait_digit_vector;
                } else if (DIGIT(c)) {
                        init_value(c, 1);
                        _state = wait_digits_or_comma_or_bracket;
                } else if (QUOTE(c)) {
                        reset_string();
                        _state = wait_string_in_vector;
                } else {
                        set_error(parser_unexpected_char);
                }
                break;
                
        case wait_digit_vector:
                if (DIGIT(c)) {
                        if (append_digit(c) == 0)
                                _state = wait_digits_or_comma_or_bracket;
                } else {
                        set_error(parser_unexpected_char);
                }
                break;
                
        case wait_digits_or_comma_or_bracket:
                if (DIGIT(c)) {
                        if (append_digit(c) == 0)
                                _state = wait_digits_or_comma_or_bracket;
                } else if (COMMA(c)) {
                        if (append_value(_tmpval) == 0)
                                _state = wait_minus_digit_or_quote_vector;
                } else if (CLOSEBRACKET(c)) {
                        if (append_value(_tmpval) == 0)
                                _state = wait_carriage_return;
                } else {
                        set_error(parser_unexpected_char);
                }
                break;
                
        case wait_single_string:
                if (QUOTE(c)) {
                        _has_string = 1;
                        _state = wait_carriage_return;                        
                } else {
                        if (append_char(c) != 0) 
                                _state = wait_opcode;
                } 
                break;
                
        case wait_string_in_vector:
                if (QUOTE(c)) {
                        _has_string = 1;
                        _state = wait_comma_or_bracket;                        
                } else {
                        if (append_char(c) != 0)
                                _state = wait_opcode;                                
                }
                break;
                
        case wait_comma_or_bracket:
                if (COMMA(c)) {
                        _state = wait_minus_digit_or_quote_vector;                        
                } else if (CLOSEBRACKET(c)) {
                        _state = wait_carriage_return;                        
                } else {
                        append_char(c);
                }
                break;
        }

        return r;
}
