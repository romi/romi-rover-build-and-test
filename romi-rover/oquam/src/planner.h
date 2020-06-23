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
#include "script.h"

#ifndef _OQUAM_PLANNER_H_
#define _OQUAM_PLANNER_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _planner_t planner_t;

int planner_convert_script(script_t *script, double *position,
                           double *vmax, double *amax, double deviation);

int planner_slice(script_t *script, double period, double maxlen);

int planner_compile_for_stepper(script_t *script, double *scale);


#ifdef __cplusplus
}
#endif

#endif // _OQUAM_PLANNER_H_
