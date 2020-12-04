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
#include "planner.h"
#include "plotter.h"
#include "v.h"

typedef struct _rect_t {
        // the coordinates to the rectable on the page
        double x, y, w, h;

        // the min and max coordinates of the data to be plotted
        double x0, x1;
        double y0, y1;
} rect_t;

typedef struct _plot_t {
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

} plot_t;

static void delete_plot(plot_t *plot);

static plot_t *new_plot(membuf_t *buffer)
{
        plot_t *plot = r_new(plot_t);
        if (plot == NULL)
                return NULL;
        plot->buffer = buffer;
        return plot;
}

static void delete_plot(plot_t *plot)
{
        if (plot)
                r_delete(plot);
}

static int plot_open(plot_t *plot)
{
        int w = (int) (plot->scale * plot->w + 0.5);
        int h = (int) (plot->scale * plot->h + 0.5);
        membuf_clear(plot->buffer);        
        membuf_printf(plot->buffer,
                      ("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n"
                       "<svg xmlns:svg=\"http://www.w3.org/2000/svg\"\n"
                       "  xmlns=\"http://www.w3.org/2000/svg\"\n"
                       "  xmlns:xlink=\"http://www.w3.org/1999/xlink\"\n"
                       "  xmlns:sodipodi=\"http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd\"\n"
                       "  xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\"\n"
                       "  version=\"1.0\" width=\"%dpx\" height=\"%dpx\">\n"),
                      w, h);
        
        membuf_printf(plot->buffer,
                      ("  <sodipodi:namedview id=\"namedview\" showgrid=\"false\">\n"
                       "    <inkscape:grid type=\"xygrid\" id=\"zgrid\" \n"
                       "        spacingx=\"0.1\" spacingy=\"0.1\" empspacing=\"10\" />\n"
                       "  </sodipodi:namedview>\n"));
        
        membuf_printf(plot->buffer,
                      "  <g transform=\"matrix(%f,0,0,-%f,0,%d)\"\n"
                      "    inkscape:groupmode=\"layer\"\n"
                      "    inkscape:label=\"graphs\"\n"
                      "    id=\"graphs\">\n",
                      plot->scale, plot->scale, h);
        return 0;
}

static int plot_close(plot_t *plot)
{
        membuf_printf(plot->buffer, "  </g>\n");
        membuf_printf(plot->buffer, "</svg>\n");
        membuf_append_zero(plot->buffer);        
        return 0;
}

static void print_moveto(plot_t *plot, double x, double y)
{
        membuf_printf(plot->buffer, "M %f,%f ", x, y);
}

static void print_lineto(plot_t *plot, double x, double y)
{
        membuf_printf(plot->buffer, "L %f,%f ", x, y);
}

static void print_line(plot_t *plot, double x0, double y0, double x1, double y1, char *color)
{
        membuf_printf(plot->buffer, "        <path d=\"");
        print_moveto(plot, x0, y0);
        print_lineto(plot, x1, y1);
        membuf_printf(plot->buffer, "\" id=\"path\" style=\"fill:none;stroke:%s;"
                      "stroke-width:0.001;stroke-linecap:butt;"
                      "stroke-linejoin:miter;stroke-miterlimit:4;"
                      "stroke-opacity:1;stroke-dasharray:none\" />\n",
                      color);
}

static void print_rect(plot_t *plot, double x, double y, double w, double h)
{
        membuf_printf(plot->buffer,
                      "      <rect x=\"%f\" y=\"%f\" width=\"%f\" height=\"%f\" "
                      "style=\"fill:none;stroke:#cecece;"
                      "stroke-width:0.002;stroke-linecap:butt;"
                      "stroke-linejoin:miter;stroke-miterlimit:4;"
                      "stroke-opacity:1;stroke-dasharray:none\" />\n",
                      x, y, w, h);
}

