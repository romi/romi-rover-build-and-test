#include "planner.h"
#include "plotter.h"
#include "v.h"

typedef struct _rect_t {
        double x;
        double y;
        double w;
        double h;
} rect_t;

typedef struct _svg_t {
        membuf_t *buffer;

        double w;
        double h;
        double scale;
        
        double d; // margin
        double L; // max(xmax)
        
        rect_t xy;
        rect_t xz;
        rect_t yz;

        rect_t v[3];
        rect_t a[3];

} svg_t;

static void delete_svg(svg_t *svg);

static svg_t *new_svg()
{
        svg_t *svg = r_new(svg_t);
        if (svg == NULL)
                return NULL;
        
        svg->buffer = new_membuf();
        if (svg->buffer == NULL) {
                delete_svg(svg);
                return NULL;
        }

        return svg;
}

static void delete_svg(svg_t *svg)
{
        if (svg) {
                if (svg->buffer)
                        delete_membuf(svg->buffer);
                r_delete(svg);
        }
}

static int svg_open(svg_t *svg)
{
        int w = (int) (svg->scale * svg->w + 0.5);
        int h = (int) (svg->scale * svg->h + 0.5);
        membuf_clear(svg->buffer);        
        membuf_printf(svg->buffer,
                      ("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n"
                       "<svg xmlns:svg=\"http://www.w3.org/2000/svg\"\n"
                       "  xmlns=\"http://www.w3.org/2000/svg\"\n"
                       "  xmlns:xlink=\"http://www.w3.org/1999/xlink\"\n"
                       "  xmlns:sodipodi=\"http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd\"\n"
                       "  xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\"\n"
                       "  version=\"1.0\" width=\"%dpx\" height=\"%dpx\">\n"),
                      w, h);
        
        membuf_printf(svg->buffer,
                      ("  <sodipodi:namedview id=\"namedview\" showgrid=\"true\">\n"
                       "    <inkscape:grid type=\"xygrid\" id=\"zgrid\" \n"
                       "        spacingx=\"0.1\" spacingy=\"0.1\" empspacing=\"10\" />\n"
                       "  </sodipodi:namedview>\n"));
        
        membuf_printf(svg->buffer,
                      "  <g transform=\"matrix(%f,0,0,-%f,0,%d)\"\n"
                      "    inkscape:groupmode=\"layer\"\n"
                      "    inkscape:label=\"graphs\"\n"
                      "    id=\"graphs\">\n",
                      svg->scale, svg->scale, h);
        return 0;
}

static int svg_close(svg_t *svg)
{
        membuf_printf(svg->buffer, "  </g>\n");
        membuf_printf(svg->buffer, "</svg>\n");
        membuf_append_zero(svg->buffer);        
        return 0;
}

static int svg_store(svg_t *svg, const char *filename)
{
        FILE *fp = fopen(filename, "w");
        if (fp == NULL) {
                r_warn("Failed to open file '%s'", filename);
                return -1;
        }
        fprintf(fp, "%s", membuf_data(svg->buffer));
        fclose(fp);
        return 0;
}

static void print_moveto(svg_t *svg, double x, double y)
{
        membuf_printf(svg->buffer, "M %f,%f ", x, y);
}

static void print_lineto(svg_t *svg, double x, double y)
{
        membuf_printf(svg->buffer, "L %f,%f ", x, y);
}

static void print_line(svg_t *svg, double x0, double y0, double x1, double y1, char *color)
{
        membuf_printf(svg->buffer, "        <path d=\"");
        print_moveto(svg, x0, y0);
        print_lineto(svg, x1, y1);
        membuf_printf(svg->buffer, "\" id=\"path\" style=\"fill:none;stroke:%s;"
                      "stroke-width:0.001;stroke-linecap:butt;"
                      "stroke-linejoin:miter;stroke-miterlimit:4;"
                      "stroke-opacity:1;stroke-dasharray:none\" />\n",
                      color);
}

