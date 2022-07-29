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

#ifndef __ROMI_I_PATH_PLANNER_H
#define __ROMI_I_PATH_PLANNER_H

#include <vector>

#include <api/Path.h>
#include <session/ISession.h>
#include <cv/Image.h>
#include <cv/cv.h>

namespace romi {
        
        class IPathPlanner
        {
        public:
                virtual ~IPathPlanner() = default;

                virtual Path trace_path(ISession& session, Centers& centers, Image& mask) = 0;
        };
}

#endif // __ROMI_I_PATH_PLANNER_H
