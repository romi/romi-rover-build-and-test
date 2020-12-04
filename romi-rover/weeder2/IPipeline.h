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

#ifndef __ROMI_I_PIPELINE_H
#define __ROMI_I_PIPELINE_H

#include "Image.h"
#include "IPathPlanner.h"
#include "IFileCabinet.h"

namespace romi {
        
        class IPipeline
        {
        public:
                virtual ~IPipeline() = default;
                
                virtual bool run(IFileCabinet *session,
                                 Image &camera,
                                 double tool_diameter,
                                 Path &path) = 0;
        };
}

#endif // __ROMI_I_PIPELINE_H