static void print_rect(svg_t *svg, double x, double y, double w, double h)
{
        membuf_printf(svg->buffer,
                      "      <rect x=\"%f\" y=\"%f\" width=\"%f\" height=\"%f\" "
                      "style=\"fill:none;stroke:#cecece;"
                      "stroke-width:0.001;stroke-linecap:butt;"
                      "stroke-linejoin:miter;stroke-miterlimit:4;"
                      "stroke-opacity:1;stroke-dasharray:none\" />\n",
                      x, y, w, h);
}

static void print_atdc_ij(svg_t *svg, atdc_t *path,
                          int i, int j, rect_t *r)
{
        membuf_printf(svg->buffer, "    <g transform=\"translate(%f %f)\">\n",
                      r->x, r->y);
        
        for (atdc_t *t0 = path; t0; t0 = t0->next) {
                print_line(svg,
                           t0->accelerate.p0[i], t0->accelerate.p0[j],
                           t0->accelerate.p1[i], t0->accelerate.p1[j],
                           "#00ff00");
                        
                print_line(svg,
                           t0->travel.p0[i], t0->travel.p0[j],
                           t0->travel.p1[i], t0->travel.p1[j],
                           "#ffff00");
                        
                print_line(svg,
                           t0->decelerate.p0[i], t0->decelerate.p0[j],
                           t0->decelerate.p1[i], t0->decelerate.p1[j],
                           "#ff0000");
                
                membuf_printf(svg->buffer, "        <path d=\"");
                print_moveto(svg, t0->curve.p0[i], t0->curve.p0[j]);

                int ms = (int) (1000.0 * t0->curve.t);
                int n = ms / 10;
                for (int k = 1; k < n; k++) {
                        double x[3];
                        double tmp[3];
                        double t = 10.0 * k * 0.001;
                        smul(x, t0->curve.v0, t);
                        smul(tmp, t0->curve.a, 0.5 * t * t);
                        vadd(x, x, tmp);
                        vadd(x, x, t0->curve.p0); 
                        print_lineto(svg, x[i], x[j]);
                }
                
                print_lineto(svg, t0->curve.p1[i], t0->curve.p1[j]);
                membuf_printf(svg->buffer, "\" id=\"path\" style=\"fill:none;stroke:#0000ce;"
                              "stroke-width:0.001;stroke-linecap:butt;"
                              "stroke-linejoin:miter;stroke-miterlimit:4;"
                              "stroke-opacity:1;stroke-dasharray:none\" />\n");
        }
        
        membuf_printf(svg->buffer, "    </g>\n");
}

static void print_atdc(svg_t *svg, atdc_t *atdc)
{
        print_atdc_ij(svg, atdc, 0, 1, &svg->xy);
        print_atdc_ij(svg, atdc, 0, 2, &svg->xz);
        print_atdc_ij(svg, atdc, 1, 2, &svg->yz);
}

static void print_atdc_list(svg_t *svg, list_t *atdc_list)
{
        membuf_printf(svg->buffer,
                      "    <g inkscape:groupmode=\"layer\" "
                      "inkscape:label=\"atdc\" "
                      "id=\"atdc\">\n");
        
        list_t *m = atdc_list;
        while (m != NULL) {
                atdc_t *atdc = list_get(m, atdc_t);
                print_atdc(svg, atdc);
                m = list_next(m);
        }
        membuf_printf(svg->buffer, "    </g>\n");
}



static void print_path_ij(svg_t *svg, segment_t *path,
                          int i, int j, rect_t *r)
{
        membuf_printf(svg->buffer, "    <g transform=\"translate(%f %f)\">\n",
                      r->x, r->y);
        membuf_printf(svg->buffer, "        <path d=\"");
        
        for (segment_t *s0 = path; s0; s0 = s0->next) {
                if (s0->prev == NULL)
                        print_moveto(svg, s0->section.p0[i], s0->section.p0[j]);
                else 
                        print_lineto(svg, s0->section.p0[i], s0->section.p0[j]);
                print_lineto(svg, s0->section.p1[i], s0->section.p1[j]);
        }
        
        membuf_printf(svg->buffer, "\" id=\"path\" style=\"fill:none;stroke:#000000;"
                      "stroke-width:0.001;stroke-linecap:butt;"
                      "stroke-linejoin:miter;stroke-miterlimit:4;"
                      "stroke-opacity:1;stroke-dasharray:none\" />\n");
        membuf_printf(svg->buffer, "    </g>\n");
}

