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
#include <HardwareSerial.h>
#include "Parser.h"

enum parser_state_t {
        wait_opcode = 0,
        wait_bracket_sign_digit_or_quote = 1,
        wait_sign_or_digit_scalar = 2,
        wait_sign_or_digit_vector = 3,
        wait_digit_scalar = 4,
        wait_digit_vector = 5,
        wait_digits_or_separator = 6,
        wait_digits_comma_or_bracket = 7,
        wait_separator = 8,
        wait_text_or_quote = 9
};

#define SIGN(_c)             ((_c) == '-')
#define DIGIT(_c)            ((_c) >= '0' && (_c) <= '9')
#define OPENBRACKET(_c)      ((_c) == '[')
#define CLOSEBRACKET(_c)     ((_c) == ']')
#define SEPARATOR(_c)        ((_c) == ' ' || (_c) == '\n' || (_c) == '\r')
#define COMMA(_c)            ((_c) == ',')
#define VALUE(_c)            ((int)((_c) - '0'))
#define OPCODE_W_VALUE(_c)   (strchr(_opcodesWithValue, (_c)) != NULL)
#define OPCODE_WO_VALUE(_c)  (strchr(_opcodesWithoutValue, (_c)) != NULL)
#define QUOTE(_c)             ((_c) == '"')

