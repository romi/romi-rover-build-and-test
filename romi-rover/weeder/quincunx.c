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
#include "quincunx.h"
#include "profiling.h"


////////////////////////////////////////////////////////////////////

typedef struct _quincunx_module_t {
        path_module_t interface;
        double distance_plants;
        double distance_rows;
        double radius_zones;
        double diameter_tool;
        double threshold;
        double meters_to_pixels;
} quincunx_module_t;

static list_t *quincunx_module_compute(path_module_t *module,
                                       image_t *image,
                                       fileset_t *fileset,
                                       membuf_t *message);

static int quincunx_module_set(path_module_t *module,
                               const char *name,
                               json_object_t value,
                               membuf_t *message);

static void delete_quincunx_module(path_module_t *module);

////////////////////////////////////////////////////////////////////

static void store_path(membuf_t *buffer, list_t *points, int h, double scale)
{
        membuf_printf(buffer, "    <path d=\"");
        float x, y;
        
        point_t *p = list_get(points, point_t);
        x = p->x * scale;
        y = h - p->y * scale;
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

static void store_svg(fileset_t *fileset,
                      int w, int h,
                      const char *image,
                      list_t *path,
                      list_t *positions,
                      double radius_zones,
                      double scale)
{
        if (fileset == NULL)
                return;
        
        file_t *file = create_file(fileset, "path");
        if (file == NULL)
                return;
        
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

        file_import_svg(file, membuf_data(buffer), membuf_len(buffer));
        delete_membuf(buffer);
}

static void store_quincunx_metadata(fileset_t *fileset, quincunx_module_t *module, double duration)
{
        fileset_set_metadata_str(fileset, "path-module", "quincunx");
        json_object_t m = json_object_create();
        json_object_setnum(m, "distance-plants", module->distance_plants);
        json_object_setnum(m, "distance-rows", module->distance_rows);
        json_object_setnum(m, "radius-zones", module->radius_zones);
        json_object_setnum(m, "diameter-tool", module->diameter_tool);
        json_object_setnum(m, "duration", duration);
        fileset_set_metadata(fileset, "quincunx", m);
        json_unref(m);
}

static void store_pattern_position(fileset_t *fileset, image_t *image,
                                   float dpx_plants, float dpx_rows,
                                   point_t ptn_pos)
{
        float red[3] = { 1.0f, 0.0f, 0.0f };
        image_t *pattern = image_clone(image);
        for (int row = 0; row < 3; row++) {
                int num_plants = ((row % 2) == 0)? 3 : 4;
                float dy = ((row % 2) == 0)? 0.0f : -dpx_plants/2;
                for (int plant = 0; plant < num_plants; plant++) {
                        float x = ptn_pos.x + row * dpx_rows;
                        float y = ptn_pos.y + dy + plant * dpx_plants;
                        image_circle(pattern, x, y, 5.0f, red);
                }
        }

        store_jpg(fileset, "positions-pattern", pattern);
        delete_image(pattern);
}

static void store_positions_metadata(fileset_t *fileset, list_t *positions,
                                     double meters_to_pixels)
{
        int i;
        json_object_t a;

        if (fileset == NULL)
                return;

        i = 0;
        a = json_array_create();
        for (list_t *l = positions; l != NULL; l = list_next(l), i++) {
                point_t *p = list_get(l, point_t);
                json_object_t q = json_array_create();
                json_array_setnum(q, p->x, 0);
                json_array_setnum(q, p->y, 1);
                json_array_set(a, q, i);
                json_unref(q);
        }
        fileset_set_metadata(fileset, "positions_image", a);
        json_unref(a);

        i = 0;
        a = json_array_create();
        for (list_t *l = positions; l != NULL; l = list_next(l), i++) {
                point_t *p = list_get(l, point_t);
                json_object_t q = json_array_create();
                json_array_setnum(q, p->x / meters_to_pixels, 0);
                json_array_setnum(q, p->y / meters_to_pixels, 1);
                json_array_set(a, q, i);
                json_unref(q);
        }
        fileset_set_metadata(fileset, "positions_workspace", a);
        json_unref(a);
}

static void store_adjusted_positions(fileset_t *fileset, image_t *image,
                                     list_t *positions, double meters_to_pixels)
{
        float red[3] = { 1.0f, 0.0f, 0.0f };
        image_t *adjusted;

        // 1
        adjusted = image_clone(image);
        for (list_t *l = positions; l != NULL; l = list_next(l)) {
                point_t *p = list_get(l, point_t);
                image_circle(adjusted, p->x, p->y, 5.0f, red);
        }
        store_jpg(fileset, "positions", adjusted);
        delete_image(adjusted);

        // 2
        store_positions_metadata(fileset, positions, meters_to_pixels);
}


////////////////////////////////////////////////////////////////////

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

static image_t *compute_convolution(fileset_t *fileset, image_t *image, int w, float *avg)
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

                {
                        image_t *im = image_clone(corr);
                        int len = im->width * im->height;
                        for (int i = 0; i < len; i++)
                                im->data[i] /= corr_max;
                        store_png(fileset, "position-probability-map", im);
                        delete_image(im);
                }
        } else {
                corr = image_clone(image);
        }
        
        delete_image(bell);
        return corr;
}