static void print_path(svg_t *svg, segment_t *path)
{
        print_path_ij(svg, path, 0, 1, &svg->xy);
        print_path_ij(svg, path, 0, 2, &svg->xz);        
        print_path_ij(svg, path, 1, 2, &svg->yz);
}

static void print_path_list(svg_t *svg, list_t *paths)
{
        membuf_printf(svg->buffer,
                      "    <g inkscape:groupmode=\"layer\" "
                      "inkscape:label=\"paths\" "
                      "id=\"paths\">\n");
        
        list_t *l = paths;
        while (l != NULL) {
                segment_t *path = list_get(l, segment_t);
                print_path(svg, path);
                l = list_next(l);
        }

        membuf_printf(svg->buffer, "    </g>\n");
}


static double atdc_duration(atdc_t *atdc)
{
        double t = 0.0;
        for (atdc_t *t0 = atdc; t0; t0 = t0->next) {
                t += (t0->accelerate.t
                      + t0->travel.t
                      + t0->decelerate.t
                      + t0->curve.t);
        }
        return t;
}

static double atdc_list_duration(list_t *atdc_list)
{
        double duration = 0.0;
        list_t *m = atdc_list;
        while (m != NULL) {
                atdc_t *atdc = list_get(m, atdc_t);
                duration += atdc_duration(atdc);
                m = list_next(m);
        }
        return duration;
}

static double slices_duration(list_t *list)
{
        list_t *last = list_last(list);
        section_t *section = list_get(last, section_t);
        return section->at + section->t;
}

static void print_path_speed_i(svg_t *svg,
                                segment_t *path,
                                atdc_t *atdc,
                                int i,
                                double duration,
                                double *vm)
{
        double xscale = svg->v[i].w / duration;
        double yscale = svg->v[i].h / vmax(vm);
        segment_t *s0 = path;
        atdc_t *a0 = atdc;
        double t = 0.0;

        membuf_printf(svg->buffer, "    <g transform=\"translate(%f %f)\">\n",
                      svg->v[i].x, svg->v[i].y);

        // curve
        while (s0) {
                double t0 = t;
                double t1 = t + a0->accelerate.t;
                print_line(svg,
                           xscale * t0,
                           yscale * a0->accelerate.v0[i],
                           xscale * t1,
                           yscale * a0->accelerate.v1[i],
                           "#00ff00");
                
                t0 = t1;
                t1 += a0->travel.t;
                print_line(svg,
                           xscale * t0,
                           yscale * a0->travel.v0[i],
                           xscale * t1,
                           yscale * a0->travel.v1[i],
                           "#ffff00");
                        
                t0 = t1;
                t1 += a0->decelerate.t;
                print_line(svg,
                           xscale * t0,
                           yscale * a0->decelerate.v0[i],
                           xscale * t1,
                           yscale * a0->decelerate.v1[i],
                           "#ff0000");

                t0 = t1;
                t1 += a0->curve.t;
                print_line(svg,
                           xscale * t0,
                           yscale * a0->curve.v0[i],
                           xscale * t1,
                           yscale * a0->curve.v1[i],
                           "#0000ce");
                
                print_line(svg,
                           xscale * t,
                           yscale * s0->section.v0[i],
                           xscale * t1,
                           yscale * s0->section.v0[i],
                           "#000000");
               
                s0 = s0->next;
                a0 = a0->next;
                t = t1;
        }
        membuf_printf(svg->buffer, "    </g>\n");
}

static void print_path_speeds(svg_t *svg,
                              segment_t *path,
                              atdc_t *atdc,
                              double duration,
                              double *vmax)
{
        for (int i = 0; i < 3; i++)
                print_path_speed_i(svg, path, atdc, i, duration, vmax);
}

