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
#include <algorithm>
#include "api/Path.h"

namespace romi {

        void Path::set_z(double z)
        {
                for (size_t i = 0; i < size(); i++) {
                        at(i).set(2, z);
                }
        }

        int Path::closest_point(v3 p)
        {
                int r = -1;
                if (size() > 0) {
                        r = 0;
                        // Initial estimate:
                        double dmin = norm(at(0) - p);
                        for (size_t i = 1; i < size(); i++) {
                                double d = norm(at(i) - p);
                                if (d < dmin) {
                                        r = i;
                                        dmin = d;
                                }
                        }
                }
                return r;
        }

        void Path::invert_y()
        {
                for (size_t i = 0; i < size(); i++) {
                        at(i).set(1, 1.0 - at(i).y());
                }
        }

        void Path::scale(v3 scale)
        {
                for (size_t i = 0; i < size(); i++) {
                        at(i) = at(i) * scale;
                }
        }

        void Path::translate(v3 t)
        {
                for (size_t i = 0; i < size(); i++) {
                        at(i) = at(i) + t;
                }
        }
        
        void Path::rotate(Path &out, int start_index)
        {
                start_index = std::clamp(start_index, 0, (int) size());
                for (size_t i = 0; i < size(); i++) {
                        size_t index = (start_index + i) % size();
                        out.push_back(at(index));
                }
        }

        bool Path::clamp(CNCRange& range, double allowed_error)
        {
                bool ok = true;
                for (size_t i = 0; i < size(); i++) {
                        if (!range.is_inside(at(i))) {
                                ok = clamp(range, i, allowed_error);
                                if (!ok)
                                        break;
                        }
                }
                return ok;
        }

        bool Path::clamp(CNCRange& range, size_t i, double allowed_error)
        {
                bool ok = true;
                double error = range.error(at(i));
                if (error <= allowed_error) {
                        at(i) = range.clamp(at(i));
                } else {
                        r_err("Point: %.3f, %.3f, %.3f: "
                              "Out of range: (%.3f, %.3f), "
                              "(%.3f, %.3f), (%.3f, %.3f)",
                              at(i).x(), at(i).y(), at(i).z(),
                              range.min.x(), range.max.x(),
                              range.min.y(), range.max.y(),
                              range.min.z(), range.max.z());

                        ok = false;
                }
                return ok;
        }
                

}