bool Parser::process(char c)
{
        switch(_state) {
        case wait_opcode:
                //Serial.print("(wait_opcode,'"); Serial.print(c); Serial.print("')=");
                if (OPCODE_W_VALUE(c)) {
                        _opcode = c;
                        _length = 0;
                        _textlen = 0;
                        _text[0] = '\0';
                        _state = wait_bracket_sign_digit_or_quote;
                        //Serial.println("wait_bracket_sign_digit_or_quote");
                        return false;
                } else if (OPCODE_WO_VALUE(c)) {
                        _opcode = c;
                        _length = 0;
                        _textlen = 0;
                        _text[0] = '\0';
                        _state = wait_separator;
                        //Serial.println("wait_separator");
                        return false;
                } else {
                        _state = wait_opcode;
                        //Serial.println("wait_opcode");
                        return false;
                }
                break;
        case wait_separator:
                //Serial.print("(wait_separator,'"); Serial.print(c); Serial.print("')=");
                if (SEPARATOR(c)) {
                        _state = wait_opcode;
                        //Serial.println("wait_opcode");
                        return true;
                } else {
                        _state = wait_opcode;
                        //Serial.println("wait_opcode");
                        return false;
                }
                break;
        case wait_bracket_sign_digit_or_quote:
                //Serial.print("(wait_bracket_sign_digit_or_quote,'"); Serial.print(c); Serial.print("')=");
                if (OPENBRACKET(c)) {
                        _tmpval = 0;
                        _sign = -1;
                        _state = wait_sign_or_digit_vector;
                        //Serial.println("wait_sign_or_digit_vector");
                        return false;
                } else if (SIGN(c)) {
                        _tmpval = 0;
                        _sign = -1;
                        _state = wait_digit_scalar;
                        //Serial.println("wait_digit_scalar");
                        return false;
                } else if (DIGIT(c)) {
                        _tmpval = VALUE(c);
                        _sign = 1;
                        _state = wait_digits_or_separator;
                        //Serial.println("wait_digits_or_separator");
                        return false;
                } else if (QUOTE(c)) {
                        _state = wait_text_or_quote;
                        _textlen = 0;
                        //Serial.println("wait_text_or_quote");
                        return false;
                } else {
                        _state = wait_opcode;
                        //Serial.println("wait_opcode");
                        return false;
                }
                break;
        case wait_sign_or_digit_scalar:
                //Serial.print("(wait_sign_or_digit_scalar,'"); Serial.print(c); Serial.print("')=");
                if (SIGN(c)) {
                        _tmpval = 0;
                        _sign = -1;
                        _state = wait_digit_scalar;
                        //Serial.println("wait_digit_scalar");
                        return false;
                } else if (DIGIT(c)) {
                        _tmpval = VALUE(c);
                        _sign = 1;
                        _state = wait_digits_or_separator;
                        //Serial.println("wait_digits_or_separator");
                        return false;
                } else {
                        _state = wait_opcode;
                        //Serial.println("wait_opcode");
                        return false;
                }
                break;
        case wait_digit_scalar:
                //Serial.print("(wait_digit_scalar,'"); Serial.print(c); Serial.print("')=");
                if (DIGIT(c)) {
                        _tmpval = _sign * VALUE(c);
                        _state = wait_digits_or_separator;
                        //Serial.println("wait_digits_or_separator");
                        return false;
                } else {
                        _state = wait_opcode;
                        //Serial.println("wait_opcode");
                        return false;
                }
                break;
        case wait_digits_or_separator:
                //Serial.print("(wait_digits_or_separator,'"); Serial.print(c); Serial.print("')=");
                if (DIGIT(c)) {
                        _tmpval = 10 * _tmpval + _sign * VALUE(c);
                        _state = wait_digits_or_separator;
                        //Serial.println("wait_digits_or_separator");
                        return false;
                } else if (SEPARATOR(c)) {
                        _value[0] = _tmpval;
                        _length = 1;
                        _state = wait_opcode;
                        //Serial.println("wait_opcode");
                        return true;
                } else {
                        _state = wait_opcode;
                        //Serial.println("wait_opcode");
                        return false;
                }
                break;
        case wait_sign_or_digit_vector:
                //Serial.print("(wait_sign_or_digit_vector,'"); Serial.print(c); Serial.print("')=");
                if (SIGN(c)) {
                        _tmpval = 0;
                        _sign = -1;
                        _state = wait_digit_vector;
                        //Serial.println("wait_digit_vector");
                        return false;
                } else if (DIGIT(c)) {
                        _tmpval = VALUE(c);
                        _sign = 1;
                        _state = wait_digits_comma_or_bracket;
                        //Serial.println("wait_digits_comma_or_bracket");
                        return false;
                } else {
                        _state = wait_opcode;
                        //Serial.println("wait_opcode");
                        return false;
                }
                break;
        case wait_digit_vector:
                //Serial.print("(wait_digit_vector,'"); Serial.print(c); Serial.print("')=");
                if (DIGIT(c)) {
                        _tmpval = _sign * VALUE(c);
                        _state = wait_digits_comma_or_bracket;
                        //Serial.println("wait_digits_comma_or_bracket");
                        return false;
                } else {
                        _state = wait_opcode;
                        //Serial.println("wait_opcode");
                        return false;
                }
                break;
        case wait_digits_comma_or_bracket:
                //Serial.print("(wait_digits_comma_or_bracket,'"); Serial.print(c); Serial.print("')=");
                if (DIGIT(c)) {
                        _tmpval = 10 * _tmpval + _sign * VALUE(c);
                        _state = wait_digits_comma_or_bracket;
                        //Serial.println("wait_digits_comma_or_bracket");
                        return false;
                } else if (COMMA(c)) {
                        if (_length < 32)
                                _value[_length++] = _tmpval;
                        _state = wait_sign_or_digit_vector;
                        //Serial.println("wait_sign_or_digit_vector");
                        return false;
                } else if (CLOSEBRACKET(c)) {
                        if (_length < 32)
                                _value[_length++] = _tmpval;
                        _state = wait_opcode;
                        //Serial.println("wait_opcode");
                        return true;
                } else {
                        _state = wait_opcode;
                        //Serial.println("wait_opcode");
                        return false;
                }
                break;
        case wait_text_or_quote:
                //Serial.print("(wait_text_or_quote,'"); Serial.print(c); Serial.print("')=");
                if (QUOTE(c)) {
                        //Serial.println("wait_opcode");
                        _state = wait_opcode;
                        if (_textlen < sizeof(_text))
                                _text[_textlen] = '\0';
                        else
                                _text[sizeof(_text)-1] = '\0';                        
                        //Serial.println(_text);
                        return true;
                } else {
                        //Serial.println("wait_text_or_quote");
                        if (_textlen < sizeof(_text))
                                _text[_textlen++] = c;
                }
                break;
        default:
                return false;
        }
}