static void print_path_acceleration_i(svg_t *svg,
                                      segment_t *path,
                                      atdc_t *atdc,
                                      int i,
                                      double duration,
                                      double *amax)
{
        double xscale = svg->a[i].w / duration;
        double yscale = svg->a[i].h / vmax(amax);
        segment_t *s0 = path;
        atdc_t *a0 = atdc;
        double t = 0.0;

        membuf_printf(svg->buffer, "    <g transform=\"translate(%f %f)\">\n",
                      svg->a[i].x, svg->a[i].y);

        // curve
        while (s0) {
                double t0 = t;
                double t1 = t + a0->accelerate.t;
                print_line(svg,
                           xscale * t0,
                           yscale * a0->accelerate.a[i],
                           xscale * t1,
                           yscale * a0->accelerate.a[i],
                           "#00ff00");
                
                t0 = t1;
                t1 += a0->travel.t;
                /* print_line(svg, */
                /*            xscale * t0, */
                /*            yscale * a0->travel.a[i], */
                /*            xscale * t1, */
                /*            yscale * a0->travel.a[i], */
                /*            "#ffff00"); */
                        
                t0 = t1;
                t1 += a0->decelerate.t;
                print_line(svg,
                           xscale * t0,
                           yscale * a0->decelerate.a[i],
                           xscale * t1,
                           yscale * a0->decelerate.a[i],
                           "#ff0000");

                t0 = t1;
                t1 += a0->curve.t;
                print_line(svg,
                           xscale * t0,
                           yscale * a0->curve.a[i],
                           xscale * t1,
                           yscale * a0->curve.a[i],
                           "#0000ce");
                               
                s0 = s0->next;
                a0 = a0->next;
                t = t1;
        }
        membuf_printf(svg->buffer, "    </g>\n");
}

static void print_path_accelerations(svg_t *svg,
                                     segment_t *path,
                                     atdc_t *atdc,
                                     double duration,
                                     double *amax)
{
        for (int i = 0; i < 3; i++)
                print_path_acceleration_i(svg, path, atdc, i, duration, amax);
}

static void print_slices_speed_i(svg_t *svg, list_t *slices, int i,
                                 double duration, double *vm)
{
        double xscale = svg->v[i].w / duration;
        double yscale = svg->v[i].h / vmax(vm);
        
        membuf_printf(svg->buffer, "    <g transform=\"translate(%f %f)\">\n",
                      svg->v[i].x, svg->v[i].y);

        for (list_t *l = slices; l; l = list_next(l)) {
                section_t *section = list_get(l, section_t);
                print_line(svg,
                           xscale * section->at, yscale * section->v0[i],
                           xscale * (section->at + section->t), yscale * section->v1[i], 
                           "#ff00ff");
        }
        
        membuf_printf(svg->buffer, "    </g>\n");
}

static void print_slices_ij(svg_t *svg, list_t *slices, int i, int j, rect_t *r)
{
        membuf_printf(svg->buffer, "    <g transform=\"translate(%f %f)\">\n",
                      r->x, r->y);

        for (list_t *l = slices; l; l = list_next(l)) {
                section_t *section = list_get(l, section_t);
                print_line(svg,
                           section->p0[i], section->p0[j],
                           section->p1[i], section->p1[j],
                           "#ff00ff");
        }
        
        membuf_printf(svg->buffer, "    </g>\n");
}

static void print_slices(svg_t *svg, list_t *slices, double duration, double *vm)
{
        membuf_printf(svg->buffer,
                      "    <g inkscape:groupmode=\"layer\" "
                      "inkscape:label=\"slices\" "
                      "id=\"slices\">\n");

        print_slices_ij(svg, slices, 0, 1, &svg->xy);
        /* print_blocks_ij(svg, blocks, n, 0, 2, 2 * d + L, y0, scale); */
        /* print_blocks_ij(svg, blocks, n, 1, 2, 3 * d + 2 * L, y0, scale); */
        print_slices_speed_i(svg, slices, 0, duration, vm);
        print_slices_speed_i(svg, slices, 1, duration, vm);
        print_slices_speed_i(svg, slices, 2, duration, vm);

        membuf_printf(svg->buffer, "    </g>\n");
}

