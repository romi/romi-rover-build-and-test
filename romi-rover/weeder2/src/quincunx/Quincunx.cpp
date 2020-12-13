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

#include "../IFolder.h"
#include "Quincunx.h"

namespace romi {
        
        static void store_svg(IFolder &session,
                              int w, int h,
                              const char *image,
                              list_t *path,
                              list_t *positions,
                              double radius_zones,
                              double scale);

        int Quincunx::set_distance_plants(json_object_t value)
        {
                int r = -1;
                if (json_isnumber(value)) {
                        double v = json_number_value(value);
                        if (v >= 0.001 && v <= 10.0) {
                                _distance_plants = v;
                                r = 0;
                        } else {
                                r_warn("Quincunx::set_distance_plants: "
                                       "invalid value: %f", v);
                        }
                } else {
                        r_warn("Quincunx::set_distance_plants: expected a numner");
                }
                return r;
        }
        
        int Quincunx::set_distance_rows(json_object_t value)
        {
                int r = -1;
                if (json_isnumber(value)) {
                        double v = json_number_value(value);
                        if (v >= 0.001 && v <= 10.0) {
                                _distance_rows = v;
                                r = 0;
                        } else {
                                r_warn("Quincunx::set_distance_rows: "
                                       "invalid value: %f", v);
                        }
                } else {
                        r_warn("Quincunx::set_distance_rows: expected a numner");
                }
                return r;
        }
        
        int Quincunx::set_radius_zones(json_object_t value)
        {
                int r = -1;
                if (json_isnumber(value)) {
                        double v = json_number_value(value);
                        if (v >= 0.001 && v <= 1.0) {
                                _radius_zones = v;
                                r = 0;
                        } else {
                                r_warn("Quincunx::set_radius_zones: "
                                       "invalid value: %f", v);
                        }
                } else {
                        r_warn("Quincunx::set_radius_zones: expected a numner");
                }
                return r;
        }
        
        int Quincunx::set_threshold(json_object_t value)
        {
                int r = -1;
                if (json_isnumber(value)) {
                        double v = json_number_value(value);
                        if (v >= 0.0 && v <= 1.0) {
                                _threshold = v;
                                r = 0;
                        } else {
                                r_warn("Quincunx::set_threshold: "
                                       "invalid value: %f", v);
                        }
                } else {
                        r_warn("Quincunx::set_threshold: expected a numner");
                }
                return r;
        }
        
        int Quincunx::set_parameter(const char *name, json_object_t value)
        {
                int r = -1;
                if (rstreq(name, "distance_plants"))
                        r = set_distance_plants(value);
                else if (rstreq(name, "distance_rows"))
                        r = set_distance_rows(value);
                else if (rstreq(name, "radius_zones"))
                        r = set_radius_zones(value);
                else if (rstreq(name, "threshold"))
                        r = set_threshold(value);
                else
                        r_warn("Quincunx::set_parameter: unknown parameter: %s", name);
                return r;
        }
        
        static float image_convolution(image_t* image, image_t* mask, int x0, int y0)
        {
                float r = 0.0;
                int xi_min = x0;
                int xi_max = x0 + mask->width;
                int yi_min = y0;
                int yi_max = y0 + mask->height;
                int xm_min = 0;
                int ym_min = 0;

                if (image->type != IMAGE_BW || mask->type != IMAGE_BW) {
                        fprintf(stderr, "image_convolution: Can only handle BW images\n");
                        return 0.0f;
                }
        
                if (xi_max < 0
                    || yi_max < 0
                    || xi_min >= image->width
                    || yi_min >= image->height)
                        return 0.0f;

                if (x0 < 0) {
                        xi_min = 0;
                        xm_min = -x0;
                }
                if (y0 < 0) {
                        yi_min = 0;
                        ym_min = -y0;
                }
                if (xi_max > image->width)
                        xi_max = image->width;
                if (yi_max > image->height)
                        yi_max = image->height;

                float area = (xi_max - xi_min) * (yi_max - yi_min);

                for (int yi = yi_min, ym = ym_min; yi < yi_max; yi++, ym++) {
                        int yi_off = yi * image->width;
                        int ym_off = ym * mask->width;
                        for (int xi = xi_min, xm = xm_min; xi < xi_max; xi++, xm++) {
                                float c1 = image->data[yi_off + xi];
                                float c2 = mask->data[ym_off + xm];
                                r += c1 * c2;
                        }
                }
                return r / area;
        }

