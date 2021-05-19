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

#include <math.h>
#include <algorithm>
#include <session/ISession.h>
#include "quincunx/Quincunx.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wfloat-conversion"
#pragma GCC diagnostic ignored "-Wsign-compare"

namespace romi {
        
        // static void store_svg(ISession &session,
        //                       int w, int h,
        //                       const char *image,
        //                       const std::vector<point_t>& path,
        //                       const std::vector<point_t>& positions,
        //                       double radius_zones,
        //                       double scale);
        
        static std::vector<point_t> boustrophedon(float x0, float x1,
                                     float y0, float y1,
                                     float dx, 
                                     float radius,
                                     std::vector<point_t>& positions);

        Quincunx::Quincunx(JsonCpp& params) :
                _distance_plants(0.0),
                _distance_rows(0.0),
                _radius_zones(0.0),
                _threshold(0.0)
        {
                try {
                        _distance_plants = (double) params["distance-plants"];
                        _distance_rows = (double) params["distance-rows"];
                        _radius_zones = params.num("radius-zones", 0.1);
                        _threshold = params.num("threshold", 0.5);
                        assert_settings();
                        
                } catch (JSONError& je) {
                        r_warn("Quincunx: invalid JSON");
                        throw std::runtime_error("Quincunx: invalid JSON");
                }
        }

        void Quincunx::assert_settings()
        {
                if (_distance_plants < 0.001 || _distance_plants > 10.0
                    || _distance_rows < 0.001 || _distance_rows > 10.0
                    || _radius_zones < 0.001 || _radius_zones > 1.0
                    || _threshold <= 0.0 || _threshold > 1.0) {
                        r_warn("Quincunx: invalid settings: distance_plants %f, "
                               "distance_rows %f, radius_zones %f, threshold %f",
                               _distance_plants, _distance_rows, _radius_zones, _threshold);
                        throw std::runtime_error("Quincunx: invalid settings");
                }
        }
        
        
        static float convolution(Image& image, Image& mask, ssize_t x0, ssize_t y0)
        {
                float r = 0.0;
                ssize_t xi_min = x0;
                ssize_t xi_max = x0 + mask.width();
                ssize_t yi_min = y0;
                ssize_t yi_max = y0 + mask.height();
                size_t xm_min = 0;
                size_t ym_min = 0;
                auto& data1 = image.data();
                auto& data2 = mask.data();

                if (image.type() != Image::BW || mask.type() != Image::BW) {
                        r_err("Quincunx: convolution: Can only handle BW images");
                        return 0.0f;
                }
        
                if (xi_max < 0
                    || yi_max < 0
                    || xi_min >= (ssize_t) image.width()
                    || yi_min >= (ssize_t) image.height())
                        return 0.0f;

                if (x0 < 0) {
                        xi_min = 0;
                        xm_min = -x0;
                }
                if (y0 < 0) {
                        yi_min = 0;
                        ym_min = -y0;
                }
                
                if (xi_max > (ssize_t) image.width())
                        xi_max = image.width();
                
                if (yi_max > (ssize_t) image.height())
                        yi_max = image.height();

                float area = (xi_max - xi_min) * (yi_max - yi_min);

                for (size_t yi = yi_min, ym = ym_min; yi < (size_t) yi_max; yi++, ym++) {
                        
                        size_t yi_off = yi * image.width();
                        size_t ym_off = ym * mask.width();
                        
                        for (size_t xi = xi_min, xm = xm_min; xi < (size_t) xi_max; xi++, xm++) {
                                float c1 = data1[yi_off + xi];
                                float c2 = data2[ym_off + xm];
                                r += c1 * c2;
                        }
                }
                return r / area;
        }

