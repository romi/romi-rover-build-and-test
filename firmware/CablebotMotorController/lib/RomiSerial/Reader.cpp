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

#include <string.h>
#include "Reader.h"

namespace romiserial {

        Reader::Reader(IInputStream& in)
                : in_(in)
        {
        }

        size_t Reader::read(char *s, size_t length)
        {
                char c;
                size_t n = 0;
                for (n = 0; n < length; n++) {
                        if (in_.read(c))
                                s[n] = c;
                        else
                                break;
                }
                return n;
        }

        ssize_t Reader::readline(char *s, size_t length)
        {
                char c;
                ssize_t n = 0;
                while (true) {
                        
                        if (in_.read(c)) {
                                if (c == '\n') {
                                        s[n] = '\0';
                                        break;
                                } else {
                                        s[n] = c;
                                }
                        } else {
                                break;
                        }
                        
                        if (++n == (ssize_t) length) {
                                n = -1;
                                break;
                        } 
                }
                return n;
        }
}