static image_t *estimate_pattern_position(fileset_t *fileset,
                                          image_t *p_map,
                                          float d_plants,
                                          float d_rows,                                   
                                          point_t *pos, float *p)
{
        int x_max = 0;
        int y_max = 0;
        float p_max = -1.0f;

        int dr = (int) d_rows;
        int dp = (int) d_plants;
        int w = dr + 1;
        int h = dp + 1;
        
        image_t *pattern = new_image_bw(w, h);

        r_debug("d_rows %d, d_plants %d", dr, dp);

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
                        
                        image_set(pattern, (int) x, (int) y, 0, v);
                        if (v > p_max) {
                                r_debug("x=%d, y=%d, p=%f", x, y, v);
                                p_max = v;
                                x_max = x;
                                y_max = y;
                        }
                }
        }
        store_png(fileset, "probability-positions", pattern);

        pos->x = (float) x_max;
        pos->y = (float) y_max;
        *p = p_max;
        
        return pattern;
}

static list_t *adjust_positions(image_t *p_map,
                                float distance_plants,
                                float distance_rows,
                                point_t *ptn_pos, float delta)
{
        list_t *positions = NULL;

        int num_pos = 10;
        point_t pos[num_pos];
        float p[num_pos];

        int i = 0;
        for (int row = 0; row < 3; row++) {
                int num_plants = ((row % 2) == 0)? 3 : 4;
                float dy = ((row % 2) == 0)? 0.0f : -distance_plants/2;
                for (int plant = 0; plant < num_plants; plant++) {
                        float x = ptn_pos->x + row * distance_rows;
                        float y = ptn_pos->y + dy + plant * distance_plants;
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

        for (int n = 0; n < num_pos; n++)
                if (p[n] > 0.003f) {
                        point_t *pt = new_point(pos[n].x, pos[n].y, 0);
                        positions = list_append(positions, pt);
                }
        
        return positions;
}

static list_t *compute_quincunx_positions(quincunx_module_t *module,
                                          fileset_t *fileset,
                                          image_t *mask,
                                          float *confidence)
{
        list_t *positions = NULL;
        //double meters_to_pixels = mask->width / workspace->width_meter;

        // 
        int w = (int) (0.1f * module->meters_to_pixels);
        float average_prob = 0.0f;
        image_t *p_map = compute_convolution(fileset, mask, w, &average_prob);

        // find best match for quincunx pattern
        double dpx_plants = module->distance_plants * module->meters_to_pixels;
        double dpx_rows = module->distance_rows * module->meters_to_pixels;
        point_t ptn_pos;
        float p_max = -1.0f;
        image_t *pattern = estimate_pattern_position(fileset, p_map,
                                                     dpx_plants, dpx_rows,
                                                     &ptn_pos, &p_max);

        /* json_object_setnum(performance, "positions-pattern", clock_time() - start_time); */
        store_pattern_position(fileset, mask, dpx_plants, dpx_rows, ptn_pos);
        
        // 
        *confidence = (p_max / 10.0f) / average_prob;
        
        r_info("quincunx: p_max %f", (double) p_max);
        r_info("quincunx: average_prob %f", (double) average_prob);
        r_info("quincunx: confidence: %f", (double) *confidence);

        // adjust the positions of the points.
        float delta = 0.04f * module->meters_to_pixels;
        positions = adjust_positions(p_map, dpx_plants, dpx_rows, &ptn_pos, delta);
        
        /* json_object_setnum(performance, "positions-adjusted", clock_time() - start_time); */
        store_adjusted_positions(fileset, mask, positions, module->meters_to_pixels);
        
        delete_image(p_map);
        delete_image(pattern);
        
        return positions;
}

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
static int intersects(float x, float y0, float y1,
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
                return 0;
        if (fabs(d2 - r2) < delta * delta) {
                float ys = p->y;
                if (p->y <= y0 || p->y >= y1)
                        return 0;
                else {
                        *ys0p = ys;
                        return 1;
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
                return 0;
        if (ys1 == y0)
                return 1;
        if (ys0 > y1)
                return 0;
        if (ys0 == y1)
                return 1;
        if ((y0 <= ys0 && ys0 < y1) && (y0 < ys1 && ys1 <= y1))
                return 2;
        if ((ys0 < y0) && (y0 < ys1 && ys1 <= y1))
                return 3;
        if ((y0 <= ys0 && ys0 < y1) && ys1 > y1)
                return 4;
        if (y0 > ys0 && y1 < ys1)
                return 5;
        
        return -1; // Shouldn't happen
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
                return 0;
        if (fabs(d2 - r2) < delta * delta) {
                float xs = p->x;
                if (p->x <= x0 || p->x >= x1)
                        return 0;
                else {
                        *xs0p = xs;
                        return 1;
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
                return 0;
        if (xs1 == x0)
                return 1;
        if (xs0 > x1)
                return 0;
        if (xs0 == x1)
                return 1;
        if ((x0 <= xs0 && xs0 < x1) && (x0 < xs1 && xs1 <= x1))
                return 2;
        if ((xs0 < x0) && (x0 < xs1 && xs1 <= x1))
                return 3;
        if ((x0 <= xs0 && xs0 < x1) && xs1 > x1)
                return 4;
        if (x0 > xs0 && x1 < xs1)
                return 5;
        return -1; // Shouldn't happen
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

static list_t *quincunx_boustrophedon(float x0, float x1, 
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
                int deviation = intersects(x, y0, y1, p, radius, &ys0, &ys1);
                switch (deviation) {
                case -1:
                        r_warn("intersects returned -1");
                        break;
                case 0: case 1:
                        break;
                case 2:
                        if (y1 - p->y < radius + 1.0f)
                                y = ys0;
                        break;
                case 3:
                        r_warn("starting point: intersects returned 3!");
                        break;
                case 4:
                        y = ys0;
                        break;
                }
        }
        
        path = list_append(path, new_point(x, y, z));
        
        while (1) {

                //// at the top, going down
                float yt = y0, xt;
                pos = list_sort(pos, (compare_func_t) largest_y_first);
                for (list_t *l = pos; l != NULL; l = list_next(l)) {
                        float ys0, ys1, dy, alpha0, alpha1, d_alpha;
                        int segments;
                        point_t *p = list_get(l, point_t);
                        int deviation = intersects(x, y0, y, p, radius, &ys0, &ys1);
                        switch (deviation) {
                        case -1:
                                r_warn("intersects returned -1");
                                break;
                        case 0: case 1:
                                break;
                        case 2: 
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
                                        path = list_append(path, new_point(x_, y_, z));
                                        y = y_;
                                }
                                break;
                        case 3:
                                if (x < p->x) {
                                        y = ys1;
                                        yt = y;
                                        path = list_append(path, new_point(x, y, z));
                                } else {
                                        // move to the edge
                                        y = ys1;
                                        path = list_append(path, new_point(x, y, z));
                                        // go around
                                        dy = ys1 - p->y;
                                        alpha0 = asinf(dy / radius);
                                        alpha1 = asinf((y0 - p->y) / radius);
                                        if (x + dx < p->x + radius) {
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
                                                path = list_append(path, new_point(x, y, z));
                                        }
                                        yt = y;
                                        
                                }
                                break;
                        case 4:
                                r_warn("intersects returned 4: should not happen here (at top, moving down)!");
                                break;
                        }
                }
                if (y > yt) {
                        y = yt;
                        path = list_append(path, new_point(x, y, z));
                }



                //// at the bottom, moving right
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
                        case -1:
                                r_warn("intersects returned -1");
                                break;
                        case 0:
                        case 1:
                                break;
                        case 2:
                        case 3:
                                // Should not happen
                                r_warn("intersects returned 2 or 3: should not happen here (at bottom, moving right)!");
                                break;
                        case 4:
                                // move to the edge
                                x = xs0;
                                path = list_append(path, new_point(x, y, z));
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
                                        path = list_append(path, new_point(x, y, z));
                                }
                                break;
                        }
                }
                if (x < xt) {
                        x = xt;
                        path = list_append(path, new_point(x, y, z));
                }


                //// at the bottom, moving up
                yt = y1;
                pos = list_sort(pos, (compare_func_t) smallest_y_first);
                for (list_t *l = pos; l != NULL; l = list_next(l)) {
                        float ys0, ys1, dy, alpha0, alpha1, d_alpha;
                        int segments;
                        point_t *p = list_get(l, point_t);
                        int deviation = intersects(x, y, y1, p, radius, &ys0, &ys1);
                        switch (deviation) {
                        case -1:
                                r_warn("intersects returned -1");
                                break;
                        case 0: case 1:
                                break;
                        case 2:
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
                                        path = list_append(path, new_point(x_, y_, z));
                                        y = y_;
                                }
                                
                                break;
                        case 3:
                                r_warn("intersects returned 3: should not happen here (at bottom, moving up)!");
                                break;
                        case 4:
                                if (x < p->x) {
                                        y = ys0;
                                        yt = y;
                                        path = list_append(path, new_point(x, y, z));
                                } else {
                                        dy = ys0 - p->y;
                                        alpha0 = asinf(dy / radius);
                                        alpha1 = asinf((y1 - p->y) / radius);
                                        if (x + dx < p->x + radius) {
                                                float alpha2 = -acosf((x + dx - p->x) / radius);
                                                if (alpha2 < alpha1)
                                                        alpha1 = alpha2;
                                        }
                                        segments = (int) (1.0f + 6.0f * (alpha1 - alpha0) / M_PI);
                                        d_alpha = (alpha1 - alpha0) / segments;
                                        for (int i = 0; i <= segments; i++) {
                                                float alpha = alpha0 + i * d_alpha;
                                                y = p->y + radius * sinf(alpha);
                                                x = p->x + radius * cosf(alpha);
                                                path = list_append(path, new_point(x, y, z));
                                        }
                                        yt = y;
                                        
                                }
                                break;
                        }
                }
                if (y < yt) {
                        y = yt;
                        path = list_append(path, new_point(x, y, z));
                }
                
                //// at the top, moving right
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
                        case -1:
                                r_warn("intersects returned -1");
                                break;
                        case 0:
                        case 1:
                                break;
                        case 2:
                        case 3:
                                // Should not happen
                                r_warn("intersects returned 2 or 3: should not happen here (at top, moving right)!");
                                break;
                        case 4:
                                // move down to pass in below of the zone
                                //y = p->y - radius;
                                //path = list_append(path, new_point(x, y, z));
                                        
                                // move to the edge
                                x = xs0;
                                path = list_append(path, new_point(x, y, z));
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
                                        path = list_append(path, new_point(x, y, z));
                                }

                                        
                                break;
                        }
                }
                if (x < xt) {
                        x = xt;
                        path = list_append(path, new_point(x, y, z));
                }
                
                
                count++;
        }

        delete_list(pos);

        return path;
}

