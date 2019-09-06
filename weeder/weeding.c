#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <romi.h>
#include "weeding.h"

////////////////////////////////////////////////////////////////////

static int store_workspace(fileset_t *fileset, workspace_t *workspace)
{
        json_object_t w = workspace_to_json(workspace);
        int err = fileset_set_metadata(fileset, "workspace", w);
        json_unref(w);
        return err;
}

static fileset_t *create_fileset(scan_t *session, const char *fileset_id,
                                 workspace_t *workspace, float distance_plants,
                                 float distance_rows, float radius, double start_time)
{
        if (session == NULL) {
                r_debug("create_fileset: session is null");
                return NULL;
        }
        
        fileset_t *fileset = NULL;
        const char *s = fileset_id;
        char id[64];

        if (fileset_id == NULL) {
                int num = 0;
                while (num < 100000) {
                        rprintf(id, sizeof(id), "weeding-%05d", num);
                        fileset = scan_get_fileset(session, id);
                        if (fileset == NULL)
                                break;
                        num++;
                }
                s = id;
        }

        r_info("Storing weeding files in fileset '%s'", s);

        fileset = scan_get_fileset(session, s);

        if (fileset == NULL)
                fileset = scan_new_fileset(session, s);
        if (fileset == NULL)
                return NULL;
        
        store_workspace(fileset, workspace);
        fileset_set_metadata_str(fileset, "method", "quincunx");
        fileset_set_metadata_num(fileset, "distance-plants", distance_plants);
        fileset_set_metadata_num(fileset, "distance-rows", distance_rows);
        fileset_set_metadata_num(fileset, "radius-zone", radius);
        fileset_set_metadata_num(fileset, "timestamp", start_time);
        return fileset;
}

static void store_jpg(fileset_t *fileset, const char *name, image_t *image)
{
        fileset_store_image(fileset, name, image, "jpg");
}

static void store_png(fileset_t *fileset, const char *name, image_t *image)
{
        fileset_store_image(fileset, name, image, "png");
}

static file_t *create_file(fileset_t *fileset, const char *name)
{
        file_t *file = fileset_new_file(fileset);
        if (file == NULL)
                return NULL;
        file_set_timestamp(file, clock_time());
        file_set_metadata_str(file, "name", name);
        return file;
}

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
                y = h - p->y * scale;
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
                double y = h - p->y * scale;
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
        if (buffer == NULL)
                return;
        
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
}

////////////////////////////////////////////////////////////////////

int workspace_parse(workspace_t *workspace, json_object_t w)
{
        if (json_falsy(w) || !json_isarray(w)) {
                r_err("Invalid workspace configuration");
                return -1;
        }
        
        double theta = json_array_getnum(w, 0);
        double x0 = json_array_getnum(w, 1);
        double y0 = json_array_getnum(w, 2);
        double width = json_array_getnum(w, 3);
        double height = json_array_getnum(w, 4);
        double width_meter = json_array_getnum(w, 5);
        double height_meter = json_array_getnum(w, 6);
        if (isnan(theta) || isnan(x0) || isnan(y0)
            || isnan(width) || isnan(height)
            || isnan(width_meter) || isnan(height_meter)) {
                r_err("Invalid workspace values");
                return -1;
        }

        workspace->theta = theta;
        workspace->x0 = (int) x0;
        workspace->y0 = (int) y0;
        workspace->width = (int) width;
        workspace->height = (int) height;
        workspace->width_meter = width_meter;
        workspace->height_meter = height_meter;

        r_debug("workspace: theta %.6f, x0 %.0f, y0 %.0f, "
                  "width %.0f px / %.3f m, height %.0f px / %.3f m", 
                  theta, x0, y0, width, width_meter, height, height_meter);

        return 0;
}

