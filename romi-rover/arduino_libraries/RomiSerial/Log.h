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

#ifndef __LOG_H
#define __LOG_H

#if defined(ARDUINO)
#include <Arduino.h>
#define log_print(__x)   { \
                Serial.print("#!"); \
                Serial.print(__x); ; \
                Serial.println(":xxxx"); }

#else
#include <r.h>
static inline void romiserial_log(char x) { r_debug("#!%c:xxxx", x); }
static inline void romiserial_log(int x) { r_debug("#!%d:xxxx", x); }
static inline void romiserial_log(const char *s) { r_debug("#!%s:xxxx", s); }

#define log_print(__x)   romiserial_log(__x)
#endif


#endif // __LOG_H
