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
#ifndef __IPARSER_H
#define __IPARSER_H

#include <stdint.h>

class IParser
{
public:
        virtual ~IParser() {}

        virtual int16_t value(int index = 0) = 0;
        virtual int length() = 0;
        virtual char has_string() = 0;
        virtual char *string() = 0;
        virtual char opcode() = 0;
        virtual char error() = 0;        
        virtual bool process(char c) = 0;
};

#endif
