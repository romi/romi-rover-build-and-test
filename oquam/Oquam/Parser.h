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

class Parser
{
protected:
        char _state;
        char _opcode;
        int16_t _value[8];
        char _length;
        char _text[32];
        unsigned char _textlen;
        int _tmpval;
        int _sign;
        const char* _opcodesWithValue;
        const char* _opcodesWithoutValue;

public:
        
        Parser(const char* opcodesWithValue,
               const char* opcodesWithoutValue) {
                _opcodesWithValue = opcodesWithValue;
                _opcodesWithoutValue = opcodesWithoutValue;
                _state = 0;
                _text[0] = 0;
                _text[127] = 0;
        }

        int value(int index = 0) {
                return _value[index];
        }

        char *text() {
                return _text;
        }

        int length() {
                return _length;
        }

        char opcode() {
                return _opcode;
        }
        
        bool process(char c);
};

#endif
