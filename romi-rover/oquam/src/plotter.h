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

#ifndef _OQUAM_PLOTTER_H_
#define _OQUAM_PLOTTER_H_

#include "script.h"

#ifdef __cplusplus
extern "C" {
#endif

void print_to_stdout(script_t *script,
                     double *xmin,
                     double *xmax,
                     double *vmax_,
                     double *amax,
                     double *scale);

int plot_to_file(const char *filepath,
                 script_t *script,
                 double *xmin,
                 double *xmax,
                 double *vmax,
                 double *amax,
                 double *scale);

membuf_t *plot_to_mem(script_t *script,
                      double *xmin,
                      double *xmax,
                      double *vmax,
                      double *amax,
                      double *scale);
        
#ifdef __cplusplus
}
#endif

#endif // _OQUAM_PLOTTER_H_
