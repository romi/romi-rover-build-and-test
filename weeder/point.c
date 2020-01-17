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
#include <r.h>
#include "point.h"

point_t *new_point(float x, float y, float z)
{
        point_t *p = r_new(point_t);
        if (p == NULL)
                return NULL;
        p->x = x;
        p->y = y;
        p->z = z;
        return p;
}

void delete_point(point_t *p)
{
        if (p)
                r_delete(p);
}

void point_set(point_t *p, float x, float y, float z)
{
        p->x = x;
        p->y = y;
        p->z = z;
}
