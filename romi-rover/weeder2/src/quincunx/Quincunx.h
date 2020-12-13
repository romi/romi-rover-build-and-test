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

#include "../IPathPlanner.h"
#include "point.h"

namespace romi {
        
        class Quincunx : public IPathPlanner
        {
        protected:
                double _distance_plants;
                double _distance_rows;
                double _radius_zones;
                double _threshold;

                int set_distance_plants(json_object_t value);
                int set_distance_rows(json_object_t value);
                int set_radius_zones(json_object_t value);
                int set_threshold(json_object_t value);

                list_t *compute_positions(IFolder &session,
                                          image_t *mask,
                                          double meters_to_pixels,
                                          float *confidence);
                
                float estimate_pattern_position(image_t *p_map, float d_plants,
                                                float d_rows, point_t *pos);

                list_t *adjust_positions(image_t *p_map,
                                         float distance_plants_px,
                                         float distance_rows_px,
                                         point_t *ptn_pos,
                                         float delta);
                
                image_t *compute_convolution(image_t *image, int w, float *avg);

                list_t *boustrophedon(float x0, float x1, 
                                      float y0, float y1,
                                      float dx, float radius,
                                      list_t *positions);
                
        public:
                Quincunx() {}
                virtual ~Quincunx() override = default;
                
                int set_parameter(const char *name, json_object_t value) override;
                
                bool trace_path(IFolder &session,
                                Image &mask,
                                double tool_diameter,
                                double meters_to_pixels,
                                Path &path) override;
        };
}

#endif // __ROMI_QUINCUNX_H
