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
#include "v.h"
#include "CNCRange.h"

namespace romi {

        class Path : public std::vector<v3>
        {
        protected:
                bool clamp(CNCRange& range, size_t index, double allowed_error);

        public:
                virtual ~Path() = default;
                
                void set_z(double z);
                int closest_point(v3 p);
                void invert_y();
                void scale(v3 scale);
                void translate(v3 t);
                void rotate(Path &out, int start_index);
                bool clamp(CNCRange& range, double allowed_error);
        };
}

#endif // __ROMI_PATH_H
