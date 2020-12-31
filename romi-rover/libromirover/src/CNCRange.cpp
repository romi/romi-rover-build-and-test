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
        
        CNCRange::CNCRange()
        {
                for (int i = 0; i < 2; i++) {
                        _x[i] = 0.0;
                        _y[i] = 0.0;
                        _z[i] = 0.0;
                }
        }

        CNCRange::CNCRange(double x0, double x1,
                           double y0, double y1,
                           double z0, double z1)
        {
                _x[0] = x0;
                _x[1] = x1;
                _y[0] = y0;
                _y[1] = y1;
                _z[0] = z0;
                _z[1] = z1;
        }
        
        CNCRange::CNCRange(const double *xmin, const double *xmax)
        {
                _x[0] = xmin[0];
                _y[0] = xmin[1];
                _z[0] = xmin[2];
                _x[1] = xmax[0];
                _y[1] = xmax[1];
                _z[1] = xmax[2];
        }
                
        void CNCRange::init(JsonCpp& range)
        {
                for (int i = 0; i < 2; i++) {
                        _x[i] = range.array(0).num(i);
                        _y[i] = range.array(1).num(i);
                        _z[i] = range.array(2).num(i);
                }
        }

        bool CNCRange::is_valid(double x, double y, double z)
        {
                return ((x >= _x[0]) && (x <= _x[1])
                        && (y >= _y[0]) && (y <= _y[1])
                        && (z >= _z[0]) && (z <= _z[1]));
        }
        
        double CNCRange::error(double x, double y, double z)
        {
                double dx = 0.0;
                double dy = 0.0;
                double dz = 0.0;
        
                if (x < _x[0])
                        dx = _x[0] - x;
                if (x > _x[1])
                        dx = x - _x[1];
                if (y < _y[0])
                        dy = _y[0] - y;
                if (y > _y[1])
                        dy = y - _y[1];
                if (z < _z[0])
                        dz = _z[0] - z;
                if (z > _z[1])
                        dz = z - _z[1];
        
                return std::sqrt(dx * dx + dy * dy + dz * dz);
        }
}
