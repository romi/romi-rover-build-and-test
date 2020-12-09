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
#ifndef _ROMI_POINT_H_
#define _ROMI_POINT_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _point_t {
        float x, y, z;
} point_t;

point_t *new_point(float x, float y, float z);
void delete_point(point_t *p);
void point_set(point_t *p, float x, float y, float z);
        
#ifdef __cplusplus
}
#endif

#endif // _ROMI_POINT_H_