static void print_blocks_ij(svg_t *svg, block_t *blocks, int n, int i, int j,
                            rect_t *r, double *scale)
{
        membuf_printf(svg->buffer, "    <g transform=\"translate(%f %f)\">\n",
                      r->x, r->y);

        int x = 0;
        int y = 0;

        membuf_printf(svg->buffer, "        <path d=\"");
        print_moveto(svg, 0, 0);
        
        for (int k = 0; k < n; k++) {
                switch (blocks[k].type) {
                case BLOCK_MOVE:
                        print_lineto(svg,
                                     (x + blocks[k].data[i+1]) / scale[i],
                                     (y + blocks[k].data[j+1]) / scale[j]);
                        x += blocks[k].data[1];
                        y += blocks[k].data[2];
                        break;
                default:
                        break;
                }
        }
                
        membuf_printf(svg->buffer, "\" id=\"path\" style=\"fill:none;stroke:%s;"
                      "stroke-width:0.001;stroke-linecap:butt;"
                      "stroke-linejoin:miter;stroke-miterlimit:4;"
                      "stroke-opacity:1;stroke-dasharray:none\" />\n",
                      "#00ffff");
        membuf_printf(svg->buffer, "    </g>\n");
}

static void print_blocks(svg_t *svg, block_t *blocks, int n, double L, double *scale)
{
        membuf_printf(svg->buffer,
                      "    <g inkscape:groupmode=\"layer\" "
                      "inkscape:label=\"blocks\" "
                      "id=\"blocks\">\n");
        
        print_blocks_ij(svg, blocks, n, 0, 1, &svg->xy, scale);
        /* print_blocks_ij(svg, blocks, n, 0, 2, 2 * d + L, y0, scale); */
        /* print_blocks_ij(svg, blocks, n, 1, 2, 3 * d + 2 * L, y0, scale); */

        membuf_printf(svg->buffer, "    </g>\n");
}

static void print_axes(svg_t *svg)
{
        membuf_printf(svg->buffer,
                      "    <g inkscape:groupmode=\"layer\" "
                      "inkscape:label=\"axes\" "
                      "id=\"axes\">\n");
        
        // xy
        print_rect(svg, svg->xy.x, svg->xy.y, svg->xy.w, svg->xy.h);

        // xz
        print_rect(svg, svg->xz.x, svg->xz.y, svg->xz.w, svg->xz.h);

        // yz
        print_rect(svg, svg->yz.x, svg->yz.y, svg->yz.w, svg->yz.h);

        // vx
        print_line(svg,
                   svg->v[0].x, svg->v[0].y,
                   svg->v[0].x + svg->v[0].w, svg->v[0].y,
                   "#000000");
        
        print_line(svg,
                   svg->v[0].x, svg->v[0].y - svg->v[0].h,
                   svg->v[0].x, svg->v[0].y + svg->v[0].h,
                   "#000000");
        
        // vy
        print_line(svg,
                   svg->v[1].x, svg->v[1].y,
                   svg->v[1].x + svg->v[1].w, svg->v[1].y,
                   "#000000");
        
        print_line(svg,
                   svg->v[1].x, svg->v[1].y - svg->v[1].h,
                   svg->v[1].x, svg->v[1].y + svg->v[1].h,
                   "#000000");
        
        // vz
        print_line(svg,
                   svg->v[2].x, svg->v[2].y,
                   svg->v[2].x + svg->v[2].w, svg->v[2].y,
                   "#000000");
        
        print_line(svg,
                   svg->v[2].x, svg->v[2].y - svg->v[2].h,
                   svg->v[2].x, svg->v[2].y + svg->v[2].h,
                   "#000000");

        // ax
        print_line(svg,
                   svg->a[0].x, svg->a[0].y,
                   svg->a[0].x + svg->a[0].w, svg->a[0].y,
                   "#000000");
        
        print_line(svg,
                   svg->a[0].x, svg->a[0].y - svg->a[0].h,
                   svg->a[0].x, svg->a[0].y + svg->a[0].h,
                   "#000000");
        
        // ay
        print_line(svg,
                   svg->a[1].x, svg->a[1].y,
                   svg->a[1].x + svg->a[1].w, svg->a[1].y,
                   "#000000");
        
        print_line(svg,
                   svg->a[1].x, svg->a[1].y - svg->a[1].h,
                   svg->a[1].x, svg->a[1].y + svg->a[1].h,
                   "#000000");
        
        // az
        print_line(svg,
                   svg->a[2].x, svg->a[2].y,
                   svg->a[2].x + svg->a[2].w, svg->a[2].y,
                   "#000000");
        
        print_line(svg,
                   svg->a[2].x, svg->a[2].y - svg->a[2].h,
                   svg->a[2].x, svg->a[2].y + svg->a[2].h,
                   "#000000");
        
        membuf_printf(svg->buffer, "    </g>\n");
}

