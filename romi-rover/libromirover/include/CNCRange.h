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

#ifndef __ROMI_CNC_RANGE_H
#define __ROMI_CNC_RANGE_H

#include "JsonCpp.h"
#include "v.h"

namespace romi {
        
        struct CNCRange
        {
                v3 min;
                v3 max;
                
                CNCRange();
                CNCRange(JsonCpp &range);
                CNCRange(const double *min, const double *max);
                CNCRange(v3 min, v3 max);
                
                void init(JsonCpp &range);
                
                v3 dimensions();
                
                bool is_inside(double x, double y, double z);
                bool is_inside(v3 p);
                
                // Computes the distance of a point that lies outside
                // the range to the border of the range.
                double error(double x, double y, double z);
                double error(v3 p);

                v3 clamp(v3 p);
        };
}

#endif // __ROMI_CNC_RANGE_H