        image_t *Quincunx::compute_convolution(image_t *image, int w, float *avg)
        {
                image_t *bell = new_image_bw(w, w);
                image_fill(bell, 0, 0.0f);
                image_bell(bell, w/2, w/2, w/6);

                image_t *corr = NULL;
                //
                if (1) {
                        corr = new_image_bw(image->width, image->height);
                        float corr_avg = 0.0f;
                        float corr_max = 0.0f;
                        for (int x = 0; x < corr->width; x++) {
                                for (int y = 0; y < corr->height; y++) {
                                        float v = image_convolution(image, bell,
                                                                    x - bell->width/2,
                                                                    y - bell->height/2);
                                        corr_avg += v;
                                        if (v > corr_max)
                                                corr_max = v;
                                        image_set(corr, x, y, 0, v);
                                }
                        }
                        corr_avg /= (corr->width * corr->height);
                        *avg = corr_avg;

                } else {
                        corr = image_clone(image);
                }
        
                delete_image(bell);
                return corr;
        }

        float Quincunx::estimate_pattern_position(image_t *p_map,
                                                  float d_plants,
                                                  float d_rows,
                                                  point_t *pos)
        {
                int x_max = 0;
                int y_max = 0;
                float p_max = -1.0f;

                int dr = (int) d_rows;
                int dp = (int) d_plants;
                int w = dr + 1;
                int h = dp + 1;

                for (int y = 0; y < h; y++) {
                        for (int x = 0; x < w; x++) {
                                float v = (image_get(p_map, x, y, 0)
                                           + image_get(p_map, x, y + dp, 0)
                                           + image_get(p_map, x, y + 2 * dp, 0)
                                   
                                           + image_get(p_map, x + dr, y - dp / 2, 0)
                                           + image_get(p_map, x + dr, y - dp / 2 + dp, 0)
                                           + image_get(p_map, x + dr, y - dp / 2 + 2 * dp, 0)
                                           + image_get(p_map, x + dr, y - dp / 2 + 3 * dp, 0)
                                   
                                           + image_get(p_map, x + 2 * dr, y, 0)
                                           + image_get(p_map, x + 2 * dr, y + dp, 0)
                                           + image_get(p_map, x + 2 * dr, y + 2 * dp, 0));
                        
                                if (v > p_max) {
                                        p_max = v;
                                        x_max = x;
                                        y_max = y;
                                }
                        }
                }

                pos->x = (float) x_max;
                pos->y = (float) y_max;
                return p_max;
        }

        list_t *Quincunx::adjust_positions(image_t *p_map,
                                           float distance_plants_px,
                                           float distance_rows_px,
                                           point_t *ptn_pos,
                                           float delta)
        {
                list_t *positions = 0;

                const int num_pos = 10;
                point_t pos[num_pos];
                float p[num_pos];

                int i = 0;
                for (int row = 0; row < 3; row++) {
                        int num_plants = ((row % 2) == 0)? 3 : 4;
                        float dy = ((row % 2) == 0)? 0.0f : -distance_plants_px/2.0f;
                        for (int plant = 0; plant < num_plants; plant++) {
                                float x = ptn_pos->x + row * distance_rows_px;
                                float y = ptn_pos->y + dy + plant * distance_plants_px;
                                point_set(&pos[i++], x, y, 0);
                        }
                }

                for (int n = 0; n < num_pos; n++) {
                        float sum_px = 0.0f;
                        float sum_py = 0.0f;
                        float sum = 0.0f;
                        int count = 0;

                        // TODO: factorize
                        for (int y = (int) (pos[n].y - delta); y < (int) (pos[n].y + delta); y++) {
                                for (int x = (int) (pos[n].x - delta); x < (int) (pos[n].x + delta); x++) {
                                        if (image_contains(p_map, x, y)) {
                                                count++;
                                                float v = image_get(p_map, x, y, 0);
                                                sum += v;
                                                sum_px += v * x;
                                                sum_py += v * y;
                                        }
                                }
                        }

                        p[n] = (count > 0)? sum / count : 0.0f;
                        if (sum > 0.0f) {
                                pos[n].x = sum_px / sum;
                                pos[n].y = sum_py / sum;
                        } 
                }

                for (int n = 0; n < num_pos; n++) {
                        if (p[n] > 0.003f) {
                                point_t *pt = new_point(pos[n].x, pos[n].y, 0);
                                positions = list_append(positions, pt);
                        }
                }
                
                return positions;
        }
        
