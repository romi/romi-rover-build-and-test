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

#include <sys/types.h>
#include "IInputStream.h"

#ifndef __ROMISERIAL_READER_H
#define __ROMISERIAL_READER_H

#ifndef __ssize_t_defined
#ifndef	_SSIZE_T_DECLARED
// Arduino 1.8.13 doesn't seem to define ssize_t
typedef signed long ssize_t;
#endif
#endif

namespace romiserial {

        class Reader
        {
        protected:
                IInputStream& in_;
        public:
                Reader(IInputStream& in);
                virtual ~Reader() = default;
                size_t read(char *s, size_t length);
                ssize_t readline(char *s, size_t length);
        };
}

#endif // __ROMISERIAL_READER_H
