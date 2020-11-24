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
#include "config.h"

#ifndef _OQUAM_LIMIT_H_
#define _OQUAM_LIMIT_H_

enum {
        LIMIT_X = 0,
        LIMIT_Y = 1,
        LIMIT_Z = 2
};

int reached_limit(int bit);
#define reached_x_limit()  reached_limit(X_LIMIT_BIT)
#define reached_y_limit()  reached_limit(Y_LIMIT_BIT)
//#define reached_z_limit()  reached_limit(Z_LIMIT_BIT)
#define limit_alert()      (get_limit_bits() != LIMIT_MASK)

#endif // _OQUAM_LIMIT_H_