        list_t *Quincunx::compute_positions(IFolder &session,
                                            image_t *mask,
                                            double meters_to_pixels,
                                            float *confidence)
        {
                list_t *positions = 0;
                float dpx_plants = (float) (_distance_plants * meters_to_pixels);
                float dpx_rows = (float) (_distance_rows * meters_to_pixels);
                point_t ptn_pos;
                float p_max = -1.0f;
                float average_prob = 0.0f;
                
                // 
                int w = (int) (0.1 * meters_to_pixels);
                
                image_t *p_map = compute_convolution(mask, w, &average_prob);

                // find best match for quincunx pattern                
                p_max = estimate_pattern_position(p_map, dpx_plants, dpx_rows, &ptn_pos);
                
                // 
                *confidence = (p_max / 10.0f) / average_prob;
        
                r_info("quincunx: p_max %f", (double) p_max);
                r_info("quincunx: average_prob %f", (double) average_prob);
                r_info("quincunx: confidence: %f", (double) *confidence);

                // adjust the positions of the points.
                float delta = (float) (0.04 * meters_to_pixels);
                positions = adjust_positions(p_map, dpx_plants, dpx_rows, &ptn_pos, delta);
                
                delete_image(p_map);
        
                return positions;
        }
                
        bool Quincunx::trace_path(IFolder &session,
                                  Image &in,
                                  double tool_diameter,
                                  double meters_to_pixels,
                                  Path &waypoints)
        {
                int success = false;
                list_t *path = 0;
                list_t *positions = 0;
                float confidence = 0.0;
                float radius_zones_px;
                float diameter_tool_px;
                float border_px;

                image_t *mask = in.ptr();

                radius_zones_px = (float) (meters_to_pixels * _radius_zones);
                diameter_tool_px = (float) (meters_to_pixels * tool_diameter);
                border_px = diameter_tool_px / 2.0f;

                positions = compute_positions(session, mask, meters_to_pixels, &confidence);

                if (positions != 0) {
                
                        path = boustrophedon(border_px, mask->width - border_px,
                                             border_px, mask->height - border_px, 
                                             diameter_tool_px, radius_zones_px,
                                             positions);
                        if (path != 0) {

                                store_svg(session, mask->width, mask->height, 
                                          "scaled.jpg", path, positions,
                                          radius_zones_px, 1.0f);
                                
                                for (list_t *l = path; l != NULL; l = list_next(l)) {
                                        point_t *p = list_get(l, point_t);
                                        Waypoint waypoint(p->x, p->y);
                                        waypoints.push_back(waypoint);
                                        delete_point(p);
                                }
                                delete_list(path);
                                
                                success = true;
                        } else {
                                r_warn("Quincunx: Failed to compute the path");
                        }

                        for (list_t *l = positions; l != NULL; l = list_next(l)) {
                                point_t *p = list_get(l, point_t);
                                delete_point(p);
                        }
                        delete_list(positions);

                } else {
                        r_warn("Quincunx: Failed to compute the positions");
                }
                
                return success;
        }

        enum {
                INTERSECTS_ERROR = -1,
                INTERSECTS_NOT = 0,
                INTERSECTS_BORDER = 1,
                INTERSECTS_IN_TWO_POINTS = 2,
                INTERSECTS_FIRST_POINT_INSIDE = 3,
                INTERSECTS_SECOND_POINT_INSIDE = 4,
                INTERSECTS_BOTH_POINTS_INSIDE = 5
        };

        /*
          Returns:
          
          -1: error (shouldn't happen :)
          0: no intersection, the segment lies outside the circle
          1: segment touches the circle on the border
          2: segment intersects the circle at two positions, ys0 and ys1, with ys0 < ys1
          3: y0 is inside the circle. the segment crosses the circle at ys1
          4: y1 is inside the circle. the segment crosses the circle at ys0
          5: no intersection, the segment is contained inside the circle
        */
        