static list_t *quincunx_module_compute(path_module_t *m,
                                       image_t *mask,
                                       fileset_t *fileset,
                                       membuf_t *message)
{
        list_t *path = NULL;
        list_t *positions = NULL;
        quincunx_module_t *module;
        float confidence = 0.0f;
        double start_time = clock_time();
        
        module = (quincunx_module_t *) m;

        confidence = 0.0f;
        positions = compute_quincunx_positions(module, fileset, mask, &confidence);

        if (positions == NULL)
                goto cleanup_and_exit;

        float diameter = (float) (module->meters_to_pixels * module->diameter_tool);
        float radius = (float) (module->meters_to_pixels * module->radius_zones);
        float border = diameter / 2.0f;
        path = quincunx_boustrophedon(0.0f, (float) mask->width,
                                      border, (float) mask->height - border, 
                                      diameter, radius, positions);
        if (path == NULL)
                goto cleanup_and_exit;

        store_svg(fileset, mask->width, mask->height, 
                  "00002.jpg", path, positions,
                  radius, 1.0f);

cleanup_and_exit:
        for (list_t *l = positions; l != NULL; l = list_next(l)) {
                point_t *p = list_get(l, point_t);
                delete_point(p);
        }
        delete_list(positions);

        double duration = clock_time() - start_time;
        store_quincunx_metadata(fileset, module, duration);
        
        // done
        return path;
}