        static void fill_bell(Image& image, float xc, float yc, float stddev)
        {
                float r = 3 * stddev;
                int ymin = ceilf(yc - r);
                int ymax = floorf(yc + r);
                float var = stddev * stddev;
                float r2 = r * r;
                int width = image.width();
                int height = image.height();
                
                if (ymin >= height || ymax < 0)
                        return;
                if (xc - r >= width || xc + r < 0)
                        return;
                
                if (ymin < 0)
                        ymin = 0;
                if (ymax >= height)
                        ymax = height - 1;
        
                for (int y = ymin; y <= ymax; y++) {
                        float _y = (float) y - yc;
                        float _x = sqrtf(r2 - (_y * _y));
                        int xmin = (int) roundf(xc - _x);
                        int xmax = (int) roundf(xc + _x);
                
                        if (xmin >= width || xmax < 0)
                                continue;
                        if (xmin < 0)
                                xmin = 0;
                        if (xmax >= width)
                                xmax = width - 1;
                
                        for (int x = xmin; x <= xmax; x++) {
                                // For normalised values, the color
                                // value should be divided by
                                // 2.pi.variance. I leave it as is so
                                // that the maximum color is white.
                                _x = (float) x - xc;
                                float color = expf(-(_x * _x + _y * _y) / (2.0f * var));
                                image.set(0, x, y, color);
                        }
                }
        }
        
        static void compute_convolution(Image& image, size_t w, float *avg, Image& out)
        {
                Image bell(Image::BW, w, w);
                bell.fill(0, 0.0f);
                fill_bell(bell, w/2, w/2, w/6);

                size_t width = image.width();
                size_t height = image.height();
                
                out.init(Image::BW, width, height);
                
                float corr_avg = 0.0f;
                float corr_max = 0.0f;
                
                for (size_t x = 0; x < width; x++) {
                        for (size_t y = 0; y < height; y++) {
                                float v = convolution(image, bell,
                                                      x - bell.width()/2,
                                                      y - bell.height()/2);
                                corr_avg += v;
                                if (v > corr_max)
                                        corr_max = v;
                                out.set(0, x, y, v);
                        }
                }
                corr_avg /= (width * height);
                *avg = corr_avg;
        }

        static float estimate_pattern_position(Image& map, float d_plants,
                                               float d_rows, point_t& pos)
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
                                float v = (map.get(0, x, y)
                                           + map.get(0, x, y + dp)
                                           + map.get(0, x, y + 2 * dp)
                                   
                                           + map.get(0, x + dr, y - dp / 2)
                                           + map.get(0, x + dr, y - dp / 2 + dp)
                                           + map.get(0, x + dr, y - dp / 2 + 2 * dp)
                                           + map.get(0, x + dr, y - dp / 2 + 3 * dp)
                                   
                                           + map.get(0, x + 2 * dr, y)
                                           + map.get(0, x + 2 * dr, y + dp)
                                           + map.get(0, x + 2 * dr, y + 2 * dp));
                        
