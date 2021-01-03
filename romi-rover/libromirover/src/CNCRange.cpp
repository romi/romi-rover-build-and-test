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

#include "CNCRange.h"

namespace romi {
        
        CNCRange::CNCRange() : min(0.0), max(0.0) {
        }
        
        CNCRange::CNCRange(const double *xmin, const double *xmax)
                : min(xmin), max(xmax) {
        }
        
        CNCRange::CNCRange(v3 xmin, v3 xmax)
                : min(xmin), max(xmax) {
        }
        
        CNCRange::CNCRange(JsonCpp& json)
        {
                init(json);
        }
        
        void CNCRange::init(JsonCpp& json)
        {
                for (int i = 0; i < 3; i++) {
                        min.set(i, json.array(i).num(0));
                        max.set(i, json.array(i).num(1));
                }
        }

        v3 CNCRange::dimensions()
        {
                return max - min;
        }
        
        bool CNCRange::is_inside(double x, double y, double z)
        {
                return ((x >= min.x()) && (x <= max.x())
                        && (y >= min.y()) && (y <= max.y())
                        && (z >= min.z()) && (z <= max.z()));
        }

        bool CNCRange::is_inside(v3 p)
        {
                return is_inside(p.x(), p.y(), p.z());
        }
        
        double CNCRange::error(double x, double y, double z)
        {
                double dx[3] = { 0.0, 0.0, 0.0 };
        
                if (x < min.x())
                        dx[0] = min.x() - x;
                else if (x > max.x())
                        dx[0] = x - max.x();
                
                if (y < min.y())
                        dx[1] = min.y() - y;
                else if (y > max.y())
                        dx[1] = y - max.y();
                
                if (z < min.z())
                        dx[2] = min.z() - z;
                else if (z > max.z())
                        dx[2] = z - max.z();
        
                return vnorm(dx);
        }

        double CNCRange::error(v3 v)
        {
                return error(v.x(), v.y(), v.z());
        }
        
        v3 CNCRange::clamp(v3 p)
        {
                return p.clamp(min, max);
        }
}