        static int intersects_y(float x, float y0, float y1,
                                point_t *p, float r,
                                float *ys0p, float *ys1p)
        {
                // A small delta to ignore rounding errors. This corresponds
                // to 3 mm - IN OUR CASE. Adapt for any other situation.
                float delta = 0.003; 
                float dx = p->x - x;
                float d2 = dx * dx;
                float r2 = r * r;
                if (d2 + delta * delta > r2)
                        return INTERSECTS_NOT;
                if (fabs(d2 - r2) < delta * delta) {
                        float ys = p->y;
                        if (p->y <= y0 || p->y >= y1)
                                return INTERSECTS_NOT;
                        else {
                                *ys0p = ys;
                                return INTERSECTS_BORDER;
                        }
                }
                float dy = sqrtf(r2 - d2);
                float ys0 = p->y - dy;
                float ys1 = p->y + dy;
        
                *ys0p = ys0;
                *ys1p = ys1;

                // Remove small rounding errors
                if (fabs(ys0 - y0) < delta)
                        y0 = ys0;
                if (fabs(ys1 - y0) < delta)
                        y0 = ys1;
                if (fabs(ys0 - y1) < delta)
                        y1 = ys0;
                if (fabs(ys1 - y1) < delta)
                        y1 = ys1;
        
                if (ys1 < y0)
                        return INTERSECTS_NOT;
                if (ys1 == y0)
                        return INTERSECTS_BORDER;
                if (ys0 > y1)
                        return INTERSECTS_NOT;
                if (ys0 == y1)
                        return INTERSECTS_BORDER;
                if ((y0 <= ys0 && y1 > ys0) && (y0 < ys1 && y1 >= ys1))
                        return INTERSECTS_IN_TWO_POINTS;
                if ((ys0 < y0 && y0 < ys1) && (ys1 <= y1))
                        return INTERSECTS_FIRST_POINT_INSIDE;
                if ((y0 <= ys0) && (ys0 < y1 && ys1 > y1))
                        return INTERSECTS_SECOND_POINT_INSIDE;
                if (y0 > ys0 && y1 < ys1)
                        return INTERSECTS_BOTH_POINTS_INSIDE;
        
                return INTERSECTS_ERROR; // Shouldn't happen
        }

        /*
          Returns:
          
          -1: error (shouldn't happen :)
          0: no intersection, the segment lies outside the circle
          1: segment touches the circle on the border
          2: segment intersects the circle at two positions, xs0 and xs1, with xs0 < xs1
          3: x0 is inside the circle. the segment crosses the circle at xs1
          4: x1 is inside the circle. the segment crosses the circle at xs0
          5: no intersection, the segment is contained inside the circle
        */
        
        static int intersects_x(float y, float x0, float x1,
                                point_t *p, float r,
                                float *xs0p, float *xs1p)
        {
                // A small delta to ignore rounding errors. This corresponds
                // to 3 mm - IN OUR CASE. Adapt for any other situation.
                float delta = 0.003; 
                float dy = p->y - y;
                float d2 = dy * dy;
                float r2 = r * r;
                if (d2 + delta * delta > r2)
                        return INTERSECTS_NOT;
                if (fabs(d2 - r2) < delta * delta) {
                        float xs = p->x;
                        if (p->x <= x0 || p->x >= x1)
                                return INTERSECTS_NOT;
                        else {
                                *xs0p = xs;
                                return INTERSECTS_BORDER;
                        }
                }
                float dx = sqrtf(r2 - d2);
                float xs0 = p->x - dx;
                float xs1 = p->x + dx;
        
                *xs0p = xs0;
                *xs1p = xs1;

                // Remove small rounding errors
                if (fabs(xs0 - x0) < delta)
                        x0 = xs0;
                if (fabs(xs1 - x0) < delta)
                        x0 = xs1;
                if (fabs(xs0 - x1) < delta)
                        x1 = xs0;
                if (fabs(xs1 - x1) < delta)
                        x1 = xs1;
        
                if (xs1 < x0)
                        return INTERSECTS_NOT;
                if (xs1 == x0)
                        return INTERSECTS_BORDER;
                if (xs0 > x1)
                        return INTERSECTS_NOT;
                if (xs0 == x1)
                        return INTERSECTS_BORDER;
                if ((x0 <= xs0 && xs0 < x1) && (x0 < xs1 && xs1 <= x1))
                        return INTERSECTS_IN_TWO_POINTS;
                if ((xs0 < x0) && (x0 < xs1 && xs1 <= x1))
                        return INTERSECTS_FIRST_POINT_INSIDE;
                if ((x0 <= xs0 && xs0 < x1) && xs1 > x1)
                        return INTERSECTS_SECOND_POINT_INSIDE;
                if (x0 > xs0 && x1 < xs1)
                        return INTERSECTS_BOTH_POINTS_INSIDE;
        
                return INTERSECTS_ERROR; // Shouldn't happen
        }

        static int largest_y_first(point_t *pa, point_t *pb)
        {
                return (pa->y > pb->y)? -1 : (pa->y == pb->y)? 0 : 1;
        }

        static int smallest_y_first(point_t *pa, point_t *pb)
        {
                return (pa->y < pb->y)? -1 : (pa->y == pb->y)? 0 : 1;
        }

        static int smallest_x_first(point_t *pa, point_t *pb)
        {
                return (pa->x < pb->x)? -1 : (pa->x == pb->x)? 0 : 1;
        }

        static list_t *path_append(list_t *path, float x, float y, float z)
        {
                r_debug("Quincunx: (%.3f, %.3f)", x, y);        
                return list_append(path, new_point(x, y, z));
        }