static void print_text(plot_t *plot, const char *s, double x, double y)
{
        membuf_printf(plot->buffer,
                      "      <g transform=\"matrix(1,0,0,-1,0,%f)\" >"
                      "<text x=\"%f\" y=\"%f\" "
                      "style=\"font-weight:300;"
                      "font-size:0.03px;"
                      "font-family:'serif';"
                      "font-style:italic;"
                      "fill:#000000;"
                      "text-anchor:end;"
                      "text-align:end;"
                      "fill-opacity:1;\" >"
                      "%s</text></g>\n",
                      2 * y, x, y, s);
}

#define _X(_r, _v) (((_r)->x1 - (_r)->x0 != 0.0)? (_r)->w * ((_v) - (_r)->x0) / ((_r)->x1 - (_r)->x0) : (_r)->x0)
#define _Y(_r, _v) (((_r)->y1 - (_r)->y0 != 0.0)? (_r)->h * ((_v) - (_r)->y0) / ((_r)->y1 - (_r)->y0) : (_r)->y0)

static void print_atdc_ij(plot_t *plot, atdc_t *path,
                          int i, int j, rect_t *r)
{
        membuf_printf(plot->buffer, "    <g transform=\"translate(%f %f)\">\n",
                      r->x, r->y);
        
        for (atdc_t *t0 = path; t0; t0 = t0->next) {
                print_line(plot,
                           _X(r, t0->accelerate.p0[i]), _Y(r, t0->accelerate.p0[j]),
                           _X(r, t0->accelerate.p1[i]), _Y(r, t0->accelerate.p1[j]),
                           "#00ff00");
                        
                print_line(plot,
                           _X(r, t0->travel.p0[i]), _Y(r, t0->travel.p0[j]),
                           _X(r, t0->travel.p1[i]), _Y(r, t0->travel.p1[j]),
                           "#ffff00");
                        
                print_line(plot,
                           _X(r, t0->decelerate.p0[i]), _Y(r, t0->decelerate.p0[j]),
                           _X(r, t0->decelerate.p1[i]), _Y(r, t0->decelerate.p1[j]),
                           "#ff0000");
                
                membuf_printf(plot->buffer, "        <path d=\"");
                print_moveto(plot, _X(r, t0->curve.p0[i]), _Y(r, t0->curve.p0[j]));

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
                        print_lineto(plot, _X(r, x[i]), _Y(r, x[j]));
                }
                
                print_lineto(plot, _X(r, t0->curve.p1[i]), _Y(r, t0->curve.p1[j]));
                membuf_printf(plot->buffer, "\" id=\"path\" style=\"fill:none;stroke:#0000ce;"
                              "stroke-width:0.001;stroke-linecap:butt;"
                              "stroke-linejoin:miter;stroke-miterlimit:4;"
                              "stroke-opacity:1;stroke-dasharray:none\" />\n");
        }
        
        membuf_printf(plot->buffer, "    </g>\n");
}

static void print_atdc(plot_t *plot, atdc_t *atdc)
{
        print_atdc_ij(plot, atdc, 0, 1, &plot->xy);
        print_atdc_ij(plot, atdc, 0, 2, &plot->xz);
        print_atdc_ij(plot, atdc, 1, 2, &plot->yz);
}

static void print_atdc_list(plot_t *plot, list_t *atdc_list)
{
        membuf_printf(plot->buffer,
                      "    <g inkscape:groupmode=\"layer\" "
                      "inkscape:label=\"atdc\" "
                      "id=\"atdc\">\n");
        
        list_t *m = atdc_list;
        while (m != NULL) {
                atdc_t *atdc = list_get(m, atdc_t);
                print_atdc(plot, atdc);
                m = list_next(m);
        }
        membuf_printf(plot->buffer, "    </g>\n");
}

static void print_path_ij(plot_t *plot, segment_t *path,
                          int i, int j, rect_t *r)
{
        membuf_printf(plot->buffer, "    <g transform=\"translate(%f %f)\">\n",
                      r->x, r->y);
        membuf_printf(plot->buffer, "        <path d=\"");
        
        for (segment_t *s0 = path; s0; s0 = s0->next) {
                if (s0->prev == NULL)
                        print_moveto(plot, _X(r, s0->section.p0[i]), _Y(r, s0->section.p0[j]));
                else 
                        print_lineto(plot, _X(r, s0->section.p0[i]), _Y(r, s0->section.p0[j]));
                print_lineto(plot, _X(r, s0->section.p1[i]), _Y(r, s0->section.p1[j]));
        }
        
        membuf_printf(plot->buffer, "\" id=\"path\" style=\"fill:none;stroke:#000000;"
                      "stroke-width:0.001;stroke-linecap:butt;"
                      "stroke-linejoin:miter;stroke-miterlimit:4;"
                      "stroke-opacity:1;stroke-dasharray:none\" />\n");
        membuf_printf(plot->buffer, "    </g>\n");
}

