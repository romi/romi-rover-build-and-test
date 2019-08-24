#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// for mkdir
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

// For log_debug() and clock_datetime
#include <rcom.h>

#include "weeding.h"

////////////////////////////////////////////////////////////////////

// FIXME: in weeder.c
database_t *get_database();
scan_t* scan = NULL;
messagelink_t *get_messagelink_db();

void broadcast_db_message(const char *event,
                          const char *scan_id,
                          const char *fileset_id,
                          const char *file_id)
{
        log_debug("broadcast_db_message: %s, %s, %s, %s",
                  event, scan_id, fileset_id, file_id);
        messagelink_t *bus = get_messagelink_db();
        if (file_id)
                messagelink_send_f(bus,
                                   "{\"event\": \"%s\", "
                                   "\"source\": \"weeder\", "
                                   "\"scan\": \"%s\", "
                                   "\"fileset\": \"%s\", "
                                   "\"file\": \"%s\"}",
                                   event, scan_id, fileset_id, file_id);
        else if (fileset_id)
                messagelink_send_f(bus,
                                   "{\"event\": \"%s\", "
                                   "\"source\": \"weeder\", "
                                   "\"scan\": \"%s\", "
                                   "\"fileset\": \"%s\"}",
                                   event, scan_id, fileset_id);
        else messagelink_send_f(bus,
                                "{\"event\": \"%s\", "
                                "\"source\": \"weeder\", "
                                "\"scan\": \"%s\"}",
                                event, scan_id);
}

static void init_store()
{
        //log_debug("init_store");
        database_t *db = get_database();
        scan = database_new_scan(db);
        broadcast_db_message("new", scan_id(scan), NULL, NULL);
}

static void close_store()
{
        //log_debug("close_store @1");
        if (scan) scan_store(scan);
        //log_debug("close_store @2");
        database_t *db = get_database();
        //log_debug("close_store @3");
        database_unload(db);
        //log_debug("close_store @4");
}

static file_t *create_file(const char *fsid)
{
        //log_debug("create_file @1: fsid=%s", fsid);
        if (scan == NULL)
                return NULL;
        //log_debug("create_file @2");
        fileset_t *fileset = scan_get_fileset(scan, fsid);
        if (fileset == NULL) {
                //log_debug("create_file @3");
                fileset = scan_new_fileset(scan, fsid);
                //log_debug("create_file @4");
                broadcast_db_message("new", scan_id(scan), fileset_id(fileset), NULL);
        }
        //log_debug("create_file @5");
        if (fileset == NULL) 
                return NULL;
        //log_debug("create_file @6");
        file_t *file = fileset_new_file(fileset);
        if (file == NULL)
                return NULL;
        //log_debug("create_file @7");
        file_set_timestamp(file, clock_time());
        //log_debug("create_file @8");
        broadcast_db_message("new", scan_id(scan), fileset_id(fileset), file_id(file));
        return file;
}

static void store_jpg(image_t *image, const char *fsid)
{
        //log_debug("store_jpg @1: fsid=%s", fsid);
        file_t *file = create_file(fsid);
        if (file == NULL)
                return;

        //log_debug("store_jpg @2");
        membuf_t *buffer = new_membuf();
        if (buffer == NULL)
                return;
        if (membuf_assure(buffer, 1024 * 1024) != 0) {
                delete_membuf(buffer);
                return;
	}
        if (image_store_to_mem(image, buffer) != 0) {
                delete_membuf(buffer);
                return;
	}
        
        //log_debug("store_jpg @3");
        file_import_jpeg(file, membuf_data(buffer), membuf_len(buffer));

	delete_membuf(buffer);
        //log_debug("store_jpg @4");
}

static void store_txt(membuf_t *data, const char *fileset)
{
        /* if (!_dir_initialized) */
        /*         return; */
        
        /* static char buffer[1024]; */
        /* rprintf(buffer, sizeof(buffer), "%s/%s", _dir, name); */
        /* membuf_append_zero(data); */
        /* FILE *fp = fopen(buffer, "w"); */
        /* if (fp) { */
        /*         log_debug("Storing data to %s", buffer); */
        /*         fprintf(fp, "%s", membuf_data(data)); */
        /*         fclose(fp); */
        /* } */
}

/* static void store_path(FILE *fp, list_t *points, int h, float scale) */
/* { */
/*         fprintf(fp, "    <path d=\""); */
/*         float x, y; */
        