        // We're in pixel coordinates: X from left to right, Y from top to
        // bottom.
        list_t *Quincunx::boustrophedon(float x0, float x1, 
                                        float y0, float y1,
                                        float dx, 
                                        float radius,
                                        list_t *positions)
        {
                list_t *path = NULL;
                float x, y, z;
                int count = 0;
        
                z = 0.0f;
                x = x0;
                y = y1;

                /* r_debug("x0=%.3f, y1=%.3f", x0, y1); */
                /* r_debug("x0=%.3f, y0=%.3f", x0, y0); */
                /* path = list_append(path, new_point(x0, y1, z));  // DEBUG */
                /* path = list_append(path, new_point(x0, y0, z));  // DEBUG */
        
                /* r_debug("y0=%.3f, y1=%.3f", y0, y1); */
                /* return NULL; */
        
                // FIXME:
                // - check that the circles don't overlap
                // - check that the tool doesn't start in an isolated corner
        
                // Make a copy because we will sort the values and change the
                // list order.
                list_t *pos = NULL;
                for (list_t *l = positions; l != NULL; l = list_next(l)) {
                        point_t *p = list_get(l, point_t);
                        pos = list_append(pos, p);
                }
        
                // Check whether the starting corner is free. If not, move the
                // starting point.
                pos = list_sort(pos, (compare_func_t) largest_y_first);
                for (list_t *l = pos; l != NULL; l = list_next(l)) {
                        float ys0, ys1;
                        point_t *p = list_get(l, point_t);
                        int deviation = intersects_y(x, y0, y1, p, radius, &ys0, &ys1);
                        switch (deviation) {
                        case INTERSECTS_ERROR:
                                r_warn("intersects returned INTERSECTS_ERROR");
                                break;
                        case INTERSECTS_NOT:
                        case INTERSECTS_BORDER:
                                break;
                        case INTERSECTS_IN_TWO_POINTS:
                                /* if (y1 - p->y < radius + 1.0f) */
                                /*         y = ys0; */
                                break;
                        case INTERSECTS_FIRST_POINT_INSIDE:
                                /* y = ys1; */
                                break;
                        case INTERSECTS_SECOND_POINT_INSIDE:
                                y = ys0;
                                break;
                        case INTERSECTS_BOTH_POINTS_INSIDE:
                                r_warn("TODO: starting point: unhandled case: both points inside!");
                                break;
                        }
                }

                /* r_debug("x=%.3f, y=%.3f", x, y); */
                path = path_append(path, x, y, z);

                /* return path; // DEBUG */
        
                while (1) {

                        //// at y1, going to y0
                        float yt = y0, xt;
                        pos = list_sort(pos, (compare_func_t) largest_y_first);
                        for (list_t *l = pos; l != NULL; l = list_next(l)) {
                                float ys0, ys1, dy, alpha0, alpha1, d_alpha;
                                int segments;
                                point_t *p = list_get(l, point_t);
                                int deviation = intersects_y(x, y0, y, p, radius, &ys0, &ys1);
                                switch (deviation) {
                                case INTERSECTS_ERROR:
                                        r_warn("intersects returned -1");
                                        break;
                                case INTERSECTS_NOT:
                                case INTERSECTS_BORDER:
                                        break;
                                case INTERSECTS_IN_TWO_POINTS: 
                                        dy = ys1 - p->y;
                                        alpha0 = asinf(dy / radius);
                                        alpha1 = -alpha0;
                                        if (p->x > x) {
                                                alpha0 = M_PI - alpha0;
                                                alpha1 = M_PI - alpha1;
                                                if (p->x - radius < 0) {
                                                        // go around the other way
                                                        alpha1 -= 2 * M_PI; 
                                                }
                                        } else {
                                                if (p->x + radius > x1) {
                                                        // go around the other way
                                                        alpha1 += 2 * M_PI; 
                                                }
                                        }
                                        if (alpha1 > alpha0)
                                                segments = (int) (1.0f + 6.0f * (alpha1 - alpha0) / M_PI);
                                        else segments = (int) (1.0f + 6.0f * (alpha0 - alpha1) / M_PI);
                                        d_alpha = (alpha1 - alpha0) / segments;
                                        for (int i = 0; i <= segments; i++) {
                                                float alpha = alpha0 + i * d_alpha;
                                                float y_ = p->y + radius * sinf(alpha);
                                                float x_ = p->x + radius * cosf(alpha);
                                                path = path_append(path, x_, y_, z);
                                                y = y_;
                                        }
                                        break;
                                case INTERSECTS_FIRST_POINT_INSIDE:
                                        if (x < p->x) {
                                                y = ys1;
                                                yt = y;
                                                path = path_append(path, x, y, z);
                                        } else {
                                                // move to the edge
                                                y = ys1;
                                                path = path_append(path, x, y, z);
                                                // go around
                                                dy = ys1 - p->y;
                                                alpha0 = asinf(dy / radius);
                                                alpha1 = asinf((y0 - p->y) / radius);
                                                if (x + dx > x1) {
                                                        float alpha2 = acosf((x1 - p->x) / radius);
                                                        if (alpha2 > alpha1)
                                                                alpha1 = alpha2;
                                                } else if (x + dx < p->x + radius) {
                                                        float alpha2 = acosf((x + dx - p->x) / radius);
                                                        if (alpha2 > alpha1)
                                                                alpha1 = alpha2;
                                                }
                                                segments = (int) (1.0f + 6.0f * (alpha0 - alpha1) / M_PI);
                                                d_alpha = (alpha0 - alpha1) / segments;
                                                for (int i = 0; i <= segments; i++) {
                                                        float alpha = alpha0 - i * d_alpha;
                                                        y = p->y + radius * sinf(alpha);
                                                        x = p->x + radius * cosf(alpha);
                                                        path = path_append(path, x, y, z);
                                                }
                                                yt = y;
                                        
                                        }
                                        break;
                                case INTERSECTS_SECOND_POINT_INSIDE:
                                        r_warn("intersects returned INTERSECTS_SECOND_POINT_INSIDE: should not happen here (at y1, moving to y0)!");
                                        break;
                                case INTERSECTS_BOTH_POINTS_INSIDE:
                                        r_warn("TODO: unhandled case: both points inside (at y1, moving to y0)!");
                                        break;
                                }
                        }
                        if (y > yt) {
                                y = yt;
                                path = path_append(path, x, y, z);
                        }

                
                        //// at y0, moving right
                        xt = x0 + 2 * count * dx + dx;
                        if (xt > x1)
                                break;

                        
                        pos = list_sort(pos, (compare_func_t) smallest_x_first);
                        for (list_t *l = pos; l != NULL; l = list_next(l)) {
                                float xs0, xs1, dxp, dxt, alpha0, alpha1, d_alpha;
                                int segments;
                                point_t *p = list_get(l, point_t);
                                int deviation = intersects_x(y, x, xt, p, radius, &xs0, &xs1);
                                switch (deviation) {
                                case INTERSECTS_ERROR:
                                        r_warn("intersects returned -1");
                                        break;
                                case INTERSECTS_NOT:
                                case INTERSECTS_BORDER:
                                        break;
                                case INTERSECTS_IN_TWO_POINTS:
                                        // Should not happen
                                        r_warn("intersects returned INTERSECTS_IN_TWO_POINTS: should not happen here (at y0, moving right)!");
                                        break;
                                case INTERSECTS_FIRST_POINT_INSIDE:
                                        // Should not happen
                                        r_warn("intersects returned INTERSECTS_FIRST_POINT_INSIDE: should not happen here (at y0, moving right)!");
                                        break;
                                case INTERSECTS_SECOND_POINT_INSIDE:
                                        // move to the edge
                                        x = xs0;
                                        path = path_append(path, x, y, z);
                                        // go around
                                        dxp = x - p->x;
                                        alpha0 = acosf(dxp / radius);
                                        if (y < p->y) alpha0 = 2.0 * M_PI - alpha0; 
                                        dxt = xt - p->x;
                                        alpha1 = acosf(dxt / radius);
                                        segments = 1 + (int)(6.0 * (alpha0 - alpha1) / M_PI);
                                        d_alpha = (alpha0 - alpha1) / segments;
                                        for (int i = 1; i <= segments; i++) {
                                                float alpha = alpha0 - i * d_alpha;
                                                y = p->y + radius * sinf(alpha);
                                                x = p->x + radius * cosf(alpha);;
                                                path = path_append(path, x, y, z);
                                        }
                                        break;
                                case INTERSECTS_BOTH_POINTS_INSIDE:
                                        r_warn("TODO: unhandled case: both points inside (at y0, moving right)!");
                                        break;
                                }
                        }
                        if (x < xt) {
                                x = xt;
                                path = path_append(path, x, y, z);
                        }


                        //// at y0, moving to y1
                        yt = y1;
                        pos = list_sort(pos, (compare_func_t) smallest_y_first);
                        for (list_t *l = pos; l != NULL; l = list_next(l)) {
                                float ys0, ys1, dy, alpha0, alpha1, d_alpha;
                                int segments;
                                point_t *p = list_get(l, point_t);
                                int deviation = intersects_y(x, y, y1, p, radius, &ys0, &ys1);
                                switch (deviation) {
                                case INTERSECTS_ERROR:
                                        r_warn("intersects returned -1");
                                        break;
                                case INTERSECTS_NOT:
                                case INTERSECTS_BORDER:
                                        break;
                                case INTERSECTS_IN_TWO_POINTS:
                                        dy = ys1 - p->y;
                                        alpha1 = asinf(dy / radius);
                                        alpha0 = -alpha1;
                                        if (p->x < x) {
                                                if (p->x + radius > x1)
                                                        alpha1 -= 2 * M_PI; 
                                        } else {
                                                alpha0 = M_PI - alpha0;
                                                alpha1 = M_PI - alpha1;
                                                if (p->x - radius < 0)
                                                        alpha1 += 2 * M_PI; 
                                        }
                                        if (alpha1 > alpha0)
                                                segments = (int) (1.0f + 6.0f * (alpha1 - alpha0) / M_PI);
                                        else segments = (int) (1.0f + 6.0f * (alpha0 - alpha1) / M_PI);
                                        d_alpha = (alpha1 - alpha0) / segments;
                                        for (int i = 0; i <= segments; i++) {
                                                float alpha = alpha0 + i * d_alpha;
                                                float y_ = p->y + radius * sinf(alpha);
                                                float x_ = p->x + radius * cosf(alpha);
                                                path = path_append(path, x_, y_, z);
                                                y = y_;
                                        }
                                
                                        break;
                                case INTERSECTS_FIRST_POINT_INSIDE:
                                        r_warn("intersects returned INTERSECTS_FIRST_POINT_INSIDE: should not happen here (at y0, moving to y1)!");
                                        break;
                                case INTERSECTS_SECOND_POINT_INSIDE:
                                        r_warn("INTERSECTS_SECOND_POINT_INSIDE");
                                        if (x < p->x) {
                                                y = ys0;
                                                yt = y;
                                                path = path_append(path, x, y, z);
                                        } else {
                                                r_warn("x >= p->x");
                                                dy = ys0 - p->y;
                                                alpha0 = asinf(dy / radius);
                                                alpha1 = asinf((y1 - p->y) / radius);
                                                if (x + dx > x1) {
                                                        float alpha2 = -acosf((x1 - p->x) / radius);
                                                        if (alpha2 < alpha1)
                                                                alpha1 = alpha2;
                                                } else if (x + dx < p->x + radius) {
                                                        float alpha2 = -acosf((x + dx - p->x) / radius);
                                                        if (alpha2 < alpha1)
                                                                alpha1 = alpha2;
                                                }
                                                segments = (int) (1.0f + 6.0f * (alpha1 - alpha0) / M_PI);
                                                r_warn("segments=%d", segments);

                                                d_alpha = (alpha1 - alpha0) / segments;
                                                for (int i = 0; i <= segments; i++) {
                                                        float alpha = alpha0 + i * d_alpha;
                                                        y = p->y + radius * sinf(alpha);
                                                        x = p->x + radius * cosf(alpha);
                                                        path = path_append(path, x, y, z);
                                                }
                                                yt = y;
                                        
                                        }
                                        break;
                                case INTERSECTS_BOTH_POINTS_INSIDE:
                                        r_warn("TODO: unhandled case: both points inside (at y0, moving to y1)!");
                                        break;
                                }
                        }
                        if (y < yt) {
                                y = yt;
                                path = path_append(path, x, y, z);
                        }
                
                        //// at y1, moving right
                        xt = x0 + 2 * count * dx + 2 * dx;
                        if (xt > x1)
                                break;
                        
                        pos = list_sort(pos, (compare_func_t) smallest_x_first);
                        for (list_t *l = pos; l != NULL; l = list_next(l)) {
                                float xs0, xs1, dxp, dxt, alpha0, alpha1, d_alpha;
                                int segments;
                                point_t *p = list_get(l, point_t);
                                int deviation = intersects_x(y, x, xt, p, radius, &xs0, &xs1);
                                switch (deviation) {
                                case INTERSECTS_ERROR:
                                        r_warn("intersects returned -1");
                                        break;
                                case INTERSECTS_NOT:
                                case INTERSECTS_BORDER:
                                        break;
                                case INTERSECTS_IN_TWO_POINTS:
                                        // Should not happen
                                        r_warn("intersects returned INTERSECTS_IN_TWO_POINTS: should not happen here (at y1, moving right)!");
                                        break;
                                case INTERSECTS_FIRST_POINT_INSIDE:
                                        // Should not happen
                                        r_warn("intersects returned INTERSECTS_FIRST_POINT_INSIDE: should not happen here (at y1, moving right)!");
                                        break;
                                case INTERSECTS_SECOND_POINT_INSIDE:
                                        // move down to pass in below of the zone
                                        //y = p->y - radius;
                                        //path = path_append(path, x, y, z);
                                        
                                        // move to the edge
                                        x = xs0;
                                        path = path_append(path, x, y, z);
                                        // go around
                                        dxp = x - p->x;
                                        alpha0 = acosf(dxp / radius);
                                        if (y < p->y) alpha0 = 2.0 * M_PI - alpha0; 
                                        dxt = xt - p->x;
                                        alpha1 = acosf(dxt / radius);
                                        alpha1 = 2.0 * M_PI - alpha1; 
                                        segments = 1 + (int)(6.0 * (alpha1 - alpha0) / M_PI);
                                        d_alpha = (alpha1 - alpha0) / segments;
                                        for (int i = 1; i <= segments; i++) {
                                                float alpha = alpha0 + i * d_alpha;
                                                y = p->y + radius * sinf(alpha);
                                                x = p->x + radius * cosf(alpha);;
                                                path = path_append(path, x, y, z);
                                        }

                                        
                                        break;
                                case INTERSECTS_BOTH_POINTS_INSIDE:
                                        r_warn("TODO: unhandled case: both points inside (at y1, moving right)!");
                                        break;
                                }
                        }
                        if (x < xt) {
                                x = xt;
                                path = path_append(path, x, y, z);
                        }
                
                        count++;
                }

                delete_list(pos);

                return path;
        }

        
        ////////////////////////////////////////////////////////////////////