json_object_t workspace_to_json(workspace_t *workspace)
{
        json_object_t w = json_array_create();
        json_array_setnum(w, workspace->theta, 0);
        json_array_setnum(w, workspace->x0, 1);
        json_array_setnum(w, workspace->y0, 2);
        json_array_setnum(w, workspace->width, 3);
        json_array_setnum(w, workspace->height, 4);
        json_array_setnum(w, workspace->width_meter, 5);
        json_array_setnum(w, workspace->height_meter, 6);
        return w;
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

// see http://www.labbookpages.co.uk/software/imgProc/otsuThreshold.html
static float image_otsu_threshold(image_t* image)
{
        if (image->type != IMAGE_BW) {
                fprintf(stderr, "image_otsu_threshold: not a BW image\n");
                return 0.0f;
        }

        // Calculate histogram
        int histogram[256];
        int len = image->width * image->height;

        memset(histogram, 0, sizeof(histogram));
        
        for (int i = 0; i < len; i++) {
                int bin = (int) roundf(image->data[i] * 255.0f);
                if (bin >= 0 && bin < 256)
                        histogram[bin]++;
        }

        int total = image->width * image->height;
        float sum = 0.0f;
        for (int i = 0 ; i < 256; i++)
                sum += i * histogram[i];

        float sumB = 0;
        int wB = 0;
        int wF = 0;

        float var_max = 0;
        int threshold = 0;

        for (int t = 0; t < 256; t++) {
                wB += histogram[t];               // Weight Background
                if (wB == 0) continue;

                wF = total - wB;                 // Weight Foreground
                if (wF == 0) break;

                sumB += (float) (t * histogram[t]);

                float mB = sumB / wB;            // Mean Background
                float mF = (sum - sumB) / wF;    // Mean Foreground

                // Calculate Between Class Variance
                float var_between = (float) wB * (float) wF * (mB - mF) * (mB - mF);

                // Check if new maximum found
                if (var_between > var_max) {
                        var_max = var_between;
                        threshold = t;
                }
        }

        return (float) threshold / 255.0f;
}

////////////////////////////////////////////////////////////////////////

point_t *new_point(float x, float y, float z)
{
        point_t *p = r_new(point_t);
        if (p == NULL) return NULL;
        p->x = x;
        p->y = y;
        p->z = z;
        return p;
}

void delete_point(point_t *p)
{
        if (p) r_delete(p);
}

void point_set(point_t *p, float x, float y, float z)
{
        p->x = x;
        p->y = y;
        p->z = z;
}

////////////////////////////////////////////////////////////////////////

list_t *make_boustrophedon(double width, double height, double dx)
{
        list_t *path = NULL;
        double x, y, z;

        z = 0.0f;
        x = 0.0f;
        y = height;

        path = list_append(path, new_point(x, y, z));
        
        while (1) {
                y -= height;
                path = list_append(path, new_point(x, y, z));

                if (x + dx <= width) {
                        x += dx;
                        path = list_append(path, new_point(x, y, z));
                } else break;

                y += height;
                path = list_append(path, new_point(x, y, z));

                if (x + dx <= width) {
                        x += dx;
                        path = list_append(path, new_point(x, y, z));
                } else break;
        }

        z = 0.0f;
        path = list_append(path, new_point(x, y, z));

        return path;
}

list_t *compute_boustrophedon(workspace_t *workspace, double diameter_tool)
{
        return make_boustrophedon(workspace->width_meter,
                                  workspace->height_meter,
                                  diameter_tool);  
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

int largest_y_first(point_t *pa, point_t *pb)
{
        return (pa->y > pb->y)? -1 : (pa->y == pb->y)? 0 : 1;
}

int smallest_y_first(point_t *pa, point_t *pb)
{
        return (pa->y < pb->y)? -1 : (pa->y == pb->y)? 0 : 1;
}

int smallest_x_first(point_t *pa, point_t *pb)
{
        return (pa->x < pb->x)? -1 : (pa->x == pb->x)? 0 : 1;
}

list_t *make_modified_boustrophedon(float x0, float x1, 
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

////////////////////////////////////////////////////////////////////////

image_t *get_workspace_view(fileset_t *fileset, image_t *camera, workspace_t *workspace)
{
        store_jpg(fileset, "camera", camera);
        
        image_t *rot = image_rotate(camera,
                                    workspace->width,
                                    camera->height - workspace->y0,
                                    workspace->theta);
        image_t *cropped = FIXME_image_crop(rot,
                                            workspace->x0,
                                            camera->height - workspace->y0 - workspace->height,
                                            workspace->width,
                                            workspace->height);
        store_jpg(fileset, "cropped", cropped);

        //image_t *scaled = FIXME_image_scale(cropped, 4);
        image_t *scaled = FIXME_image_scale(cropped, 3);
        store_jpg(fileset, "scaled", scaled);

        delete_image(rot);
        delete_image(cropped);

        return scaled;
}

image_t *compute_mask(fileset_t *fileset, image_t *image)
{
        image_t* exg = image_excess_green(image);
        store_png(fileset, "excess-green", exg);

        float threshold = image_otsu_threshold(exg);
        
        image_t *mask = image_binary(exg, threshold);
        store_png(fileset, "mask", mask);

        delete_image(exg);
        return mask;
}

image_t *compute_probability_map(fileset_t *fileset, image_t *image, int w, float *avg)
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

                image_t *im = image_clone(corr);
                int len = im->width * im->height;
                for (int i = 0; i < len; i++)
                        im->data[i] /= corr_max;
                store_png(fileset, "position-probability-map", im);
        } else {
                corr = image_clone(image);
        }
        
        delete_image(bell);
        return corr;
}

image_t *estimate_pattern_position(fileset_t *fileset,
                                   image_t *p_map,
                                   float d_plants,
                                   float d_rows,                                   
                                   point_t *pos, float *p)
{
        float x_max = 0.0f;
        float y_max = 0.0f;
        float p_max = -1.0f;

        image_t *pattern = new_image_bw((int) d_rows, (int) d_plants);
        
        for (float y = 0.0f; y < d_plants; y += 1.0f) {
                for (float x = 0.0f; x < d_rows; x += 1.0f) {
                        float v = (image_get(p_map, x, y, 0)
                                   + image_get(p_map, x, y + d_plants, 0)
                                   + image_get(p_map, x, y + 2.0f * d_plants, 0)
                                   
                                   + image_get(p_map, x + d_rows, y - d_plants/2, 0)
                                   + image_get(p_map, x + d_rows, y - d_plants/2 + d_plants, 0)
                                   + image_get(p_map, x + d_rows, y - d_plants/2 + 2.0f * d_plants, 0)
                                   + image_get(p_map, x + d_rows, y - d_plants/2 + 3.0f * d_plants, 0)
                                   
                                   + image_get(p_map, x + 2 * d_rows, y, 0)
                                   + image_get(p_map, x + 2 * d_rows, y + d_plants, 0)
                                   + image_get(p_map, x + 2 * d_rows, y + 2.0f * d_plants, 0));
                        
                        image_set(pattern, (int) x, (int) y, 0, v);
                        if (v > p_max) {
                                p_max = v;
                                x_max = x;
                                y_max = y;
                        }
                }
        }
        store_png(fileset, "probability-positions", pattern);

        pos->x = x_max;
        pos->y = y_max;
        *p = p_max;
        
        return pattern;
}

list_t *adjust_positions(image_t *p_map,
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
                if (p[n] > 0.004f) {
                        point_t *pt = new_point(pos[n].x, pos[n].y, 0);
                        positions = list_append(positions, pt);
                }
        
        return positions;
}

void store_pattern_position(fileset_t *fileset, image_t *image, float dpx_plants, float dpx_rows, point_t ptn_pos)
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

void store_positions_metadata(fileset_t *fileset, list_t *positions, double meters_to_pixels)
{
        int i;
        json_object_t a;

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

void store_adjusted_positions(fileset_t *fileset, image_t *image,
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

list_t *compute_quincunx_positions(fileset_t *fileset,
                                   image_t *image,
                                   image_t *mask,
                                   workspace_t *workspace,
                                   double distance_plants,
                                   double distance_rows,
                                   json_object_t performance,
                                   double start_time,
                                   float *confidence)
{
        list_t *positions = NULL;
        double meters_to_pixels = mask->width / workspace->width_meter;

        // 
        int w = (int) (0.1f * meters_to_pixels);
        float average_prob = 0.0f;
        image_t *p_map = compute_probability_map(fileset, mask, w, &average_prob);

        // find best match for quincunx pattern
        double dpx_plants = distance_plants * meters_to_pixels;
        double dpx_rows = distance_rows * meters_to_pixels;
        point_t ptn_pos;
        float p_max = -1.0f;
        image_t *pattern = estimate_pattern_position(fileset, p_map,
                                                     dpx_plants, dpx_rows,
                                                     &ptn_pos, &p_max);

        json_object_setnum(performance, "positions-pattern", clock_time() - start_time);
        store_pattern_position(fileset, image, dpx_plants, dpx_rows, ptn_pos);
        
        // 
        *confidence = (p_max / 10.0f) / average_prob;
        r_info("quincunx: confidence: %f", *confidence);

        // adjust the positions of the points.
        float delta = 0.04f * meters_to_pixels;
        positions = adjust_positions(p_map, dpx_plants, dpx_rows, &ptn_pos, delta);
        
        json_object_setnum(performance, "positions-adjusted", clock_time() - start_time);
        store_adjusted_positions(fileset, image, positions, meters_to_pixels);

        // Change the coordinates from pixels to meters
        for (list_t *l = positions; l != NULL; l = list_next(l)) {
                point_t *p = list_get(l, point_t);
                p->y = mask->height - p->y;
                p->y = (float) workspace->height_meter * p->y / (float) mask->height;
                p->x = (float) workspace->width_meter * p->x / (float) mask->width;
        }
        
        delete_image(p_map);
        delete_image(pattern);
        
        return positions;
}

list_t *compute_path(scan_t *session, const char *fileset_id, image_t *camera,
                     workspace_t *workspace, double distance_plants,
                     double distance_rows, double radius_zones,
                     double diameter_tool, float *confidence)
{
        list_t *path = NULL;
        image_t *image = NULL;
        image_t *mask = NULL;
        list_t *positions = NULL;
        double start_time = clock_time();

        // If fileset is NULL, continue anyway
        fileset_t *fileset = create_fileset(session, fileset_id,
                                            workspace,  distance_plants,
                                            distance_rows, radius_zones, start_time);
        json_object_t performance = json_object_create();
        
        // 1. preprocessing: rotate, crop, and scale the camera image
        image = get_workspace_view(fileset, camera, workspace);
        if (image == NULL) goto cleanup_and_exit;
        
        json_object_setnum(performance, "preprocessing", clock_time() - start_time);

        // 2. compute the binary mask (plants=white, soil=black...)
        mask = compute_mask(fileset, image);
        if (mask == NULL) goto cleanup_and_exit;

        json_object_setnum(performance, "segmentation", clock_time() - start_time);

        // 3. compute the positions and/or contours
        positions = compute_quincunx_positions(fileset, image, mask, workspace,
                                               distance_plants, distance_rows,
                                               performance, start_time, confidence);
        if (positions == NULL) goto cleanup_and_exit;

        json_object_setnum(performance, "contours", clock_time() - start_time);

        // 4. compute the path of the weeding tool
        double border = diameter_tool / 2;
        path = make_modified_boustrophedon(0.0f, workspace->width_meter,
                                           border, workspace->height_meter - border, 
                                           diameter_tool, radius_zones,
                                           positions);
        if (path == NULL) goto cleanup_and_exit;
        

        double scale = (double) image->width / workspace->width_meter;
        store_svg(fileset, image->width, image->height, "00002.jpg" /*FIXME*/,
                  path, positions, radius_zones, scale);

        json_object_setnum(performance, "path", clock_time() - start_time);
        fileset_set_metadata(fileset, "performance", performance);
        
        // Cleaning up
cleanup_and_exit:
        for (list_t *l = positions; l != NULL; l = list_next(l)) {
                point_t *p = list_get(l, point_t);
                delete_point(p);
        }
        delete_list(positions);
        delete_image(image);
        delete_image(mask);
        json_unref(performance);

        // done
        return path;
}
