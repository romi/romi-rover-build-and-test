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
#include "r.h"
#include "romi.h"
#include "Path.h"
#include "Image.h"

namespace romi {

        class IFileCabinet;
        
        class IPathPlanner
        {
        public:
                virtual ~IPathPlanner() = default;
                
                virtual int set_parameter(const char *name, json_object_t value) = 0;
                
                /* virtual bool trace_path(IFileCabinet &session, */
                /*                         image_t &mask, */
                /*                         double tool_diameter, // in meters */
                /*                         double meters_to_pixels, */
                /*                         Path &path) = 0; */

                virtual bool trace_path(IFileCabinet *session,
                                        Image &mask,
                                        double tool_diameter, // in meters
                                        double meters_to_pixels,
                                        Path &path) = 0;
        };
}

#endif // __ROMI_I_PATH_PLANNER_H