                                if (v > p_max) {
                                        p_max = v;
                                        x_max = x;
                                        y_max = y;
                                }
                        }
                }

                pos.x = (float) x_max;
                pos.y = (float) y_max;
                return p_max;
        }

        static std::vector<point_t> adjust_positions(Image& map,
                                        float distance_plants_px,
                                        float distance_rows_px,
                                        point_t *ptn_pos,
                                        float delta)
        {
                std::vector<point_t> positions;

                const int num_pos = 10;
//                point_t pos[num_pos];
                std::vector<point_t> pos;
                float p[num_pos];

//                int i = 0;
                for (int row = 0; row < 3; row++) {
                        int num_plants = ((row % 2) == 0)? 3 : 4;
                        float dy = ((row % 2) == 0)? 0.0f : -distance_plants_px/2.0f;
                        for (int plant = 0; plant < num_plants; plant++) {
                                float x = ptn_pos->x + row * distance_rows_px;
                                float y = ptn_pos->y + dy + plant * distance_plants_px;
//                                point_set(&pos[i++], x, y, 0);
                                pos.emplace_back(x,y,0);
                        }
                }

                for (int n = 0; n < num_pos; n++) {
                        float sum_px = 0.0f;
                        float sum_py = 0.0f;
                        float sum = 0.0f;
                        int count = 0;

                        // TODO: factorize
                        for (int y = (int) (pos[n].y - delta);
                             y < (int) (pos[n].y + delta);
                             y++) {
                                for (int x = (int) (pos[n].x - delta);
                                     x < (int) (pos[n].x + delta);
                                     x++) {
                                        if (map.contains(x, y)) {
                                                count++;
                                                float v = map.get(0, x, y);
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
//                                point_t *pt = new_point(pos[n].x, pos[n].y, 0);
//                                positions = list_append(positions, pt);
                                positions.emplace_back(pos[n].x, pos[n].y, 0);
                        }
                }
                
                return positions;
        }
        
        static std::vector<point_t>
        compute_positions(Image &mask, double distance_plants, double distance_rows, double meters_to_pixels,
                          float *confidence)
        {
                std::vector<point_t> positions;
                float dpx_plants = (float) (distance_plants * meters_to_pixels);
                float dpx_rows = (float) (distance_rows * meters_to_pixels);
                point_t ptn_pos;
                float p_max = -1.0f;
                float average_prob = 0.0f;
                
                // 
                size_t w = (size_t) (0.1 * meters_to_pixels);
                
                Image p_map;
                compute_convolution(mask, w, &average_prob, p_map);

                // find best match for quincunx pattern                
                p_max = estimate_pattern_position(p_map, dpx_plants, dpx_rows, ptn_pos);
                
                // 
                *confidence = (p_max / 10.0f) / average_prob;
        
                r_info("quincunx: p_max %f", (double) p_max);
                r_info("quincunx: average_prob %f", (double) average_prob);
                r_info("quincunx: confidence: %f", (double) *confidence);

                // adjust the positions of the points.
                float delta = (float) (0.04 * meters_to_pixels);
                positions = adjust_positions(p_map, dpx_plants, dpx_rows, &ptn_pos, delta);
        
                return positions;
        }

        Path Quincunx::trace_path(ISession& session, Centers& centers, Image& mask)
        {
                (void) session;
                (void) centers;
                (void) mask;
                throw std::runtime_error("Quincunx::trace_path: NOT IMPLEMENTED");
        }
        
        bool Quincunx::trace_path(ISession &session,
                                  Image &mask,
                                  double tool_diameter,
                                  double meters_to_pixels,
                                  Path &waypoints)
        {
                (void) session;
                int success = false;
                std::vector<point_t> path;
                std::vector<point_t> positions;
                float confidence = 0.0;
                float radius_zones_px;
                float diameter_tool_px;
                float border_px;

                radius_zones_px = (float) (meters_to_pixels * _radius_zones);
                diameter_tool_px = (float) (meters_to_pixels * tool_diameter);
                border_px = diameter_tool_px / 2.0f;

                positions = compute_positions(mask,
                                              _distance_plants,
                                              _distance_rows,
                                              meters_to_pixels, &confidence);

                if (!positions.empty()) {
                
                        path = boustrophedon(border_px, mask.width() - border_px,
                                             border_px, mask.height() - border_px, 
                                             diameter_tool_px, radius_zones_px,
                                             positions);
                        if (!path.empty()) {

                                // store_svg(session, mask.width(), mask.height(), 
                                //           "scaled.jpg", path, positions,
                                //           radius_zones_px, 1.0f);

                                for (auto& point : path)
                                {
                                        waypoints.push_back(v3(point.x, point.y, 0));
                                }
//
//                                for (list_t *l = path; l != nullptr; l = list_next(l)) {
//                                        point_t *p = list_get(l, point_t);
//                                        waypoints.push_back(v3(p->x, p->y, 0));
//                                        delete_point(p);
//                                }
//                                delete_list(path);
                                
                                success = true;
                        } else {
                                r_warn("Quincunx: Failed to compute the path");
                        }
//
//                        for (list_t *l = positions; l != nullptr; l = list_next(l)) {
//                                point_t *p = list_get(l, point_t);
//                                delete_point(p);
//                        }
//                        delete_list(positions);

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
                                point_t& p, float r,
                                float *ys0p, float *ys1p)
        {
                // A small delta to ignore rounding errors. This corresponds
                // to 3 mm - IN OUR CASE. Adapt for any other situation.
                float delta = 0.003; 
                float dx = p.x - x;
                float d2 = dx * dx;
                float r2 = r * r;
                if (d2 + delta * delta > r2)
                        return INTERSECTS_NOT;
                if (fabs(d2 - r2) < delta * delta) {
                        float ys = p.y;
                        if (p.y <= y0 || p.y >= y1)
                                return INTERSECTS_NOT;
                        else {
                                *ys0p = ys;
                                return INTERSECTS_BORDER;
                        }
                }
                float dy = sqrtf(r2 - d2);
                float ys0 = p.y - dy;
                float ys1 = p.y + dy;
        
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
                                point_t& p, float r,
                                float *xs0p, float *xs1p)
        {
                // A small delta to ignore rounding errors. This corresponds
                // to 3 mm - IN OUR CASE. Adapt for any other situation.
                float delta = 0.003; 
                float dy = p.y - y;
                float d2 = dy * dy;
                float r2 = r * r;
                if (d2 + delta * delta > r2)
                        return INTERSECTS_NOT;
                if (fabs(d2 - r2) < delta * delta) {
                        float xs = p.x;
                        if (p.x <= x0 || p.x >= x1)
                                return INTERSECTS_NOT;
                        else {
                                *xs0p = xs;
                                return INTERSECTS_BORDER;
                        }
                }
                float dx = sqrtf(r2 - d2);
                float xs0 = p.x - dx;
                float xs1 = p.x + dx;
        
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

        static int largest_y_first(const point_t& pa, const point_t& pb)
        {
                return (pa.y > pb.y)? -1 : (pa.y == pb.y)? 0 : 1;
        }

        static int smallest_y_first(const point_t& pa, const point_t& pb)
        {
                return (pa.y < pb.y)? -1 : (pa.y == pb.y)? 0 : 1;
        }

        static int smallest_x_first(const point_t& pa, const point_t& pb)
        {
                return (pa.x < pb.x)? -1 : (pa.x == pb.x)? 0 : 1;
        }

//        static void path_append(std::vector<point_t>& path, float x, float y, float z)
//        {
////                r_debug("Quincunx: (%.3f, %.3f)", x, y);
////                return list_append(path, new_point(x, y, z));
//                path.emplace_back(x,y,z);
//        }

        // We're in pixel coordinates: X from left to right, Y from top to
        // bottom.
        static std::vector<point_t> boustrophedon(float x0, float x1,
                                     float y0, float y1,
                                     float dx, 
                                     float radius,
                                      std::vector<point_t>& positions)
        {
                std::vector<point_t> path;
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
                std::vector<point_t> pos = positions;

//                for (list_t *l = positions; l != NULL; l = list_next(l)) {
//                        point_t *p = list_get(l, point_t);
//                        pos = list_append(pos, p);
//                }
        
                // Check whether the starting corner is free. If not, move the
                // starting point.
//                pos = list_sort(pos, (compare_func_t) largest_y_first);

                std::sort( pos.begin( ), pos.end( ), [ ]( const point_t & lhs, const point_t & rhs )
                {
                    return largest_y_first(lhs, rhs);
                });


//                for (list_t *l = pos; l != nullptr; l = list_next(l)) {
                        for (auto& position : pos) {
                        float ys0, ys1;
//                        point_t *p = list_get(l, point_t);
                        int deviation = intersects_y(x, y0, y1, position, radius, &ys0, &ys1);
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
                        default:
                                r_warn("boustrophedon unknown deviation. %d",deviation);
                        }
                }

                /* r_debug("x=%.3f, y=%.3f", x, y); */
                path.emplace_back(x, y, z);
//                path = path_append(path, x, y, z);

                /* return path; // DEBUG */

                // coule be (xt <= x1) or do while if bottom check!
                while (1) {

                        //// at y1, going to y0
                        float yt = y0, xt;
//                        pos = list_sort(pos, (compare_func_t) largest_y_first);
                        std::sort( pos.begin( ), pos.end( ), [ ]( const point_t & lhs, const point_t & rhs )
                        {
                            return largest_y_first(lhs, rhs);
                        });

//                        for (list_t *l = pos; l != nullptr; l = list_next(l)) {
                        for (auto& position : pos) {
                                float ys0, ys1, dy, alpha0, alpha1, d_alpha;
                                int segments;
//                                point_t *p = list_get(l, point_t);
                                int deviation = intersects_y(x, y0, y, position, radius, &ys0, &ys1);
                                switch (deviation) {
                                case INTERSECTS_ERROR:
                                        r_warn("intersects returned -1");
                                        break;
                                case INTERSECTS_NOT:
                                case INTERSECTS_BORDER:
                                        break;
                                case INTERSECTS_IN_TWO_POINTS: 
                                        dy = ys1 - position.y;
                                        alpha0 = asinf(dy / radius);
                                        alpha1 = -alpha0;
                                        if (position.x > x) {
                                                alpha0 = M_PI - alpha0;
                                                alpha1 = M_PI - alpha1;
                                                if (position.x - radius < 0) {
                                                        // go around the other way
                                                        alpha1 -= 2 * M_PI; 
                                                }
                                        } else {
                                                if (position.x + radius > x1) {
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
                                                float y_ = position.y + radius * sinf(alpha);
                                                float x_ = position.x + radius * cosf(alpha);
//                                                path = path_append(path, x_, y_, z);
                                                path.emplace_back(x_, y_, z);
                                                y = y_;
                                        }
                                        break;
                                case INTERSECTS_FIRST_POINT_INSIDE:
                                        if (x < position.x) {
                                                y = ys1;
                                                yt = y;
//                                                path = path_append(path, x, y, z);
                                                path.emplace_back(x, y, z);
                                        } else {
                                                // move to the edge
                                                y = ys1;
//                                                path = path_append(path, x, y, z);
                                                path.emplace_back(x, y, z);
                                                // go around
                                                dy = ys1 - position.y;
                                                alpha0 = asinf(dy / radius);
                                                alpha1 = asinf((y0 - position.y) / radius);
                                                if (x + dx > x1) {
                                                        float alpha2 = acosf((x1 - position.x) / radius);
                                                        if (alpha2 > alpha1)
                                                                alpha1 = alpha2;
                                                } else if (x + dx < position.x + radius) {
                                                        float alpha2 = acosf((x + dx - position.x) / radius);
                                                        if (alpha2 > alpha1)
                                                                alpha1 = alpha2;
                                                }
                                                segments = (int) (1.0f + 6.0f * (alpha0 - alpha1) / M_PI);
                                                d_alpha = (alpha0 - alpha1) / segments;
                                                for (int i = 0; i <= segments; i++) {
                                                        float alpha = alpha0 - i * d_alpha;
                                                        y = position.y + radius * sinf(alpha);
                                                        x = position.x + radius * cosf(alpha);
//                                                        path = path_append(path, x, y, z);
                                                        path.emplace_back(x, y, z);
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
                                default:
                                        r_warn("boustrophedon at y1, going to y0 unknown deviation. %d",deviation);
                                }
                        }
                        if (y > yt) {
                                y = yt;
//                                path = path_append(path, x, y, z);
                                path.emplace_back(x, y, z);
                        }

                
                        //// at y0, moving right
                        xt = x0 + 2 * count * dx + dx;
                        if (xt > x1)
                                break;

                        
//                        pos = list_sort(pos, (compare_func_t) smallest_x_first);
                        std::sort( pos.begin( ), pos.end( ), [ ]( const point_t& lhs, const point_t& rhs )
                        {
                            return smallest_x_first(lhs, rhs);
                        });

//                        for (list_t *l = pos; l != nullptr; l = list_next(l)) {
                        for (auto& position : pos) {
                                float xs0, xs1, dxp, dxt, alpha0, alpha1, d_alpha;
                                int segments;
//                                point_t *p = list_get(l, point_t);
                                int deviation = intersects_x(y, x, xt, position, radius, &xs0, &xs1);
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
//                                        path = path_append(path, x, y, z);
                                        path.emplace_back(x, y, z);
                                        // go around
                                        dxp = x - position.x;
                                        alpha0 = acosf(dxp / radius);
                                        if (y < position.y) alpha0 = 2.0 * M_PI - alpha0;
                                        dxt = xt - position.x;
                                        alpha1 = acosf(dxt / radius);
                                        segments = 1 + (int)(6.0 * (alpha0 - alpha1) / M_PI);
                                        d_alpha = (alpha0 - alpha1) / segments;
                                        for (int i = 1; i <= segments; i++) {
                                                float alpha = alpha0 - i * d_alpha;
                                                y = position.y + radius * sinf(alpha);
                                                x = position.x + radius * cosf(alpha);;
//                                                path = path_append(path, x, y, z);
                                                path.emplace_back(x, y, z);
                                        }
                                        break;
                                case INTERSECTS_BOTH_POINTS_INSIDE:
                                        r_warn("TODO: unhandled case: both points inside (at y0, moving right)!");
                                        break;
                                default:
                                        r_warn("boustrophedon at y0, moving right unknown deviation. %d",deviation);
                                }
                        }
                        if (x < xt) {
                                x = xt;
//                                path = path_append(path, x, y, z);
                                path.emplace_back(x, y, z);
                        }


                        //// at y0, moving to y1
                        yt = y1;
//                        pos = list_sort(pos, (compare_func_t) smallest_y_first);
                        std::sort( pos.begin( ), pos.end( ), [ ]( const point_t & lhs, const point_t & rhs )
                        {
                            return smallest_y_first(lhs, rhs);
                        });

//                        for (list_t *l = pos; l != nullptr; l = list_next(l)) {
                        for (auto& position : pos) {
                                float ys0, ys1, dy, alpha0, alpha1, d_alpha;
                                int segments;
//                                point_t *p = list_get(l, point_t);
                                int deviation = intersects_y(x, y, y1, position, radius, &ys0, &ys1);
                                switch (deviation) {
                                case INTERSECTS_ERROR:
                                        r_warn("intersects returned -1");
                                        break;
                                case INTERSECTS_NOT:
                                case INTERSECTS_BORDER:
                                        break;
                                case INTERSECTS_IN_TWO_POINTS:
                                        dy = ys1 - position.y;
                                        alpha1 = asinf(dy / radius);
                                        alpha0 = -alpha1;
                                        if (position.x < x) {
                                                if (position.x + radius > x1)
                                                        alpha1 -= 2 * M_PI; 
                                        } else {
                                                alpha0 = M_PI - alpha0;
                                                alpha1 = M_PI - alpha1;
                                                if (position.x - radius < 0)
                                                        alpha1 += 2 * M_PI; 
                                        }
                                        if (alpha1 > alpha0)
                                                segments = (int) (1.0f + 6.0f * (alpha1 - alpha0) / M_PI);
                                        else segments = (int) (1.0f + 6.0f * (alpha0 - alpha1) / M_PI);
                                        d_alpha = (alpha1 - alpha0) / segments;
                                        for (int i = 0; i <= segments; i++) {
                                                float alpha = alpha0 + i * d_alpha;
                                                float y_ = position.y + radius * sinf(alpha);
                                                float x_ = position.x + radius * cosf(alpha);
//                                                path = path_append(path, x_, y_, z);
                                                path.emplace_back(x_, y_, z);
                                                y = y_;
                                        }
                                
                                        break;
                                case INTERSECTS_FIRST_POINT_INSIDE:
                                        r_warn("intersects returned INTERSECTS_FIRST_POINT_INSIDE: should not happen here (at y0, moving to y1)!");
                                        break;
                                case INTERSECTS_SECOND_POINT_INSIDE:
                                        r_warn("INTERSECTS_SECOND_POINT_INSIDE");
                                        if (x < position.x) {
                                                y = ys0;
                                                yt = y;
//                                                path = path_append(path, x, y, z);
                                                path.emplace_back(x, y, z);
                                        } else {
                                                r_warn("x >= p->x");
                                                dy = ys0 - position.y;
                                                alpha0 = asinf(dy / radius);
                                                alpha1 = asinf((y1 - position.y) / radius);
                                                if (x + dx > x1) {
                                                        float alpha2 = -acosf((x1 - position.x) / radius);
                                                        if (alpha2 < alpha1)
                                                                alpha1 = alpha2;
                                                } else if (x + dx < position.x + radius) {
                                                        float alpha2 = -acosf((x + dx - position.x) / radius);
                                                        if (alpha2 < alpha1)
                                                                alpha1 = alpha2;
                                                }
                                                segments = (int) (1.0f + 6.0f * (alpha1 - alpha0) / M_PI);
                                                r_warn("segments=%d", segments);

                                                d_alpha = (alpha1 - alpha0) / segments;
                                                for (int i = 0; i <= segments; i++) {
                                                        float alpha = alpha0 + i * d_alpha;
                                                        y = position.y + radius * sinf(alpha);
                                                        x = position.x + radius * cosf(alpha);
//                                                        path = path_append(path, x, y, z);
                                                        path.emplace_back(x, y, z);
                                                }
                                                yt = y;
                                        
                                        }
                                        break;
                                case INTERSECTS_BOTH_POINTS_INSIDE:
                                        r_warn("TODO: unhandled case: both points inside (at y0, moving to y1)!");
                                        break;
                                default:
                                        r_warn("boustrophedon at y0, moving right unknown deviation. %d",deviation);
                                }
                        }
                        if (y < yt) {
                                y = yt;
//                                path = path_append(path, x, y, z);
                                path.emplace_back(x, y, z);
                        }
                
                        //// at y1, moving right
                        xt = x0 + 2 * count * dx + 2 * dx;
                        if (xt > x1)
                                break;
                        
//                        pos = list_sort(pos, (compare_func_t) smallest_x_first);
                        std::sort( pos.begin( ), pos.end( ), [ ]( const point_t & lhs, const point_t & rhs )
                        {
                            return smallest_x_first(lhs, rhs);
                        });

//                        for (list_t *l = pos; l != nullptr; l = list_next(l)) {
                        for (auto& position : pos) {
                                float xs0, xs1, dxp, dxt, alpha0, alpha1, d_alpha;
                                int segments;
//                                point_t *p = list_get(l, point_t);
                                int deviation = intersects_x(y, x, xt, position, radius, &xs0, &xs1);
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
//                                        path = path_append(path, x, y, z);
                                        path.emplace_back(x, y, z);
                                        // go around
                                        dxp = x - position.x;
                                        alpha0 = acosf(dxp / radius);
                                        if (y < position.y) alpha0 = 2.0 * M_PI - alpha0;
                                        dxt = xt - position.x;
                                        alpha1 = acosf(dxt / radius);
                                        alpha1 = 2.0 * M_PI - alpha1; 
                                        segments = 1 + (int)(6.0 * (alpha1 - alpha0) / M_PI);
                                        d_alpha = (alpha1 - alpha0) / segments;
                                        for (int i = 1; i <= segments; i++) {
                                                float alpha = alpha0 + i * d_alpha;
                                                y = position.y + radius * sinf(alpha);
                                                x = position.x + radius * cosf(alpha);;
//                                                path = path_append(path, x, y, z);
                                                path.emplace_back(x, y, z);
                                        }

                                        
                                        break;
                                case INTERSECTS_BOTH_POINTS_INSIDE:
                                        r_warn("TODO: unhandled case: both points inside (at y1, moving right)!");
                                        break;
                                default:
                                        r_warn("boustrophedon at y1, moving right unknown deviation. %d",deviation);
                                }
                        }
                        if (x < xt) {
                                x = xt;
//                                path = path_append(path, x, y, z);
                                path.emplace_back(x, y, z);
                        }
                
                        count++;
                }

//                delete_list(pos);

                return path;
        }

        
        ////////////////////////////////////////////////////////////////////

//         static void store_path(membuf_t *buffer,
//                                const std::vector<point_t>& points,
//                                __attribute((unused))int h, double scale)
//         {
//                 membuf_printf(buffer, "    <path d=\"");
//                 float x, y;
        
// //                point_t *p = list_get(points, point_t);
//                 auto p  = points[0];

//                 x = p.x * scale;
//                 /* y = h - p->y * scale; */
//                 y = p.y * scale;
//                 membuf_printf(buffer, "M %f,%f L", x, y);

// //                points = list_next(points);
//                 int index = 1;
//                 while (index < points.size()) {
// //                        p = list_get(points, point_t);
//                         p = points[index++];
//                         x = p.x * scale;
//                         y = p.y * scale;
//                         //y = h - p->y * scale;
//                         membuf_printf(buffer, " %f,%f", x, y);
// //                        points = list_next(points);
//                 }

//                 membuf_printf(buffer, "\" id=\"path\" style=\"fill:none;stroke:#0000ce;"
//                               "stroke-width:2;stroke-linecap:butt;"
//                               "stroke-linejoin:miter;stroke-miterlimit:4;"
//                               "stroke-opacity:1;stroke-dasharray:none\" />\n");
//         }

//         static void store_zones(membuf_t *buffer,
//                                 const std::vector<point_t>& positions,
//                                 double radius_zones,
//                                 __attribute((unused))double h, double scale)
//         {
//                 double r = radius_zones * scale;

//                 for (auto& position : positions) {
// //                for (list_t *l = positions; l != NULL; l = list_next(l)) {
// //                        point_t *p = list_get(l, point_t);
//                         double x = position.x * scale;
//                         //double y = h - p->y * scale;
//                         double y = position.y * scale;
//                         membuf_printf(buffer,
//                                       "  <circle "
//                                       "style=\"fill:#ff80ff;fill-opacity:0.25;stroke-width:0\" "
//                                       "cx=\"%.3f\" cy=\"%.3f\" r=\"%.3f\" />\n",
//                                       x, y, r);
//                 }
//         }

//         static void store_svg(ISession &session,
//                               int w, int h,
//                               const char *image,
//                               const std::vector<point_t>& path,
//                               const std::vector<point_t>& positions,
//                               double radius_zones,
//                               double scale)
//         {
//                 membuf_t *buffer = new_membuf();
        
//                 membuf_printf(buffer,
//                               ("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>"
//                                "<svg xmlns:svg=\"http://www.w3.org/2000/svg\" "
//                                "xmlns=\"http://www.w3.org/2000/svg\" "
//                                "xmlns:xlink=\"http://www.w3.org/1999/xlink\" "
//                                "version=\"1.0\" width=\"%dpx\" height=\"%dpx\">\n"),
//                               w, h);
//                 membuf_printf(buffer,
//                               "    <image xlink:href=\"%s\" "
//                               "x=\"0px\" y=\"0px\" width=\"%dpx\" height=\"%dpx\" />\n",
//                               image, w, h);
//                 store_path(buffer, path, h, scale);
//                 store_zones(buffer, positions, radius_zones, h, scale);
//                 membuf_printf(buffer, "</svg>\n");

//                 session.store_svg("path", std::string(membuf_data(buffer), membuf_len(buffer)));
//                 delete_membuf(buffer);
//         }

}
#pragma GCC diagnostic pop