int print_paths(const char *filepath,
                list_t *paths,
                list_t *atdc_list,
                list_t *slices,
                block_t *blocks,
                int num_blocks,
                double *xmax,
                double *vm,
                double *amax,
                double *scale)
{
        svg_t *svg = new_svg();
        if (svg == NULL)
                return -1;

        svg->d = 0.1;
        svg->L = vmax(xmax);
        
        svg->w = 4 * svg->d + xmax[0] + xmax[0] + xmax[1];
        svg->h = 3 * svg->d + vmax(xmax) + 0.7;
        svg->scale = 1000.0;
        
        svg->xy.x = svg->d;
        svg->xy.y = svg->h - svg->d - svg->L;
        svg->xy.w = xmax[0];
        svg->xy.h = xmax[1];
        
        svg->xz.x = svg->xy.x + svg->xy.w + svg->d;
        svg->xz.y = svg->xy.y;
        svg->xz.w = xmax[0];
        svg->xz.h = xmax[2];
        
        svg->yz.x = svg->xz.x + svg->xz.w + svg->d;
        svg->yz.y = svg->xy.y;
        svg->yz.w = xmax[1];
        svg->yz.h = xmax[2];
        
        svg->v[0].x = svg->d;
        svg->v[0].y = 7.5 * svg->d;
        svg->v[0].w = svg->w - 2 * svg->d;
        svg->v[0].h = 0.9 * svg->d / 2.0;
        
        svg->v[1].x = svg->d;
        svg->v[1].y = 6.5 * svg->d;
        svg->v[1].w = svg->w - 2 * svg->d;
        svg->v[1].h = 0.9 * svg->d / 2.0;
        
        svg->v[2].x = svg->d;
        svg->v[2].y = 5.5 * svg->d;
        svg->v[2].w = svg->w - 2 * svg->d;
        svg->v[2].h = 0.9 * svg->d / 2.0;
        
        svg->a[0].x = svg->d;
        svg->a[0].y = 3.5 * svg->d;
        svg->a[0].w = svg->w - 2 * svg->d;
        svg->a[0].h = 0.9 * svg->d / 2.0;
        
        svg->a[1].x = svg->d;
        svg->a[1].y = 2.5 * svg->d;
        svg->a[1].w = svg->w - 2 * svg->d;
        svg->a[1].h = 0.9 * svg->d / 2.0;
        
        svg->a[2].x = svg->d;
        svg->a[2].y = 1.5 * svg->d;
        svg->a[2].w = svg->w - 2 * svg->d;
        svg->a[2].h = 0.9 * svg->d / 2.0;

        
        svg_open(svg);

        print_axes(svg);

        double duration = 1.0;

        if (atdc_list)
                duration = atdc_list_duration(atdc_list);
        else if (slices)
                duration = slices_duration(slices);

        if (paths)
                print_path_list(svg, paths);
        
        if (atdc_list)
                print_atdc_list(svg, atdc_list);

        if (slices != NULL)
                print_slices(svg, slices, duration, vm);

        if (blocks != NULL)
                print_blocks(svg, blocks, num_blocks, xmax[0], scale);

        
        list_t *l = paths;
        list_t *m = atdc_list;
        while (l != NULL && m != NULL) {
                segment_t *path = list_get(l, segment_t);
                atdc_t *atdc = list_get(m, atdc_t);
                print_path_speeds(svg, path, atdc, duration, vm);
                print_path_accelerations(svg, path, atdc, duration, amax);
                l = list_next(l);
                m = list_next(m);
        }

        svg_close(svg);

        svg_store(svg, filepath);
        delete_svg(svg);
}


