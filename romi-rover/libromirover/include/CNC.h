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
#ifndef __ROMI_CNC_H
#define __ROMI_CNC_H

#include "IConfiguration.h"
#include "Path.h"
#include "CNCRange.h"

namespace romi {
        
        class CNC
        {
        public:

                static constexpr double UNCHANGED = -999999.0;
                
                virtual ~CNC() = default;

                virtual bool get_range(CNCRange &range) = 0;

                // The positions are given in meters. The speed is
                // given as a fraction of the maximum speed. A value
                // of 1.0 means maximum speed. The actual speed will
                // depend on the direction of the (x,y,z) vector and
                // the allowed maximum speeds on each of the axes.
                virtual bool moveto(double x, double y, double z,
                                    double relative_speed = 0.1) = 0;
                
                virtual bool spindle(double speed) = 0;
                virtual bool travel(Path &path, double relative_speed = 0.1) = 0;
                virtual bool homing() = 0;

                virtual bool stop_execution() = 0;
                virtual bool continue_execution() = 0;
                virtual bool reset() = 0;
        };
}

#endif // __ROMI_CNC_H
