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
#include "script_priv.h"

#ifndef _OQUAM_SCRIPT_H_
#define _OQUAM_SCRIPT_H_

#ifdef __cplusplus
extern "C" {
#endif

script_t *new_script();
void delete_script(script_t *script);

/**
 * \brief Add a move action to the current script.
 *
 * Move to absolute position (x,y,z) in meters at a speed of v m/s.
 */
int script_moveto(script_t *script, double x, double y, double z, double v);


#ifdef __cplusplus
}
#endif

#endif // _OQUAM_SCRIPT_H_