static void print_path(plot_t *plot, segment_t *path)
{
        print_path_ij(plot, path, 0, 1, &plot->xy);
        print_path_ij(plot, path, 0, 2, &plot->xz);        
        print_path_ij(plot, path, 1, 2, &plot->yz);
}

static void print_path_list(plot_t *plot, list_t *paths)
{
        membuf_printf(plot->buffer,
                      "    <g inkscape:groupmode=\"layer\" "
                      "inkscape:label=\"paths\" "
                      "id=\"paths\">\n");
        
        list_t *l = paths;
        while (l != NULL) {
                segment_t *path = list_get(l, segment_t);
                print_path(plot, path);
                l = list_next(l);
        }

        membuf_printf(plot->buffer, "    </g>\n");
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

static void print_path_speed_i(plot_t *plot,
                                segment_t *path,
                                atdc_t *atdc,
                                int i,
                                double duration,
                                double *vm)
{
        double xscale = plot->v[i].w / duration;
        double yscale = plot->v[i].h / vmax(vm);
        segment_t *s0 = path;
        atdc_t *a0 = atdc;
        double t = 0.0;

        membuf_printf(plot->buffer, "    <g transform=\"translate(%f %f)\">\n",
                      plot->v[i].x, plot->v[i].y);

        // curve
        while (s0) {
                double t0 = t;
                double t1 = t + a0->accelerate.t;
                print_line(plot,
                           xscale * t0,
                           yscale * a0->accelerate.v0[i],
                           xscale * t1,
                           yscale * a0->accelerate.v1[i],
                           "#00ff00");
                
                t0 = t1;
                t1 += a0->travel.t;
                print_line(plot,
                           xscale * t0,
                           yscale * a0->travel.v0[i],
                           xscale * t1,
                           yscale * a0->travel.v1[i],
                           "#ffff00");
                        
                t0 = t1;
                t1 += a0->decelerate.t;
                print_line(plot,
                           xscale * t0,
                           yscale * a0->decelerate.v0[i],
                           xscale * t1,
                           yscale * a0->decelerate.v1[i],
                           "#ff0000");

                t0 = t1;
                t1 += a0->curve.t;
                print_line(plot,
                           xscale * t0,
                           yscale * a0->curve.v0[i],
                           xscale * t1,
                           yscale * a0->curve.v1[i],
                           "#0000ce");
                
                print_line(plot,
                           xscale * t,
                           yscale * s0->section.v0[i],
                           xscale * t1,
                           yscale * s0->section.v0[i],
                           "#000000");
               
                s0 = s0->next;
                a0 = a0->next;
                t = t1;
        }
        membuf_printf(plot->buffer, "    </g>\n");
}

static void print_path_speeds(plot_t *plot,
                              segment_t *path,
                              atdc_t *atdc,
                              double duration,
                              double *vmax)
{
        for (int i = 0; i < 3; i++)
                print_path_speed_i(plot, path, atdc, i, duration, vmax);
}

static void print_path_acceleration_i(plot_t *plot,
                                      segment_t *path,
                                      atdc_t *atdc,
                                      int i,
                                      double duration,
                                      double *amax)
{
        double xscale = plot->a[i].w / duration;
        double yscale = plot->a[i].h / vmax(amax);
        segment_t *s0 = path;
        atdc_t *a0 = atdc;
        double t = 0.0;

        membuf_printf(plot->buffer, "    <g transform=\"translate(%f %f)\">\n",
                      plot->a[i].x, plot->a[i].y);

        // curve
        while (s0) {
                double t0 = t;
                double t1 = t + a0->accelerate.t;
                print_line(plot,
                           xscale * t0,
                           yscale * a0->accelerate.a[i],
                           xscale * t1,
                           yscale * a0->accelerate.a[i],
                           "#00ff00");
                
                t0 = t1;
                t1 += a0->travel.t;
                /* print_line(plot, */
                /*            xscale * t0, */
                /*            yscale * a0->travel.a[i], */
                /*            xscale * t1, */
                /*            yscale * a0->travel.a[i], */
                /*            "#ffff00"); */
                        
                t0 = t1;
                t1 += a0->decelerate.t;
                print_line(plot,
                           xscale * t0,
                           yscale * a0->decelerate.a[i],
                           xscale * t1,
                           yscale * a0->decelerate.a[i],
                           "#ff0000");

                t0 = t1;
                t1 += a0->curve.t;
                print_line(plot,
                           xscale * t0,
                           yscale * a0->curve.a[i],
                           xscale * t1,
                           yscale * a0->curve.a[i],
                           "#0000ce");
                               
                s0 = s0->next;
                a0 = a0->next;
                t = t1;
        }
        membuf_printf(plot->buffer, "    </g>\n");
}

static void print_path_accelerations(plot_t *plot,
                                     segment_t *path,
                                     atdc_t *atdc,
                                     double duration,
                                     double *amax)
{
        for (int i = 0; i < 3; i++)
                print_path_acceleration_i(plot, path, atdc, i, duration, amax);
}

static void print_slices_speed_i(plot_t *plot, list_t *slices, int i,
                                 double duration, double *vm)
{
        double xscale = plot->v[i].w / duration;
        double yscale = plot->v[i].h / vmax(vm);
        
        membuf_printf(plot->buffer, "    <g transform=\"translate(%f %f)\">\n",
                      plot->v[i].x, plot->v[i].y);

        for (list_t *l = slices; l; l = list_next(l)) {
                section_t *section = list_get(l, section_t);
                print_line(plot,
                           xscale * section->at, yscale * section->v0[i],
                           xscale * (section->at + section->t), yscale * section->v1[i], 
                           "#ff00ff");
        }
        
        membuf_printf(plot->buffer, "    </g>\n");
}

static void print_slices_ij(plot_t *plot, list_t *slices, int i, int j, rect_t *r)
{
        membuf_printf(plot->buffer, "    <g transform=\"translate(%f %f)\">\n",
                      r->x, r->y);

        for (list_t *l = slices; l; l = list_next(l)) {
                section_t *section = list_get(l, section_t);
                print_line(plot,
                           _X(r, section->p0[i]), _Y(r, section->p0[j]),
                           _X(r, section->p1[i]), _Y(r, section->p1[j]),
                           "#ff00ff");
        }
        
        membuf_printf(plot->buffer, "    </g>\n");
}

static void print_slices(plot_t *plot, list_t *slices, double duration, double *vm)
{
        membuf_printf(plot->buffer,
                      "    <g inkscape:groupmode=\"layer\" "
                      "inkscape:label=\"slices\" "
                      "id=\"slices\">\n");

        print_slices_ij(plot, slices, 0, 1, &plot->xy);
        /* print_blocks_ij(plot, blocks, n, 0, 2, 2 * d + L, y0, scale); */
        /* print_blocks_ij(plot, blocks, n, 1, 2, 3 * d + 2 * L, y0, scale); */
        print_slices_speed_i(plot, slices, 0, duration, vm);
        print_slices_speed_i(plot, slices, 1, duration, vm);
        print_slices_speed_i(plot, slices, 2, duration, vm);

        membuf_printf(plot->buffer, "    </g>\n");
}

static void print_blocks_ij(plot_t *plot, block_t *blocks, int n, int i, int j,
                            rect_t *r, double *scale)
{
        membuf_printf(plot->buffer, "    <g transform=\"translate(%f %f)\">\n",
                      r->x, r->y);

        int x = 0;
        int y = 0;

        membuf_printf(plot->buffer, "        <path d=\"");
        print_moveto(plot, 0, 0);
        
        for (int k = 0; k < n; k++) {
                switch (blocks[k].type) {
                case BLOCK_MOVE:
                        print_lineto(plot,
                                     _X(r, (x + blocks[k].data[i+1]) / scale[i]),
                                     _Y(r, (y + blocks[k].data[j+1]) / scale[j]));
                        x += blocks[k].data[1];
                        y += blocks[k].data[2];
                        break;
                default:
                        break;
                }
        }
                
        membuf_printf(plot->buffer, "\" id=\"path\" style=\"fill:none;stroke:%s;"
                      "stroke-width:0.001;stroke-linecap:butt;"
                      "stroke-linejoin:miter;stroke-miterlimit:4;"
                      "stroke-opacity:1;stroke-dasharray:none\" />\n",
                      "#00ffff");
        membuf_printf(plot->buffer, "    </g>\n");
}

static void print_blocks(plot_t *plot, block_t *blocks, int n, double L, double *scale)
{
        membuf_printf(plot->buffer,
                      "    <g inkscape:groupmode=\"layer\" "
                      "inkscape:label=\"blocks\" "
                      "id=\"blocks\">\n");
        
        print_blocks_ij(plot, blocks, n, 0, 1, &plot->xy, scale);
        /* print_blocks_ij(plot, blocks, n, 0, 2, 2 * d + L, y0, scale); */
        /* print_blocks_ij(plot, blocks, n, 1, 2, 3 * d + 2 * L, y0, scale); */

        membuf_printf(plot->buffer, "    </g>\n");
}

static void print_axes(plot_t *plot)
{
        membuf_printf(plot->buffer,
                      "    <g inkscape:groupmode=\"layer\" "
                      "inkscape:label=\"axes\" "
                      "id=\"axes\">\n");
        
        // xy
        print_rect(plot, plot->xy.x, plot->xy.y, plot->xy.w, plot->xy.h);
        print_text(plot, "x", plot->xy.x + plot->xy.w / 2, plot->xy.y - 0.3 * plot->d);
        print_text(plot, "y", plot->xy.x - 0.15 * plot->d, plot->xy.y + plot->xy.h / 2);

        // xz
        print_rect(plot, plot->xz.x, plot->xz.y, plot->xz.w, plot->xz.h);
        print_text(plot, "x", plot->xz.x + plot->xz.w / 2, plot->xz.y - 0.3 * plot->d);
        print_text(plot, "z", plot->xz.x - 0.15 * plot->d, plot->xz.y + plot->xz.h / 2);

        // yz
        print_rect(plot, plot->yz.x, plot->yz.y, plot->yz.w, plot->yz.h);
        print_text(plot, "y", plot->yz.x + plot->yz.w / 2, plot->yz.y - 0.3 * plot->d);
        print_text(plot, "z", plot->yz.x - 0.15 * plot->d, plot->yz.y + plot->yz.h / 2);

        // vx
        print_line(plot,
                   plot->v[0].x, plot->v[0].y,
                   plot->v[0].x + plot->v[0].w, plot->v[0].y,
                   "#cecece");
        
        print_line(plot,
                   plot->v[0].x, plot->v[0].y - plot->v[0].h,
                   plot->v[0].x, plot->v[0].y + plot->v[0].h,
                   "#cecece");

        print_text(plot, "vx", plot->v[0].x - 0.15 * plot->d, plot->v[0].y);
        
        // vy
        print_line(plot,
                   plot->v[1].x, plot->v[1].y,
                   plot->v[1].x + plot->v[1].w, plot->v[1].y,
                   "#cecece");
        
        print_line(plot,
                   plot->v[1].x, plot->v[1].y - plot->v[1].h,
                   plot->v[1].x, plot->v[1].y + plot->v[1].h,
                   "#cecece");
        
        print_text(plot, "vy", plot->v[1].x - 0.15 * plot->d, plot->v[1].y);
        
        // vz
        print_line(plot,
                   plot->v[2].x, plot->v[2].y,
                   plot->v[2].x + plot->v[2].w, plot->v[2].y,
                   "#cecece");
        
        print_line(plot,
                   plot->v[2].x, plot->v[2].y - plot->v[2].h,
                   plot->v[2].x, plot->v[2].y + plot->v[2].h,
                   "#cecece");

        print_text(plot, "vz", plot->v[2].x - 0.15 * plot->d, plot->v[2].y);
        
        // ax
        print_line(plot,
                   plot->a[0].x, plot->a[0].y,
                   plot->a[0].x + plot->a[0].w, plot->a[0].y,
                   "#cecece");
        
        print_line(plot,
                   plot->a[0].x, plot->a[0].y - plot->a[0].h,
                   plot->a[0].x, plot->a[0].y + plot->a[0].h,
                   "#cecece");
        
        print_text(plot, "ax", plot->a[0].x - 0.15 * plot->d, plot->a[0].y);
        
        // ay
        print_line(plot,
                   plot->a[1].x, plot->a[1].y,
                   plot->a[1].x + plot->a[1].w, plot->a[1].y,
                   "#cecece");
        
        print_line(plot,
                   plot->a[1].x, plot->a[1].y - plot->a[1].h,
                   plot->a[1].x, plot->a[1].y + plot->a[1].h,
                   "#cecece");
        
        print_text(plot, "ay", plot->a[1].x - 0.15 * plot->d, plot->a[1].y);
        
        // az
        print_line(plot,
                   plot->a[2].x, plot->a[2].y,
                   plot->a[2].x + plot->a[2].w, plot->a[2].y,
                   "#cecece");
        
        print_line(plot,
                   plot->a[2].x, plot->a[2].y - plot->a[2].h,
                   plot->a[2].x, plot->a[2].y + plot->a[2].h,
                   "#cecece");
        
        print_text(plot, "az", plot->a[2].x - 0.15 * plot->d, plot->a[2].y);
        
        membuf_printf(plot->buffer, "    </g>\n");
}

void print_to_stdout(script_t *script,
                     double *xmax,
                     double *vmax_,
                     double *amax,
                     double *scale)
{
        printf("Segments:\n");
        list_t *l = script->segments;
        while (l != NULL) {
                printf("* \n");
                segment_t *path = list_get(l, segment_t);
                segments_print(path);
                l = list_next(l);
                printf("\n");
        }
        printf("\n\n");


        printf("ATDC:\n");
        list_t *m = script->atdc_list;
        while (m != NULL) {
                printf("*\n");
                atdc_t *atdc = list_get(m, atdc_t);
                atdc_print(atdc);
                m = list_next(m);
                printf("\n");
        }
        printf("\n\n");
        
}

membuf_t *plot_to_mem(script_t *script,
                      double *xmax,
                      double *vmax_,
                      double *amax,
                      double *scale)
{
        membuf_t *buffer = new_membuf();
        if (buffer == NULL)
                return NULL;

        plot_t *plot = new_plot(buffer);
        if (plot == NULL) {
                delete_membuf(buffer);
                return NULL;
        }
        
        plot->d = 0.1;
        plot->L = vmax(xmax);
        
        plot->w = 4 * plot->d + xmax[0] + xmax[0] + xmax[1];
        plot->h = 3 * plot->d + vmax(xmax) + 0.7;
        plot->scale = 1000.0;

        // XY
        plot->xy.x = plot->d;
        plot->xy.y = plot->h - plot->d - plot->L;
        plot->xy.w = xmax[0];
        plot->xy.h = xmax[1];
        if (xmax[0] < 0) {
                plot->xy.x0 = xmax[0];
                plot->xy.x1 = 0.0;
        } else {
                plot->xy.x0 = 0.0;
                plot->xy.x1 = xmax[0];
        }
        if (xmax[1] < 0) {
                plot->xy.y0 = xmax[1];
                plot->xy.y1 = 0.0;
        } else {
                plot->xy.y0 = 0.0;
                plot->xy.y1 = xmax[1];
        }
        
        // XZ
        plot->xz.x = plot->xy.x + plot->xy.w + plot->d;
        plot->xz.y = plot->xy.y;
        plot->xz.w = fabs(xmax[0]);
        plot->xz.h = fabs(xmax[2]);
        if (xmax[0] < 0) {
                plot->xz.x0 = xmax[0];
                plot->xz.x1 = 0.0;
        } else {
                plot->xz.x0 = 0.0;
                plot->xz.x1 = xmax[0];
        }
        if (xmax[2] < 0) {
                plot->xz.y0 = xmax[2];
                plot->xz.y1 = 0.0;
        } else {
                plot->xz.y0 = 0.0;
                plot->xz.y1 = xmax[2];
        }

        // YZ        
        plot->yz.x = plot->xz.x + plot->xz.w + plot->d;
        plot->yz.y = plot->xy.y;
        plot->yz.w = fabs(xmax[1]);
        plot->yz.h = fabs(xmax[2]);
        if (xmax[1] < 0) {
                plot->yz.x0 = xmax[1];
                plot->yz.x1 = 0.0;
        } else {
                plot->yz.x0 = 0.0;
                plot->yz.x1 = xmax[1];
        }
        if (xmax[2] < 0) {
                plot->yz.y0 = xmax[2];
                plot->yz.y1 = 0.0;
        } else {
                plot->yz.y0 = 0.0;
                plot->yz.y1 = xmax[2];
        }
        
        plot->v[0].x = plot->d;
        plot->v[0].y = 7.5 * plot->d;
        plot->v[0].w = plot->w - 2 * plot->d;
        plot->v[0].h = 0.9 * plot->d / 2.0;
        
        plot->v[1].x = plot->d;
        plot->v[1].y = 6.5 * plot->d;
        plot->v[1].w = plot->w - 2 * plot->d;
        plot->v[1].h = 0.9 * plot->d / 2.0;
        
        plot->v[2].x = plot->d;
        plot->v[2].y = 5.5 * plot->d;
        plot->v[2].w = plot->w - 2 * plot->d;
        plot->v[2].h = 0.9 * plot->d / 2.0;
        
        plot->a[0].x = plot->d;
        plot->a[0].y = 3.5 * plot->d;
        plot->a[0].w = plot->w - 2 * plot->d;
        plot->a[0].h = 0.9 * plot->d / 2.0;
        
        plot->a[1].x = plot->d;
        plot->a[1].y = 2.5 * plot->d;
        plot->a[1].w = plot->w - 2 * plot->d;
        plot->a[1].h = 0.9 * plot->d / 2.0;
        
        plot->a[2].x = plot->d;
        plot->a[2].y = 1.5 * plot->d;
        plot->a[2].w = plot->w - 2 * plot->d;
        plot->a[2].h = 0.9 * plot->d / 2.0;

        
        plot_open(plot);

        print_axes(plot);

        double duration = 1.0;

        if (script->atdc_list)
                duration = atdc_list_duration(script->atdc_list);
        else if (script->slices)
                duration = slices_duration(script->slices);

        if (script->segments)
                print_path_list(plot, script->segments);
        
        if (script->atdc_list)
                print_atdc_list(plot, script->atdc_list);

        if (script->slices != NULL)
                print_slices(plot, script->slices, duration, vmax_);

        if (script->block != NULL)
                print_blocks(plot, script->block, script->num_blocks, xmax[0], scale);

        
        list_t *l = script->segments;
        list_t *m = script->atdc_list;
        while (l != NULL && m != NULL) {
                segment_t *path = list_get(l, segment_t);
                atdc_t *atdc = list_get(m, atdc_t);
                print_path_speeds(plot, path, atdc, duration, vmax_);
                print_path_accelerations(plot, path, atdc, duration, amax);
                l = list_next(l);
                m = list_next(m);
        }

        plot_close(plot);
        delete_plot(plot);

        return buffer;
}

int plot_to_file(const char *filepath,
                 script_t *script,
                 double *xmax,
                 double *vmax,
                 double *amax,
                 double *scale)
{
        membuf_t *buffer = plot_to_mem(script, xmax, vmax, amax, scale);
        if (buffer == NULL)
                return -1;
        
        FILE *fp = fopen(filepath, "w");
        if (fp == NULL) {
                r_warn("Failed to open file '%s'", filepath);
                return -1;
        }
        fprintf(fp, "%s", membuf_data(buffer));
        fclose(fp);
        delete_membuf(buffer);
        return 0;
}

