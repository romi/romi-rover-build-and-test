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

#ifndef __ROMI_QUINCUNX_H
#define __ROMI_QUINCUNX_H

#include "weeder/IPathPlanner.h"
#include "point.h"

namespace romi {
        
        class Quincunx : public IPathPlanner
        {
        protected:
                double _distance_plants;
                double _distance_rows;
                double _radius_zones;
                double _threshold;

                void assert_settings();
                
        public:
                Quincunx(JsonCpp& params);
                virtual ~Quincunx() override = default;
                
                bool trace_path(IFolder &session,
                                Image &mask,
                                double tool_diameter,
                                double meters_to_pixels,
                                Path &path) override;
        };
}

#endif // __ROMI_QUINCUNX_H
