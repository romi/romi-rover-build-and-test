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
#include "Printer.h"

namespace romiserial {

        Printer::Printer(IOutputStream& out)
                : out_(out)
        {
        }

        size_t Printer::write(const char *s, size_t length)
        {
                size_t n;
                for (n = 0; n < length; n++) {
                        if (!out_.write(s[n]))
                                break;
                }
                return n;
        }

        size_t Printer::print(const char *s)
        {
                size_t length = strlen(s);
                return write(s, length);
        }

        size_t Printer::println(const char *s)
        {
                size_t n = print(s);
                n += write("\n", 1);
                return n;
        }
}
