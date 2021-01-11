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

#ifndef _OQUAM_IS_VALID_H_
#define _OQUAM_IS_VALID_H_

#include "CNCRange.h"
#include "oquam/SmoothPath.h"

namespace romi {
        
        bool is_valid(Section& section, const char *name, double tmax,
                      const double *xmin, const double *xmax, 
                      const double *vmax, const double *amax);

        bool is_valid(ATDC& atdc, double tmax,
                      double *xmin, double *xmax, 
                      double *vmax, double *amax);

        bool is_valid(SmoothPath& script, double tmax, CNCRange& range, 
                      double *vmax, double *amax);
       
}

#endif // _OQUAM_IS_VALID_H_