        static void store_path(membuf_t *buffer, list_t *points, int h, double scale)
        {
                membuf_printf(buffer, "    <path d=\"");
                float x, y;
        
                point_t *p = list_get(points, point_t);
                x = p->x * scale;
                /* y = h - p->y * scale; */
                y = p->y * scale;
                membuf_printf(buffer, "M %f,%f L", x, y);
                points = list_next(points);

                while (points) {
                        p = list_get(points, point_t);
                        x = p->x * scale;
                        y = p->y * scale;
                        //y = h - p->y * scale;
                        membuf_printf(buffer, " %f,%f", x, y);
                        points = list_next(points);
                }
                membuf_printf(buffer, "\" id=\"path\" style=\"fill:none;stroke:#0000ce;"
                              "stroke-width:2;stroke-linecap:butt;"
                              "stroke-linejoin:miter;stroke-miterlimit:4;"
                              "stroke-opacity:1;stroke-dasharray:none\" />\n");
        }

        static void store_zones(membuf_t *buffer,
                                list_t *positions,
                                double radius_zones,
                                double h, double scale)
        {
                double r = radius_zones * scale;
                for (list_t *l = positions; l != NULL; l = list_next(l)) {
                        point_t *p = list_get(l, point_t);
                        double x = p->x * scale;
                        //double y = h - p->y * scale;
                        double y = p->y * scale;
                        membuf_printf(buffer,
                                      "  <circle "
                                      "style=\"fill:#ff80ff;fill-opacity:0.25;stroke-width:0\" "
                                      "cx=\"%.3f\" cy=\"%.3f\" r=\"%.3f\" />\n",
                                      x, y, r);
                }
        }

        static void store_svg(IFolder &session,
                              int w, int h,
                              const char *image,
                              list_t *path,
                              list_t *positions,
                              double radius_zones,
                              double scale)
        {
                membuf_t *buffer = new_membuf();
        
                membuf_printf(buffer,
                              ("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>"
                               "<svg xmlns:svg=\"http://www.w3.org/2000/svg\" "
                               "xmlns=\"http://www.w3.org/2000/svg\" "
                               "xmlns:xlink=\"http://www.w3.org/1999/xlink\" "
                               "version=\"1.0\" width=\"%dpx\" height=\"%dpx\">\n"),
                              w, h);
                membuf_printf(buffer,
                              "    <image xlink:href=\"%s\" "
                              "x=\"0px\" y=\"0px\" width=\"%dpx\" height=\"%dpx\" />\n",
                              image, w, h);
                store_path(buffer, path, h, scale);
                store_zones(buffer, positions, radius_zones, h, scale);
                membuf_printf(buffer, "</svg>\n");

                session.store_svg("path", membuf_data(buffer), membuf_len(buffer));
                delete_membuf(buffer);
        }

}