static int quincunx_module_set(path_module_t *m,
                               const char *name,
                               json_object_t value,
                               membuf_t *message)
{
        quincunx_module_t *module = (quincunx_module_t *) m;

        if (rstreq(name, "distance-plants")) {
                if (!json_isnumber(value)
                    || json_number_value(value) < 0.01
                    || json_number_value(value) > 2.0) {
                        r_err("Invalid distance between plants");
                        membuf_printf(message, "Invalid distance between rows");
                        return -1;
                }
                module->distance_plants = json_number_value(value);
                return 1;
                
        } else if (rstreq(name, "distance-rows")) {
                if (!json_isnumber(value)
                    || json_number_value(value) < 0.01
                    || json_number_value(value) > 2.0) {
                        r_err("Invalid distance between plants");
                        membuf_printf(message, "Invalid distance between rows");
                        return -1;
                }
                module->distance_rows = json_number_value(value);
                return 1;

        } else if (rstreq(name, "radius-zones")) {
                if (!json_isnumber(value)
                    || json_number_value(value) < 0.01
                    || json_number_value(value) > 2.0) {
                        r_err("Invalid plant radius");
                        membuf_printf(message, "Invalid plant radius");
                        return -1;
                }
                module->radius_zones = json_number_value(value);
                return 1;

        } else if (rstreq(name, "diameter-tool")) {
                if (!json_isnumber(value)
                    || json_number_value(value) < 0.01
                    || json_number_value(value) > 1.0) {
                        r_err("Invalid tool diameter");
                        membuf_printf(message, "Invalid tool diameter");
                        return -1;
                }
                module->diameter_tool = json_number_value(value);
                return 1;

        } else if (rstreq(name, "quincunx-threshold")) {
                if (!json_isnumber(value)
                    || json_number_value(value) < 0.0
                    || json_number_value(value) > 100000.0) {
                        r_err("Invalid threshold");
                        membuf_printf(message, "Invalid threshold");
                        return -1;
                }
                module->threshold = json_number_value(value);
                return 1;
        }
        
        return 0;
}

path_module_t *new_quincunx_module(double distance_plants,
                                   double distance_rows,
                                   double radius_zones,
                                   double diameter_tool,
                                   double threshold,
                                   double meters_to_pixels)
{
        quincunx_module_t *module; 
        module = r_new(quincunx_module_t);
        if (module == NULL)
                return NULL;
        module->interface.compute = quincunx_module_compute;
        module->interface.cleanup = delete_quincunx_module;
        module->interface.set = quincunx_module_set;
        module->distance_plants = distance_plants;
        module->distance_rows = distance_rows;
        module->radius_zones = radius_zones;
        module->diameter_tool = diameter_tool;
        module->threshold = threshold;
        module->meters_to_pixels = meters_to_pixels;
        return (path_module_t *) module;
}

static void delete_quincunx_module(path_module_t *module)
{
        if (module)
                r_delete(module);
}
