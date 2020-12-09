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

#ifndef __ROMI_PATH_H
#define __ROMI_PATH_H

#include <vector>

namespace romi {
        
        struct Waypoint {
                double x;
                double y;
                double z;

                Waypoint() : x(0), y(0), z(0) {}
                Waypoint(double x_, double y_) : x(x_), y(y_), z(0.0) {}
                Waypoint(float x_, float y_) : x(x_), y(y_), z(0.0f) {}
                
                Waypoint(double x_, double y_, double z_) : x(x_), y(y_), z(z_) {}
                Waypoint(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
        };

        using Path = std::vector<Waypoint>;

        inline void path_set_z(Path &path, double z) {
                for (size_t i = 0; i < path.size(); i++)
                        path[i].z = z;
        }

        inline int path_closest_point(Path &path, double x, double y, double z) {
                int r = -1;
                if (path.size() > 0) {
                        r = 0;
                        double dmin = (path[0].x * path[0].x
                                       + path[0].y * path[0].y
                                       + path[0].z * path[0].z);
                        for (size_t i = 1; i < path.size(); i++) {
                                double d = (path[i].x * path[i].x
                                            + path[i].y * path[i].y
                                            + path[i].z * path[i].z);
                                if (d < dmin) {
                                        r = i;
                                        dmin = d;
                                }
                        }
                }
                return r;
        }

        inline void path_invert_y(Path &path) {
                for (size_t i = 0; i < path.size(); i++) {
                        path[i].y = -path[i].y;
                }
        }

        inline void path_scale(Path &path, double ax, double ay, double az) {
                for (size_t i = 0; i < path.size(); i++) {
                        path[i].x *= ax;
                        path[i].y *= ay;
                        path[i].z *= az;
                }
        }

        inline void path_translate(Path &path, double tx, double ty, double tz) {
                for (size_t i = 0; i < path.size(); i++) {
                        path[i].x += tx;
                        path[i].y += ty;
                        path[i].z += tz;
                }
        }
        
        inline void path_shift(Path &in, Path &out, size_t shift) {
                if (shift < in.size()) {
                        for (size_t i = 0; i < in.size(); i++) {
                                size_t index = (shift + i) % in.size();
                                Waypoint p = in[index];
                                out.push_back(p);
                        }
                }
        }
}

#endif // __ROMI_PATH_H