/*         point_t *p = list_get(points, point_t); */
/*         x = p->x * scale; */
/*         y = h - p->y * scale; */
/*         fprintf(fp, "M %f,%f L", x, y); */
/*         points = list_next(points); */

/*         while (points) { */
/*                 p = list_get(points, point_t); */
/*                 x = p->x * scale; */
/*                 y = h - p->y * scale; */
/*                 fprintf(fp, " %f,%f", x, y); */
/*                 points = list_next(points); */
/*         } */
/*         fprintf(fp, "\" id=\"path\" style=\"fill:none;stroke:#0000ce;" */
/*                 "stroke-width:2;stroke-linecap:butt;" */
/*                 "stroke-linejoin:miter;stroke-miterlimit:4;" */
/*                 "stroke-opacity:1;stroke-dasharray:none\" />"); */
/* } */

static void store_svg(int w, int h, const char *image, list_t *path, float scale)
{
        /* if (!_dir_initialized) */
        /*         return; */
        
        /* static char buffer[1024]; */
        /* rprintf(buffer, sizeof(buffer), "%s/%s", _dir, "path.svg"); */
        
        /* FILE *fp = fopen(buffer, "w"); */
        /* fprintf(fp, ("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>" */
        /*              "<svg xmlns:svg=\"http://www.w3.org/2000/svg\" " */
        /*              "xmlns=\"http://www.w3.org/2000/svg\" " */
        /*              "xmlns:xlink=\"http://www.w3.org/1999/xlink\" " */
        /*              "version=\"1.0\" width=\"%dpx\" height=\"%dpx\">\n"), */
        /*         w, h); */
        /* fprintf(fp, "    <image xlink:href=\"%s\" " */
        /*         "x=\"0px\" y=\"0px\" width=\"%dpx\" height=\"%dpx\" />\n", */
        /*         image, w, h); */
        /* store_path(fp, path, h, scale); */
        /* fprintf(fp, "\n</svg>\n"); */
        /* fclose(fp); */
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

static image_t* image_excess_green(image_t* image)
{
        if (image->type != IMAGE_RGB) {
                fprintf(stderr, "image_excess_green: not a RGB image\n");
                return NULL;
        }

        image_t* exg = new_image_bw(image->width, image->height);
        int len = image->width * image->height;

        float r_max = 0.0f, g_max = 0.0f, b_max = 0.0f;
        for (int i = 0, j = 0; i < len; i++) {
                float r = image->data[j++];
                float g = image->data[j++];
                float b = image->data[j++];
                if (r > r_max) r_max = r;
                if (g > g_max) g_max = g;
                if (b > b_max) b_max = b;
        }

        if (r_max == 0.0f)
                r_max = 1.0f;
        if (g_max == 0.0f)
                g_max = 1.0f;
        if (b_max == 0.0f)
                b_max = 1.0f;

        float exg_min = 2.0f;
        float exg_max = -2.0f;
        
        for (int i = 0, j = 0; i < len; i++) {
                float rn = image->data[j++] / r_max;
                float gn = image->data[j++] / g_max;
                float bn = image->data[j++] / b_max;

                if (0) {
                        exg->data[i] = 3.0f * gn / (rn + gn + bn) - 1.0f;
                } else {
                        float n = rn + gn + bn;
                        float r = rn / n;
                        float g = gn / n;
                        float b = bn / n;
                        float v = 2.0f * g - r - b;

                        if (v < exg_min) exg_min = v;
                        if (v > exg_max) exg_max = v;

                        exg->data[i] = v;
                }
        }
        
        float norm = exg_max - exg_min;
        for (int i = 0; i < len; i++) {
                exg->data[i] = (exg->data[i] - exg_min) / norm;
        }
        
        return exg;
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

image_t *image_convert_bw(image_t *image)
{
        if (image->type == IMAGE_BW) {
                return image_clone(image);
        }
        image_t *bw = new_image_bw(image->width, image->height);
        if (bw == NULL) {
                // FIXME
                return NULL;
        }
        int len = image->width * image->height;
        for (int i = 0, j = 0; i < len; i++) {
                float r = image->data[j++];
                float g = image->data[j++];
                float b = image->data[j++];
                float v = 0.2989f * r + 0.5870f * g + 0.1140f * b;
                bw->data[i] = v;
        }
        return bw;
}

////////////////////////////////////////////////////////////////////////

point_t *new_point(float x, float y, float z)
{
        point_t *p = new_obj(point_t);
        if (p == NULL) return NULL;
        p->x = x;
        p->y = y;
        p->z = z;
        return p;
}

void delete_point(point_t *p)
{
        if (p) delete_obj(p);
}

void point_set(point_t *p, float x, float y, float z)
{
        p->x = x;
        p->y = y;
        p->z = z;
}

////////////////////////////////////////////////////////////////////////

list_t *make_boustrophedon(float width, float height, float dx, float z0)
{
        list_t *path = NULL;
        float x, y, z;

        z = 0.0f;
        x = 0.0f;
        y = height;

        path = list_append(path, new_point(x, y, z));

        z = z0;
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

        y = height;
        x = 0.0f;
        path = list_append(path, new_point(x, y, z));

        return path;
}

list_t *compute_boustrophedon(workspace_t *workspace, float z0)
{
        return make_boustrophedon(workspace->width_mm, workspace->height_mm, 50.0f, z0);  
}

/*
  Returns:

  -1: error (shouldn't happen :)
  0: no intersection, the segment lies outside the circle
  1: segment touches the circle on the border
  2: segment intersects the circle at two positions, ys0 and ys1, with ys0 < ys1
  3: y0 is inside the circle. the segment exits the circle at ys1
  4: y1 is inside the circle. the segment exits the circle at ys0
  FIXME/TODO: 5: no intersection, the segment is contained inside the circle
*/
static int intersects(float x, float y0, float y1,
                      point_t *p, float r,
                      float *ys0p, float *ys1p)
{
        float dx = p->x - x;
        float d2 = dx * dx;
        float r2 = r * r;
        if (d2 > r2)
                return 0;
        if (d2 == r2) {
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
        return -1; // Shouldn't happen
}

static int intersects_x(float y, float x0, float x1,
                        point_t *p, float r,
                        float *xs0p, float *xs1p)
{
        float dy = p->y - y;
        float d2 = dy * dy;
        float r2 = r * r;
        if (d2 > r2)
                return 0;
        if (d2 == r2) {
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
                                    float dx, float z0,
                                    float d_min, list_t *pos)
{
        list_t *path = NULL;
        float x, y, z;
        
        z = 0.0f;
        x = x0;
        y = y1;

        // check whether the starting corner is free. If not, move the
        // starting point.
        pos = list_sort(pos, (compare_func_t) largest_y_first);
        for (list_t *l = pos; l != NULL; l = list_next(l)) {
                float ys0, ys1;
                point_t *p = list_get(l, point_t);
                int deviation = intersects(x, y0, y1, p, d_min, &ys0, &ys1);
                switch (deviation) {
                case -1:
                        fprintf(stderr, "intersects returned -1\n");
                        break;
                case 0: case 1:
                        break;
                case 2:
                        if (y1 - p->y < d_min + 1.0f)
                                y = ys0;
                        break;
                case 3:
                        fprintf(stderr, "starting point: intersects returned 3!\n");
                        break;
                case 4:
                        y = ys0;
                        break;
                }
        }
        
        path = list_append(path, new_point(x, y, z));

        z = z0;
        path = list_append(path, new_point(x, y, z));
        
        while (1) {

                //// at the top, going down
                float yt = y0;
                pos = list_sort(pos, (compare_func_t) largest_y_first);
                for (list_t *l = pos; l != NULL; l = list_next(l)) {
                        float ys0, ys1, dy, alpha0, alpha1, d_alpha;
                        int segments;
                        point_t *p = list_get(l, point_t);
                        int deviation = intersects(x, y0, y, p, d_min, &ys0, &ys1);
                        switch (deviation) {
                        case -1:
                                fprintf(stderr, "intersects returned -1\n");
                                break;
                        case 0: case 1:
                                break;
                        case 2: 
                                dy = ys1 - p->y;
                                alpha0 = asinf(dy / d_min);
                                alpha1 = -alpha0;
                                if (p->x > x) {
                                        alpha0 = M_PI - alpha0;
                                        alpha1 = M_PI - alpha1;
                                        if (p->x - d_min < 0) {
                                                // go around the other way
                                                alpha1 -= 2 * M_PI; 
                                        }
                                } else {
                                        if (p->x + d_min > x1) {
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
                                        float y_ = p->y + d_min * sinf(alpha);
                                        float x_ = p->x + d_min * cosf(alpha);
                                        path = list_append(path, new_point(x_, y_, z));
                                        y = y_;
                                }
                                break;
                        case 3:
                                y = ys1;
                                yt = y;
                                path = list_append(path, new_point(x, y, z));
                                break;
                        case 4:
                                fprintf(stderr, "intersects returned 4: should not happen here!\n");
                                break;
                        }
                }
                if (y > yt) {
                        y = yt;
                        path = list_append(path, new_point(x, y, z));
                }



                //// at the bottom, moving right
                if (x + dx > x1) {
                        break;
                } else {
                        
                        float xt = x + dx;
                        pos = list_sort(pos, (compare_func_t) smallest_x_first);
                        for (list_t *l = pos; l != NULL; l = list_next(l)) {
                                float xs0, xs1;
                                point_t *p = list_get(l, point_t);
                                int deviation = intersects_x(y, x, x + dx, p, d_min, &xs0, &xs1);
                                switch (deviation) {
                                case -1:
                                        fprintf(stderr, "intersects returned -1\n");
                                        break;
                                case 0:
                                case 1:
                                        break;
                                case 2:
                                case 3:
                                case 4:
                                        // move up to pass in front of the zone
                                        y = p->y + d_min;
                                        path = list_append(path, new_point(x, y, z));
                                        break;
                                }
                        }
                        if (x < xt) {
                                x = xt;
                                path = list_append(path, new_point(x, y, z));
                        }
                }


                //// at the bottom, moving up
                yt = y1;
                pos = list_sort(pos, (compare_func_t) smallest_y_first);
                for (list_t *l = pos; l != NULL; l = list_next(l)) {
                        float ys0, ys1, dy, alpha0, alpha1, d_alpha;
                        int segments;
                        point_t *p = list_get(l, point_t);
                        int deviation = intersects(x, y, y1, p, d_min, &ys0, &ys1);
                        switch (deviation) {
                        case -1:
                                fprintf(stderr, "intersects returned -1\n");
                                break;
                        case 0: case 1:
                                break;
                        case 2:
                                dy = ys1 - p->y;
                                alpha1 = asinf(dy / d_min);
                                alpha0 = -alpha1;
                                if (p->x < x) {
                                        if (p->x + d_min > x1)
                                                alpha1 -= 2 * M_PI; 
                                } else {
                                        alpha0 = M_PI - alpha0;
                                        alpha1 = M_PI - alpha1;
                                        if (p->x - d_min < 0)
                                                alpha1 += 2 * M_PI; 
                                }
                                if (alpha1 > alpha0)
                                        segments = (int) (1.0f + 6.0f * (alpha1 - alpha0) / M_PI);
                                else segments = (int) (1.0f + 6.0f * (alpha0 - alpha1) / M_PI);
                                d_alpha = (alpha1 - alpha0) / segments;
                                for (int i = 0; i <= segments; i++) {
                                        float alpha = alpha0 + i * d_alpha;
                                        float y_ = p->y + d_min * sinf(alpha);
                                        float x_ = p->x + d_min * cosf(alpha);
                                        path = list_append(path, new_point(x_, y_, z));
                                        y = y_;
                                }
                                
                                break;
                        case 3:
                                fprintf(stderr, "intersects returned 4: should not happen here!\n");
                                break;
                        case 4:
                                y = ys0;
                                yt = y;
                                path = list_append(path, new_point(x, y, z));
                                break;
                        }
                }
                if (y < yt) {
                        y = yt;
                        path = list_append(path, new_point(x, y, z));
                }
                
                //// at the top, moving right
                if (x + dx > x1) {
                        break;
                } else {
                        
                        float xt = x + dx;
                        pos = list_sort(pos, (compare_func_t) smallest_x_first);
                        for (list_t *l = pos; l != NULL; l = list_next(l)) {
                                float xs0, xs1;
                                point_t *p = list_get(l, point_t);
                                int deviation = intersects_x(y, x, x + dx, p, d_min, &xs0, &xs1);
                                switch (deviation) {
                                case -1:
                                        fprintf(stderr, "intersects returned -1\n");
                                        break;
                                case 0:
                                case 1:
                                        break;
                                case 2:
                                case 3:
                                case 4:
                                        // move down to pass in below of the zone
                                        y = p->y - d_min;
                                        path = list_append(path, new_point(x, y, z));
                                        break;
                                }
                        }
                        if (x < xt) {
                                x = xt;
                                path = list_append(path, new_point(x, y, z));
                        }
                }
                
        }

        z = 0.0f;
        path = list_append(path, new_point(x, y, z));

        y = y1;
        x = x0;
        path = list_append(path, new_point(x, y, z));

        return path;
}

////////////////////////////////////////////////////////////////////////

image_t *get_workspace_view(image_t *camera, workspace_t *workspace)
{
        image_t *rot = image_rotate(camera,
                                    workspace->width,
                                    camera->height - workspace->y0,
                                    workspace->theta);
        image_t *cropped = FIXME_image_crop(rot,
                                            workspace->x0,
                                            camera->height - workspace->y0 - workspace->height,
                                            workspace->width,
                                            workspace->height);
        store_jpg(cropped, "extra");

        image_t *scaled = FIXME_image_scale(cropped, 4);
        store_jpg(scaled, "scaled");

        delete_image(rot);
        delete_image(cropped);

        return scaled;
}

image_t *compute_mask(image_t *image)
{
        image_t* exg = image_excess_green(image);
        store_jpg(exg, "extra");

        float threshold = image_otsu_threshold(exg);
        
        image_t *mask = image_binary(exg, threshold);
        store_jpg(mask, "masked");

        delete_image(exg);
        return mask;
}

image_t *compute_probability_map(image_t *image, int w, float *avg)
{
        image_t *bell = new_image_bw(w, w);
        image_fill(bell, 0, 0.0f);
        image_bell(bell, w/2, w/2, w/6);

        //
        image_t *corr = new_image_bw(image->width, image->height);
        float corr_sum = 0.0f;
        for (int x = 0; x < corr->width; x++) {
                for (int y = 0; y < corr->height; y++) {
                        float v = image_convolution(image, bell,
                                                    x - bell->width/2,
                                                    y - bell->height/2);
                        corr_sum += v;
                        image_set(corr, x, y, 0, v);
                }
        }
        store_jpg(corr, "extra");
        corr_sum /= (corr->width * corr->height);
        
        delete_image(bell);
        *avg = corr_sum;
        return corr;
}

image_t *estimate_pattern_position(image_t *p_map, float d0, point_t *pos, float *p)
{
        float cos_d0 = d0 * cosf(M_PI / 6.0f);
        float sin_d0 = d0 * sinf(M_PI / 6.0f);

        float x_max = 0.0f;
        float y_max = 0.0f;
        float p_max = -1.0f;

        image_t *pattern = new_image_bw((int) d0, (int) cos_d0);
        
        for (float y = 0.0f; y < d0; y += 1.0f) {
                for (float x = 0.0f; x < cos_d0; x += 1.0f) {
                        float v = (image_get(p_map, x, y, 0)
                                   + image_get(p_map, x, y + d0, 0)
                                   + image_get(p_map, x, y + 2.0f * d0, 0)
                                   
                                   + image_get(p_map, x + cos_d0, y - sin_d0, 0)
                                   + image_get(p_map, x + cos_d0, y - sin_d0 + d0, 0)
                                   + image_get(p_map, x + cos_d0, y - sin_d0 + 2.0f * d0, 0)
                                   + image_get(p_map, x + cos_d0, y - sin_d0 + 3.0f * d0, 0)
                                   
                                   + image_get(p_map, x + 2 * cos_d0, y, 0)
                                   + image_get(p_map, x + 2 * cos_d0, y + d0, 0)
                                   + image_get(p_map, x + 2 * cos_d0, y + 2.0f * d0, 0));
                        image_set(pattern, (int) x, (int) y, 0, v);
                        if (v > p_max) {
                                p_max = v;
                                x_max = x;
                                y_max = y;
                        }
                }
        }
        store_jpg(pattern, "extra");

        pos->x = x_max;
        pos->y = y_max;
        *p = p_max;
        
        return pattern;
}

list_t *adjust_positions(image_t *p_map, float d0, point_t *ptn_pos, float delta)
{
        list_t *positions = NULL;

        float cos_d0 = d0 * cosf(M_PI / 6.0f);
        float sin_d0 = d0 * sinf(M_PI / 6.0f);
        int num_pos = 10;
        point_t pos[num_pos];
        float p[num_pos];

        point_set(&pos[0], ptn_pos->x, ptn_pos->y, 0);
        point_set(&pos[1], ptn_pos->x, ptn_pos->y + d0, 0);
        point_set(&pos[2], ptn_pos->x, ptn_pos->y + 2.0f * d0, 0);
        point_set(&pos[3], ptn_pos->x + cos_d0, ptn_pos->y - sin_d0, 0);
        point_set(&pos[4], ptn_pos->x + cos_d0, ptn_pos->y - sin_d0 + d0, 0);
        point_set(&pos[5], ptn_pos->x + cos_d0, ptn_pos->y - sin_d0 + 2.0f * d0, 0);
        point_set(&pos[6], ptn_pos->x + cos_d0, ptn_pos->y - sin_d0 + 3.0f * d0, 0);
        point_set(&pos[7], ptn_pos->x + 2 * cos_d0, ptn_pos->y, 0);
        point_set(&pos[8], ptn_pos->x + 2 * cos_d0, ptn_pos->y + d0, 0);
        point_set(&pos[9], ptn_pos->x + 2 * cos_d0, ptn_pos->y + 2.0f * d0, 0);

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
                        printf("pos[%d]: (%f,%f) -> (%f,%f), P=%f\n",
                               n, pos[n].x, pos[n].y,
                               sum_px / sum, sum_py / sum,
                               p[n]);
                        pos[n].x = sum_px / sum;
                        pos[n].y = sum_py / sum;
                } else {
                        printf("pos[%d]: P=%f\n", n, p[n]);
                }
        }

        for (int n = 0; n < num_pos; n++)
                if (p[n] > 0.004f) {
                        point_t *pt = new_point(pos[n].x, pos[n].y, 0);
                        positions = list_append(positions, pt);
                }
        
        return positions;
}

list_t *compute_path(image_t *camera, workspace_t *workspace,
                     float distance, float z0, float threshold)
{
        list_t *path = NULL;
        list_t *positions = NULL;
        membuf_t *data = new_membuf();

        init_store();
        
        membuf_printf(data, "{\n    \"timestamp\": %f,\n", clock_time());        
        store_jpg(camera, "images");
        
        // rotate and crop the camera image
        image_t *image = get_workspace_view(camera, workspace);

        // compute the binary mask (plants=white, soil=black...)
        image_t *mask = compute_mask(image);
        
        float meters_to_pixels = 1000.0f * image->width / (float) workspace->width_mm;

        // 
        int w = (int) (0.1f * meters_to_pixels);
        float average_prob = 0.0f;
        image_t *p_map = compute_probability_map(mask, w, &average_prob);

        // find best match for quincunx pattern
        float d0 = distance * meters_to_pixels;
        point_t ptn_pos;
        float p_max = -1.0f;
        image_t *pattern = estimate_pattern_position(p_map, d0, &ptn_pos, &p_max);

        // 
        float confidence = (p_max / 10.0f) / average_prob;
        log_info("*** quincunx: confidence: %f\n", confidence);

        membuf_printf(data, "    \"confidence\": \"%f\",\n", (double) confidence);        
        
        // DEBUG
        // FIXME: ugly!
        // Store image of positions + add to data output
        if (1) {
                float black[3] = { 0.0f, 0.0f, 0.0f };
                float cos_d0 = d0 * cosf(M_PI / 6.0f);
                float sin_d0 = d0 * sinf(M_PI / 6.0f);
                image_t *pattern = image_clone(image);
                membuf_printf(data, "    \"quincunx\": [");
                image_circle(pattern, ptn_pos.x, ptn_pos.y, 5.0f, black);
                membuf_printf(data, "[%f,%f], ", ptn_pos.x, ptn_pos.y);
                image_circle(pattern, ptn_pos.x, ptn_pos.y + d0, 5.0f, black);
                membuf_printf(data, "[%f,%f], ", ptn_pos.x, ptn_pos.y + d0);
                image_circle(pattern, ptn_pos.x, ptn_pos.y + 2.0f * d0, 5.0f, black);
                membuf_printf(data, "[%f,%f], ", ptn_pos.x, ptn_pos.y + 2.0f * d0);
                image_circle(pattern, ptn_pos.x + cos_d0, ptn_pos.y - sin_d0, 5.0f, black);
                membuf_printf(data, "[%f,%f], ", ptn_pos.x + cos_d0, ptn_pos.y - sin_d0);
                image_circle(pattern, ptn_pos.x + cos_d0, ptn_pos.y - sin_d0 + d0, 5.0f, black);
                membuf_printf(data, "[%f,%f], ", ptn_pos.x + cos_d0, ptn_pos.y - sin_d0 + d0);
                image_circle(pattern, ptn_pos.x + cos_d0, ptn_pos.y - sin_d0 + 2.0f * d0, 5.0f, black);
                membuf_printf(data, "[%f,%f], ", ptn_pos.x + cos_d0, ptn_pos.y - sin_d0 + 2.0f * d0);
                image_circle(pattern, ptn_pos.x + cos_d0, ptn_pos.y - sin_d0 + 3.0f * d0, 5.0f, black);
                membuf_printf(data, "[%f,%f], ", ptn_pos.x + cos_d0, ptn_pos.y - sin_d0 + 3.0f * d0);
                image_circle(pattern, ptn_pos.x + 2 * cos_d0, ptn_pos.y, 5.0f, black);
                membuf_printf(data, "[%f,%f], ", ptn_pos.x + 2 * cos_d0, ptn_pos.y);
                image_circle(pattern, ptn_pos.x + 2 * cos_d0, ptn_pos.y + d0, 5.0f, black);
                membuf_printf(data, "[%f,%f], ", ptn_pos.x + 2 * cos_d0, ptn_pos.y + d0);
                image_circle(pattern, ptn_pos.x + 2 * cos_d0, ptn_pos.y + 2.0f * d0, 5.0f, black);
                membuf_printf(data, "[%f,%f]]", ptn_pos.x + 2 * cos_d0, ptn_pos.y + 2.0f * d0);
                store_jpg(pattern, "extra");
                delete_image(pattern);
        }

        if (confidence < threshold) {
                /* image_t *adjusted = new_image_bw(image->width, image->height); */
                /* store_jpg(adjusted, "extra"); */
                /* delete_image(adjusted); */
                membuf_printf(data, "\n}\n");
                store_txt(data, "weeding");
                goto cleanup_and_exit;
        }

        // adjust the positions of the points.
        float delta = 0.04f * meters_to_pixels;
        positions = adjust_positions(p_map, d0, &ptn_pos, delta);

        // FIXME: ugly!
        // Store image of adjusted positions + add to data output
        if (1) {
                float black[3] = { 0.0f, 0.0f, 0.0f };
                image_t *adjusted = image_clone(image);
                for (list_t *l = positions; l != NULL; l = list_next(l)) {
                        point_t *p = list_get(l, point_t);
                        image_circle(adjusted, p->x, p->y, 5.0f, black);
                }
                store_jpg(adjusted, "extra");
                delete_image(adjusted);

                membuf_printf(data, ",\n    \"adjusted\": [");
                adjusted = image_clone(image);
                for (list_t *l = positions; l != NULL; l = list_next(l)) {
                        point_t *p = list_get(l, point_t);
                        image_circle(adjusted, p->x, p->y, 0.120f * meters_to_pixels, black);
                        membuf_printf(data, "[%f,%f]", p->x, p->y);
                        if (list_next(l) != NULL) membuf_printf(data, ",");
                }
                membuf_printf(data, "]\n}");
                store_txt(data, "weeding");
                store_jpg(adjusted, "positions");
                delete_image(adjusted);
        }

        // Change the coordinate system to that of the CNC
        for (list_t *l = positions; l != NULL; l = list_next(l)) {
                point_t *p = list_get(l, point_t);
                p->y = image->height - p->y;
                
                p->y = 1000.0f * p->y / meters_to_pixels;
                p->x = 1000.0f * p->x / meters_to_pixels;
        }

        // compute the path of the weeding tool
        path = make_modified_boustrophedon(0.0f, workspace->width_mm,
                                           25.0f, workspace->height_mm - 25.0f, // FIXME: safety border. Could be removed if the algo analyses an area larger than the workspace
                                           50.0f,
                                           z0,
                                           120.0f,
                                           positions);

        // DEBUG
        if (1) {
                for (list_t *l = positions; l != NULL; l = list_next(l)) {
                        point_t *p = list_get(l, point_t);
                        p->y += 20.0f;
                }
                float scale = meters_to_pixels / 1000.0f;
                store_svg(image->width, image->height, "positions.jpg",
                          path, scale);
        }

        // clean up
cleanup_and_exit:
        for (list_t *l = positions; l != NULL; l = list_next(l)) {
                point_t *p = list_get(l, point_t);
                delete_point(p);
        }
        delete_membuf(data);
        delete_list(positions);
        delete_image(image);
        delete_image(mask);
        delete_image(p_map);
        delete_image(pattern);
        close_store();        

        // done
        return path;
}
