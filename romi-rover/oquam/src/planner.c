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
#include <r.h>
#include <float.h>
#include "v.h"
#include "script_priv.h"

/**************************************************************************/

static inline double max(double a, double b)
{
        return (a > b)? a : b;
}

static inline double min(double a, double b)
{
        return (a < b)? a : b;
}

static inline double sign(double a)
{
        return (a < 0.0)? -1.0 : 1.0;
}

/**************************************************************************/

static void delete_section_list(list_t *list)
{
        for (list_t *l = list; l; l = list_next(l)) {
                section_t *section = list_get(l, section_t);
                delete_section(section);
        }
        delete_list(list);
}

section_t *new_section()
{
        section_t *section = r_new(section_t);
        if (section == NULL)
                return NULL;
        return section;
}

static section_t *new_section_init(double t, double at,
                                   double *p0, double *p1,
                                   double *v0, double *v1,
                                   double *a, int id)
{
        section_t *section = new_section();
        if (section == NULL) {
                return NULL;
        }

        section->t = t;
        section->at = at;
        vcopy(section->p0, p0);
        vcopy(section->p1, p1);
        vcopy(section->v0, v0);
        vcopy(section->v1, v1);
        vcopy(section->a, a);
        vsub(section->d, section->p1, section->p0);
        section->id = id;
        
        return section;
}

void delete_section(section_t *section)
{
        if (section) {
                r_delete(section);
        }
}

static list_t *section_slice(section_t *section, double period, double maxlen, int id)
{
        list_t *slices = NULL;
        double tmp[3];        
        double T = period;
        
        /* The segment has a constant speed. Sample at distances
         * 'maxlen' instead of 'period'. */
        if (norm(section->a) == 0)
                T = maxlen;

        double t = 0.0;

        r_debug("section->t=%f", section->t);
        
        while (t < section->t) {
                double dt = section->t - t;
                if (dt > T)
                        dt = T;
                
                double v0[3];
                smul(v0, section->a, t);
                vadd(v0, section->v0, v0);
                
                double p0[3];
                smul(tmp, section->v0, t);
                smul(p0, section->a, 0.5 * t * t);
                vadd(p0, p0, tmp);
                vadd(p0, p0, section->p0);
                
                double v1[3];
                smul(v1, section->a, t+dt);
                vadd(v1, section->v0, v1);
                
                double p1[3];
                smul(tmp, section->v0, t+dt);
                smul(p1, section->a, 0.5 * (t+dt) * (t+dt));
                vadd(p1, p1, tmp);
                vadd(p1, p1, section->p0);
                
                section_t *s = new_section_init(dt, section->at + t,
                                                p0, p1, v0, v1,
                                                section->a, id);
                if (s == NULL) {
                        r_err("section_slice: new_section_init returned NULL");
                        delete_section_list(slices);
                        return NULL;
                }

                slices = list_append(slices, s);
                if (slices == NULL) {
                        r_err("section_slice: list_append returned NULL");
                        return NULL;
                }
                
                t += dt;
        }
        
        r_err("section_slice: return %p", slices);
        return slices;
}

void section_print(section_t *section, const char *prefix, int id)
{
        printf("%s[%d]: ", prefix, id);
        printf("p0(%0.3f,%0.3f,%0.3f)-p1(%0.3f,%0.3f,%0.3f); ",
               section->p0[0], section->p0[1], section->p0[2],
               section->p1[0], section->p1[1], section->p1[2]);
        printf("v0(%0.3f,%0.3f,%0.3f)-v1(%0.3f,%0.3f,%0.3f); ",
               section->v0[0], section->v0[1], section->v0[2],
               section->v1[0], section->v1[1], section->v1[2]);
        printf("a(%0.3f,%0.3f,%0.3f)",
               section->a[0], section->a[1], section->a[2]);
        printf("\n");
}

/**************************************************************************/

segment_t *new_segment()
{
        segment_t *segment = r_new(segment_t);
        if (segment == NULL)
                return NULL;
        return segment;
}

void delete_segment(segment_t *segment)
{
        if (segment) {
                r_delete(segment);
        }
}

void segments_print(segment_t *segment)
{
        segment_t *s = segment;
        while (s != NULL) {
                section_print(&s->section, "S", s->section.id);
                s = s->next;
        }
}

/**************************************************************************/

atdc_t *new_atdc()
{
        atdc_t *atdc = r_new(atdc_t);
        if (atdc == NULL)
                return NULL;
        return atdc;
}

void delete_atdc(atdc_t *atdc)
{
        if (atdc) {
                r_delete(atdc);
        }
}

static list_t *atdc_slice(atdc_t *atdc, double period, double maxlen)
{
        list_t *list = NULL;

        /* r_err("atdc_slice: a=%f, t=%f, d=%f, c=%f", */
        /*       atdc->accelerate.t, atdc->travel.t, */
        /*       atdc->decelerate.t, atdc->curve.t); */

        if (atdc->accelerate.t > 0) {
                list_t *slices = section_slice(&atdc->accelerate, period, maxlen, atdc->id);
                if (slices == NULL) {
                        r_err("atdc_slice(a): section_slice returned NULL");
                        delete_section_list(list);
                        return NULL;
                }
                list = list_concat(list, slices);
        }
        
        if (atdc->travel.t > 0) {
                list_t *slices = section_slice(&atdc->travel, period, maxlen, atdc->id);
                if (slices == NULL) {
                        r_err("atdc_slice(t): section_slice returned NULL");
                        delete_section_list(list);
                        return NULL;
                }
                list = list_concat(list, slices);
        }
        
        if (atdc->decelerate.t > 0) {
                list_t *slices = section_slice(&atdc->decelerate, period, maxlen, atdc->id);
                if (slices == NULL) {
                        r_err("atdc_slice(d): section_slice returned NULL");
                        delete_section_list(list);
                        return NULL;
                }
                list = list_concat(list, slices);
        }
        
        if (atdc->curve.t > 0) {
                list_t *slices = section_slice(&atdc->curve, period, maxlen, atdc->id);
                if (slices == NULL) {
                        r_err("atdc_slice(c): section_slice returned NULL");
                        delete_section_list(list);
                        return NULL;
                }
                list = list_concat(list, slices);
        }

        //r_err("atdc_slice: return %p", list);
        return list;
}

static list_t *atdc_list_slice(atdc_t *atdc_list, double period, double maxlen)
{
        list_t *list = NULL;
        
        for (atdc_t *atdc = atdc_list; atdc != NULL; atdc = atdc->next) {
                list_t *slices = atdc_slice(atdc, period, maxlen);
                if (slices == NULL) {
                        r_err("atdc_list_slice: atdc_slice returned NULL");
                        delete_section_list(list);
                        return NULL;
                }
                list = list_concat(list, slices);
        }
        
        return list;
}

void segment_print(segment_t *segment)
{
        segment_t *s = segment;
        while (s != NULL) {
                section_print(&s->section, "S", s->section.id);
                printf("\n");
                s = s->next;
        }
}

void atdc_print(atdc_t *atdc)
{
        atdc_t *s = atdc;
        while (s != NULL) {
                section_print(&s->accelerate, "A", s->id);
                section_print(&s->travel, "T", s->id);
                section_print(&s->decelerate, "D", s->id);
                section_print(&s->curve, "C", s->id);
                s = s->next;
                if (s)
                        printf("-\n");
        }
}

/**************************************************************************/

static void segment_compute_curve(segment_t *s0, atdc_t *t0, double d, double *amax)
{
        segment_t *s1 = s0->next;
        atdc_t *t1 = t0->next;

        if (s1 == NULL) {
                t0->curve.t = 0.0;
                vzero(t0->curve.v0);
                vzero(t0->curve.v1);
                vcopy(t0->curve.p0, s0->section.p1);
                vcopy(t0->curve.p1, s0->section.p1);
                vzero(t0->curve.a);
                vzero(t0->curve.d);
                return;
        }

        // Set the entry and exit speed of the curve to smallest speed
        // before and after the junction.
        double w = min(norm(s0->section.v0), norm(s1->section.v0));

        // The speed - and direction - before and after the curve.
        double w0[3];
        double w1[3];
        smul(w0, normalize(w0, s0->section.d), w);
        smul(w1, normalize(w1, s1->section.d), w);
        
        /*

          We have to compute the maximum speed at the entry of the
          curve such that the deviation at the junction is less than
          the maximum allowable deviation d. The maximim acceleration
          that can be applied is 'amax'.

          We also have to compute the points at which the controller
          has to start and stop accelerating. We'll call these points
          q0(x0,y0) and q1(x1, y1).

          In the discussion below, we change the coordinate system. We
          place the curve in the plane that goes through _w0 and _w1
          (I'm using underscore to denote vectors). The y-axis is
          positioned half-way between the _w0 and _w1 vectors.

          We have:
          _wx = 1/2 (_w0 + _w1), _ex = _vx / |_vx| 
          _wy = 1/2 (_w0 - _w1), _ey = _vy / |_vy|

                  y
                  |          
           w0 \   |   / w1       
               \  |  /        
                \ | /         
                 \|/         
           -------o--->--- x
                  |   wx
                  |   
                  |   
                  v wy

           The origin is placed in the junction point. In this frame
           of reference, we have for _w0(wx0, wy0, wz0) and _w1(wx1,
           wy1, wz1) that wx0=wx1, wy0=-wy1, and wz0=wz1=0.

        */
        
        // The x and y unit vectors in the new coordinate space (ex,
        // ey). The wx0 and wy0 are the speeds in these directions.
        double ex[3] = { 0.0, 0.0, 0.0 };
        double ey[3] = { 0.0, 0.0, 0.0 };
        double wx0, wy0;

        double c[3];
        vcross(c, w0, w1);

        if (norm(c) < 0.001) {
                // The two speed vectors are collinear...
                double n;
                
                if (w1[0] != 0.0)
                        n = w0[0] / w1[0];
                else if (w1[1] != 0.0)
                        n = w0[1] / w1[1];
                else if (w1[2] != 0.0)
                        n = w0[2] / w1[2];
                
                if (n > 0.0) {
                        // pointing in the same direction.
                        wx0 = norm(w0);
                        wy0 = 0.0;
                        smul(ex, w0, 1/wx0);
                        ey[0] = -ex[1];
                        ey[1] = ex[0];
                        ey[2] = ex[2];

                } else {
                        // pointing in the opposite directions.
                        wx0 = 0.0;
                        wy0 = norm(w0);
                        smul(ey, w0, -1/wy0);
                        ex[0] = ey[1];
                        ex[1] = -ey[0];
                        ex[2] = ey[2];
                }
                
        } else {
                vadd(ex, w1, w0);
                smul(ex, ex, 0.5);
                wx0 = norm(ex);
                smul(ex, ex, 1/wx0);

                vsub(ey, w0, w1);
                smul(ey, ey, 0.5);        
                wy0 = norm(ey);
                smul(ey, ey, -1 / wy0);
        }

        /* Compute the maximum allowed acceleration in the direction
           of wy (or ey). */
        double a[3];
        double s = DBL_MAX;
        for (int i = 0; i < 3; i++) {
                if (ey[i] != 0.0)
                        s = min(s, amax[i] / fabs(ey[i]));
        }
        vcopy(a, ey);
        smul(a, a, s);

        double am = norm(a);

        /*
          The acceleration is applied only along the y-axis, a =
          ay. The absolute speed at the entry of the curve is the same
          as the absolute speed at the exit, |_w0|=|_w1|. The speed
          along the x-axis remain the same, wx0 = vx1, and the speed
          along the y-axis inverses, wy1 = -wy0. wy0 and ay have
          opposite signs.

          We have:
          wx = wx0 = wx1 = |_wx| 
          wy = wy0 = -wy1 = |_wy|

          ∆wy = wy1 - wy0 = -|_w0 - _w1| 
           
          The equation for the speed is (wy0 and a have opposite
          signs):
          wy1 = wy0 + a·∆t => ∆t = -2wy0/a                      (1)
          
          The equation for the y position is:
          y = y0 + wy0·t + a.t²/2

          When t = ∆t/2, y reaches its minimim ym  
          => ym = y0 + wy0·∆t/2 + a.(∆t/2)²/2, using (1) and develop
          => ym = y0 - wy0²/2a                                    (2)
          
          The time it takes to follow the speed curve is the same as
          the time it takes to follow the two segments of the original
          path that go through the junction point. This follows from
          the fact the the speed along the x-axis remains constant in
          both cases. Following the orginal straight path for ∆t/2,
          the junction at 0 is reached:
          
          y0 + wy0·∆t/2 = 0 => y0 = wy0²/a                        (3)
          
          (2) and (3) combined gives: ym = wy0²/2a
          
          The error ym should be smaller than the maximim deviation d.
          ym < d => wy0²/2a < d => wy0 < sqrt(2ad)               
          
          If the requested speed is larger than the maximum, all
          speed components will be scaled linearly.
          
          We already calculated the y coordinate of the entry and exit
          points in (3). The x coordinates of the entry and exit
          points are:
          
          ∆x = wx0·∆t => ∆x = -2wy0.wx0/a,            using (1)           
          => x0 = -∆x/2 = wy0.wx0/a and x1 = ∆x/2 = -wy0·wx0/a

          To obtain the acceleration to apply on the stepper motors,
          we have to rotate the acceleration (0,ay,0) back into the
          coordinate space of the CNC. 

         */
        
        // Check for maximum velocity wymax at the entry point, given
        // the maximum acceleration and deviation. If the current
        // speed is higher than wymax then scale the entry velocity
        // proportionally.
        double wymax = sqrt(2.0 * am * d);
        double vscale = 1.0;
        
        if (fabs(wy0) > wymax)
                vscale = wymax / wy0;

        wx0 *= vscale;
        wy0 *= vscale;
        smul(w0, w0, vscale);
        smul(w1, w1, vscale);
        
        // Compute the entry and exit coordinates 
        double x0 = wy0 * wx0 / am;
        double y0 = wy0 * wy0 / am;

        // The distance between the entry/exit points and the
        // junction.
        double lq = sqrt(x0 * x0 + y0 * y0);

        // If the entry or exit point is more than halfway along the
        // segment then we should slow down.
        double len0 = norm(s0->section.d);
        double len1 = norm(s1->section.d);
        double len = min(len0, len1);
        
        if (lq > len / 2.0) {
                vscale = len / (2.0 * lq);
                double sqvs = sqrt(vscale);
                x0 *= vscale;
                y0 *= vscale;
                wx0 *= sqvs;
                wy0 *= sqvs;
                smul(w0, w0, sqvs);
                smul(w1, w1, sqvs);
                lq = len / 2.0;
        }

        vcopy(t0->curve.v0, w0);                                 // v0
        vcopy(t0->curve.v1, w1);                                 // v1

        // FYI, The deviation is:
        //s0->deviation = 0.5 * wy0 * wy0 / am; 
        
        // Compute a vector along d0 with length lq
        double p0[3];
        normalize(p0, s0->section.d);
        smul(p0, p0, lq);

        /* The absolute position of the entry point of the speed
         * curve. */
        vsub(p0, s0->section.p1, p0);
        vcopy(t0->curve.p0, p0);                                 // p0

        
        // Compute a vector along d1 with length lq
        double p1[3];
        normalize(p1, s1->section.d);
        smul(p1, p1, lq);

        /* The absolute position of the exit point of the speed
         * curve */
        vadd(p1, s0->section.p1, p1);
        vcopy(t0->curve.p1, p1);                                 // p1

        /* Copy the next segment's entry speed and entry point to its
         * accelerate section */
        vcopy(t1->accelerate.v0, t0->curve.v1);
        vcopy(t1->accelerate.p0, p1);
        
        /* The acceleration, in the coordinate space of the CNC
         * machine. */
        vcopy(t0->curve.a, a);                                   // a
        
        /* The time it takes to follow the speed curve. */
        t0->curve.t = fabs(2.0 * wy0 / am);                      // t
}

static int section_is_valid(section_t *s,
                            const char *name,
                            double tmax,
                            double *xmin, 
                            double *xmax, 
                            double *vmax,
                            double *amax)
{
        if (s->t > tmax) {
                r_warn("section_is_valid (%s): t too long", name);
                return 0;
        }
        if (s->t < 0.0) {
                r_warn("section_is_valid (%s): t < 0", name);
                return 0;
        }
        if (s->at < 0.0) {
                r_warn("section_is_valid (%s): at < 0", name);
                return 0;
        }
        
        double dx[3];
        double dmax;

        vsub(dx, xmax, xmin);
        dmax = norm(dx);
        
        for (int i = 0; i < 3; i++) {
                if (isnan(s->p0[i])) {
                        r_warn("section_is_valid (%s): p0[%d] is NaN", name, i);
                        return 0;
                }
                if (isnan(s->p1[i])) {
                        r_warn("section_is_valid (%s): p1[%d] is NaN", name, i);
                        return 0;
                }
                if (isnan(s->v0[i])) {
                        r_warn("section_is_valid (%s): v0[%d] is NaN", name, i);
                        return 0;
                }
                if (isnan(s->v1[i])) {
                        r_warn("section_is_valid (%s): v1[%d] is NaN", name, i);
                        return 0;
                }
                if (isnan(s->a[i])) {
                        r_warn("section_is_valid (%s): a[%d] is NaN", name, i);
                        return 0;
                }
                if (isnan(s->d[i])) {
                        r_warn("section_is_valid (%s): d[%d] is NaN", name, i);
                        return 0;
                }
                
                if (s->p0[i] < xmin[i] - 0.001) {
                        r_warn("section_is_valid (%s): p0[%d] is too small: %.12f < %.12f",
                               name, i, s->p0[i], xmin[i]);
                        return 0;
                }
                if (s->p0[i] > xmax[i] + 0.001) {
                        r_warn("section_is_valid (%s): p0[%d] is too large: %.12f > %.12f",
                               name, i, s->p0[i], xmax[i]);
                        return 0;
                }
                if (s->p1[i] < xmin[i] - 0.001) {
                        r_warn("section_is_valid (%s): p1[%d] is too small: %.12f < %.12f",
                               name, i, s->p1[i], xmin[i]);
                        return 0;
                }
                if (s->p1[i] > xmax[i] + 0.001) {
                        r_warn("section_is_valid (%s): p1[%d] is too large: %.12f > %.12f",
                               name, i, s->p1[i], xmax[i]);
                        return 0;
                }
                if (fabs(s->v0[i]) > vmax[i] + 0.01) {
                        r_warn("section_is_valid (%s): v0[%d] is too large: %.12f > %.12f",
                               name, i, fabs(s->v0[i]), vmax[i]);
                        return 0;
                }
                if (fabs(s->v1[i]) > vmax[i] + 0.01) {
                        r_warn("section_is_valid (%s): v1[%d] is too large: %.12f > %.12f",
                               name, i, fabs(s->v1[i]), vmax[i]);
                        return 0;
                }
                if (fabs(s->a[i]) > amax[i] + 0.001) {
                        r_warn("section_is_valid (%s): a[%d] is too large: %.12f > %.12f",
                               name, i, fabs(s->a[i]), amax[i]);
                        return 0;
                }
                if (fabs(s->d[i]) > dmax + 0.01) {
                        r_warn("section_is_valid (%s): d[%d] is too large: %.12f > %.12f",
                               name, i, fabs(s->d[i]), dmax);
                        return 0;
                }
        }


        double v = norm(vmax); 
        double a = norm(amax);
        if (norm(s->v0) + 0.0001 > v) {
                r_warn("section_is_valid (%s): v0 is too large");
                return 0;
        }
        if (norm(s->v1) + 0.0001 > v) {
                r_warn("section_is_valid (%s): v1 is too large");
                return 0;
        }
        if (norm(s->a) + 0.0001 > a) {
                r_warn("section_is_valid (%s): a is too large");
                return 0;
        }
        
        return 1;
}

static int atdc_is_valid(atdc_t *t,
                         double tmax,
                         double *xmin, 
                         double *xmax, 
                         double *vmax,
                         double *amax)
{
        if (t->accelerate.t == 0.0
            && t->travel.t == 0.0
            && t->decelerate.t == 0.0
            && t->curve.t == 0.0) {
                r_warn("segment_is_valid: empty segment");
                return 0;
        }

        if (!section_is_valid(&t->accelerate, "accelerate", tmax, xmin, xmax, vmax, amax)
            || !section_is_valid(&t->travel, "travel", tmax, xmin, xmax, vmax, amax)
            || !section_is_valid(&t->decelerate, "decelerate", tmax, xmin, xmax, vmax, amax)
            || !section_is_valid(&t->curve, "curve", tmax, xmin, xmax, vmax, amax)) {
                return 0;
        }

        return 1;
}

static void segment_compute_accelerations(atdc_t *t0, double *v,
                                          double *amax, double at)
{
        double dv[3];
        double dt[3];
        double dx[3];
        double a[3];
        double tmp[3];
        
        vsub(dx, t0->curve.p0, t0->accelerate.p0);
        double len = norm(dx);

        if (len == 0.0) {

                r_debug("segment_compute_accelerations: len =0");
                
                t0->accelerate.t = 0.0;                      // t
                vzero(t0->accelerate.a);                     // a
                vzero(t0->accelerate.d);                     // d
                vcopy(t0->accelerate.p1, t0->curve.p0);      // p1
                vcopy(t0->accelerate.v1, t0->curve.v0);      // v1
                
                t0->travel.t = 0;                            // t
                vzero(t0->travel.a);                         // a
                vzero(t0->travel.d);                         // d
                vcopy(t0->travel.p0, t0->curve.p0);          // p1
                vcopy(t0->travel.p1, t0->curve.p0);          // p1
                vcopy(t0->travel.v0, t0->curve.v0);          // v0
                vcopy(t0->travel.v1, t0->curve.v0);          // v1
                
                t0->decelerate.t = 0;                        // t
                vzero(t0->decelerate.a);                     // a
                vzero(t0->decelerate.d);                     // d
                vcopy(t0->decelerate.p0, t0->curve.p0);      // p0
                vcopy(t0->decelerate.p1, t0->curve.p0);      // p1
                vcopy(t0->decelerate.v0, t0->curve.v0);      // v0
                vcopy(t0->decelerate.v1, t0->curve.v0);      // v1
                
        } else {
        
                /* Accelerate */
        
                // p0 and v0 are already set
                vsub(dv, v, t0->accelerate.v0);
                vdiv(dt, dv, amax);
                vabs(dt, dt);
                t0->accelerate.t = vmax(dt);                             // t
                if (t0->accelerate.t == 0) {
                        vzero(t0->accelerate.a);                         // a
                        vcopy(t0->accelerate.v1, t0->accelerate.v0);     // v1
                        vzero(t0->accelerate.d);                         // d
                        vcopy(t0->accelerate.p1, t0->accelerate.p0);     // p1
                } else {
                        sdiv(t0->accelerate.a, dv, t0->accelerate.t);    // a
                        vcopy(t0->accelerate.v1, v);                     // v1
                        smul(dx, t0->accelerate.v0, t0->accelerate.t);
                        smul(tmp, t0->accelerate.a, 0.5 * t0->accelerate.t * t0->accelerate.t);
                        vadd(dx, dx, tmp);
                        vcopy(t0->accelerate.d, dx);                     // d
                        vadd(t0->accelerate.p1, t0->accelerate.p0, dx);  // p1
                }
                vsub(dx, t0->accelerate.p1, t0->accelerate.p0);
                double len_a = norm(dx);

                /* Decelerate */
        
                vcopy(t0->decelerate.v0, v);                             // v0
                vcopy(t0->decelerate.p1, t0->curve.p0);                  // p1
                vcopy(t0->decelerate.v1, t0->curve.v0);                  // v1
                vsub(dv, t0->decelerate.v0, t0->decelerate.v1);
                vdiv(dt, dv, amax);
                vabs(dt, dt);
                t0->decelerate.t = vmax(dt);                             // t
        
                if (t0->decelerate.t == 0.0) {
                        vzero(t0->decelerate.a);                         // a
                        vzero(t0->decelerate.d);                         // d
                        vcopy(t0->decelerate.p0, t0->decelerate.p1);     // p0
                } else {
                        sdiv(t0->decelerate.a, dv, -t0->decelerate.t);   // a
                        smul(dx, t0->decelerate.v0, t0->decelerate.t);
                        smul(tmp, t0->decelerate.a, 0.5 * t0->decelerate.t * t0->decelerate.t);
                        vadd(dx, dx, tmp);
                        vcopy(t0->decelerate.d, dx);                     // d
                        vsub(t0->decelerate.p0, t0->decelerate.p1, dx);  // p0
                }
                vsub(dx, t0->decelerate.p1, t0->decelerate.p0);
                double len_d = norm(dx);

                /* Travel at constant speed for the remaining lenght */
                if (len_a + len_d < len) {

                        r_debug("segment_compute_accelerations: "
                                "accelerate/travel/decelerate");
                                        
                        double len_t = len - len_a - len_d;
                        double vn = norm(v);
                        t0->travel.t = len_t / vn;                         // t
                        vcopy(t0->travel.p0, t0->accelerate.p1);           // p0
                        vcopy(t0->travel.p1, t0->decelerate.p0);           // p1
                        vcopy(t0->travel.v0, v);                           // v0
                        vcopy(t0->travel.v1, v);                           // v1
                        vzero(t0->travel.a);                               // a
                        vsub(t0->travel.d, t0->travel.p1, t0->travel.p0);  // d
                
                } else {

                        /* There isn't enough space to accelerate to the
                         * maximum speed and decelerate again. In this case,
                         * we use a simpler strategy: instead of the
                         * accelerate-travel-decelarate combo, we apply a
                         * constant acceleration to change the speed from the
                         * start and end value. */

                        r_debug("** constant acceleration from start to end **");

//                if (t0->id == 36) {
//                        double dummy = fabs(0.0);
//                }
                
                        vsub(dv, t0->curve.v0, t0->accelerate.v0);
                        
                        if (norm(dv) > 0.0) {

                                r_debug("segment_compute_accelerations: "
                                        "constant acceleration, no travel");
                                
                                vdiv(dt, dv, amax);
                                vabs(dt, dt);
                                double t = vmax(dt);
                
                                sdiv(a, dv, t);

                                t0->accelerate.t = t;                        // t
                                vcopy(t0->accelerate.a, a);                  // a
                                vcopy(t0->accelerate.p1, t0->curve.p0);      // p1
                                vsub(t0->accelerate.d, t0->accelerate.p1, t0->accelerate.p0); // d
                                vcopy(t0->accelerate.v1, t0->curve.v0);      // v1
                                                
                                t0->travel.t = 0;                            // t
                                vzero(t0->travel.a);                         // a
                                vzero(t0->travel.d);                         // d
                                vcopy(t0->travel.p0, t0->accelerate.p1);     // p0
                                vcopy(t0->travel.p1, t0->accelerate.p1);     // p1
                                vcopy(t0->travel.v0, t0->curve.v0);          // v0
                                vcopy(t0->travel.v1, t0->curve.v0);          // v1
                        
                                t0->decelerate.t = 0;                        // t
                                vzero(t0->decelerate.a);                     // a
                                vzero(t0->decelerate.d);                     // d
                                vcopy(t0->decelerate.p0, t0->accelerate.p1); // p0
                                vcopy(t0->decelerate.p1, t0->accelerate.p1); // p1
                                vcopy(t0->decelerate.v0, t0->curve.v0);      // v0
                                vcopy(t0->decelerate.v1, t0->curve.v0);      // v1
                        
                        } else {
                                // No acceleration / deceleration is
                                // required because same speed at the
                                // start and end. Travel at constant
                                // speed.

                                r_debug("segment_compute_accelerations: "
                                        "constant travel, no acceleration/deceleration");
                                
                                t0->accelerate.t = 0.0;                      // t
                                vzero(t0->accelerate.a);                     // a
                                vcopy(t0->accelerate.p1, t0->accelerate.p0); // p1
                                vsub(t0->accelerate.d, t0->accelerate.p1, t0->accelerate.p0); // d
                                vcopy(t0->accelerate.v1, t0->curve.v0);      // v1
                                                
                                t0->travel.t = 0;                            // t
                                vzero(t0->travel.a);                         // a
                                vcopy(t0->travel.p0, t0->accelerate.p1);     // p1
                                vcopy(t0->travel.p1, t0->curve.p0);          // p1
                                vsub(t0->travel.d, t0->travel.p1, t0->travel.p0); // d
                                vcopy(t0->travel.v0, t0->curve.v0);          // v0
                                vcopy(t0->travel.v1, t0->curve.v0);          // v1
                        
                                t0->decelerate.t = 0;                        // t
                                vzero(t0->decelerate.a);                     // a
                                vzero(t0->decelerate.d);                     // d
                                vcopy(t0->decelerate.p0, t0->accelerate.p1); // p0
                                vcopy(t0->decelerate.p1, t0->accelerate.p1); // p1
                                vcopy(t0->decelerate.v0, t0->curve.v0);      // v0
                                vcopy(t0->decelerate.v1, t0->curve.v0);      // v1
                        }
                }
        }

        
        
        t0->accelerate.at = at;             
        t0->travel.at = t0->accelerate.at + t0->accelerate.t;
        t0->decelerate.at = t0->travel.at + t0->travel.t;
        t0->curve.at = t0->decelerate.at + t0->decelerate.t;
}

/**
 *  Readjusts the speed in case the next segment had to slow down.
 *
 */
static void segment_update_speeds(segment_t *s0, atdc_t *t0, double *amax)
{
        /* Compare the speeds at the start of the segments and at the
           entry of the curve. Check whether there is enough space to
           accelerate/decelerate. If not, adapt one of the two
           speeds. */

        /* The available path length */
        double len;
        double d[3];
        vsub(d, t0->curve.p0, t0->accelerate.p0);
        len = norm(d);

        /* The two speeds */
        double v0 = norm(t0->accelerate.v0);
        double v1 = norm(t0->curve.v1);
        double v0_next = 0.0;
        if (t0->next)
                v0_next = norm(t0->next->accelerate.v0);

        // The next segment lowered its entry speed. Slow down the
        // curve.
        if (v1 != v0_next) {
                double alpha = v0_next / v1;
                smul(t0->curve.v0, t0->curve.v0, alpha);
                smul(t0->curve.v1, t0->curve.v1, alpha);
                smul(t0->curve.a, t0->curve.a, alpha * alpha);
                t0->curve.t /= alpha;
        }
        
        /* The maximum allowable acceleration in the direction of this
           segment */
        double tmp[3];
        normalize(tmp, s0->section.d);
        vmul(tmp, tmp, amax);
        double am = norm(tmp);

        /* The path length needed to change the speed */
        double dv = v1 - v0;
        double dt = sign(dv) * dv / am;
        double dx = v0 * dt + sign(dv) * 0.5 * am * dt * dt;

        if (dx > len && v0 > v1) {
                
                /* The speed at the entry of this segment is too high
                   and the segment is not long enough to slow down to
                   the maximum entry speed of the curve at the end of
                   the segment. We must scale back the speed at the
                   start of this segment. */

                /* This change will have to be propagated back to the
                   previous segments. This will be done during the
                   backward traversal of the segments. */
                
                dt = (sqrt(v1 * v1 + 2.0 * am * len) - v1) / am;
                double v0s = v1 + am * dt;
                v0s = sqrt(2 * am * len + v1 * v1);
                double alpha = v0s / v0;
                smul(t0->accelerate.v0, t0->accelerate.v0, alpha);

                // Propagate the speed change backwards through the path.
                if (s0->prev == NULL || t0->prev == NULL) {
                        r_err("check_curve_speed_forward: pushing speed change "
                              "backwards but we're at the first segment?!");
                        return;
                }
                
                segment_update_speeds(s0->prev, t0->prev, amax);
                
        } else if (dx > len && v0 < v1) {
                
                /* The segment is too short to accelerate up to the
                   desired entry speed of the curve. Reduce the
                   curve's entry speed and recalculate the force to
                   apply.  TODO: ideally, the maximum acceleration is
                   maintained during the curve but the entry and exit
                   points are remapped. For simplicity, I started by
                   changing the acceleration and speeds. */
                
                dt = (sqrt(v0 * v0 + 2.0 * am * len) - v0) / am;
                double v1s = v0 + am * dt;
                double alpha = v1s / v1;
                smul(t0->curve.v0, t0->curve.v0, alpha);
                smul(t0->curve.v1, t0->curve.v1, alpha);
                smul(t0->curve.a, t0->curve.a, alpha * alpha);
                t0->curve.t /= alpha;
        }
}

/**
 *  Computes the curve at the end of the segment. This function may
 *  adjust the entry speed of the segment to assure that the curve
 *  isn't entered with too high a speed.
 *
 *  s0: The current segment.
 *  amax: The maximum acceleration that the machine tolerates. 
 */
static void segment_compute_curve_and_speeds(segment_t *s0, atdc_t *t0,
                                             double d, double *amax)
{
        segment_compute_curve(s0, t0, d, amax);
        segment_update_speeds(s0, t0, amax);
}

static void segment_check_max_speed(segment_t *s0, double *vmax)
{
        double s = 1.0;

        for (int i = 0; i < 3; i++) {
                if (s0->section.v0[i] != 0.0)
                        s = min(s, vmax[i] / fabs(s0->section.v0[i]));
        }
        
        smul(s0->section.v0, s0->section.v0, s);
        smul(s0->section.v1, s0->section.v1, s);
}

/**************************************************************************/

static void compute_curves_and_speeds(segment_t *path, atdc_t *atdc,
                                      double d, double *amax)
{
        segment_t *s0 = path;
        atdc_t *t0 = atdc;
        
        while (s0) {
                segment_compute_curve_and_speeds(s0, t0, d, amax);
                s0 = s0->next;
                t0 = t0->next;
        }
}

static void compute_accelerations(segment_t *path,
                                  atdc_t *atdc,
                                  int path_count,
                                  double tmax,
                                  double *xmin, 
                                  double *xmax, 
                                  double *vmax,
                                  double *amax)
{
        segment_t *s0 = path;
        atdc_t *t0 = atdc;
        double at = 0.0;
        int atdc_count = 0;
        
        while (s0) {
                r_debug("compute_accelerations: ==== path %d, atdc %d ===================",
                       path_count, atdc_count);

                segment_compute_accelerations(t0, s0->section.v0, amax, at);

                if (!atdc_is_valid(t0, tmax, xmin, xmax, vmax, amax)) {
                        r_warn("compute_accelerations: invalid atdc: "
                               "path %d, atdc %d", path_count, atdc_count);
                }

                
                at = t0->curve.at + t0->curve.t;
                s0 = s0->next;
                t0 = t0->next;
                atdc_count++;
        }

        r_debug("compute_accelerations: ========================================");
}

static void check_max_speeds(segment_t *path, double *vmax)
{
        for (segment_t *segment = path; segment != NULL; segment = segment->next)
                segment_check_max_speed(segment, vmax);
}

static atdc_t *copy_segments_to_atdc(segment_t *path)
{
        atdc_t *first = NULL;
        atdc_t *prev = NULL;
        
        for (segment_t *segment = path; segment != NULL; segment = segment->next) {
                atdc_t *atdc = new_atdc();
                
                // Append to double-linked list
                if (first == NULL)
                        first = atdc;
                if (prev != NULL) {
                        prev->next = atdc;
                        atdc->prev = prev;
                }
                prev = atdc;
                
                atdc->id = segment->section.id;                
        }

        return first;
}

static atdc_t *segments_to_atdc(segment_t *path,
                                int path_count,
                                double d,                                   
                                double tmax,
                                double *xmin, 
                                double *xmax, 
                                double *vmax,
                                double *amax)
{
        check_max_speeds(path, vmax);
        atdc_t *atdc = copy_segments_to_atdc(path);

        compute_curves_and_speeds(path, atdc, d, amax);
                
        /* printf("Curves and speed:\n"); */
        /* atdc_print(atdc); */
        /* printf("\n\n"); */
        
        compute_accelerations(path, atdc, path_count, tmax, xmin, xmax, vmax, amax);

        /* printf("Accelerations:\n"); */
        /* atdc_print(atdc); */
        /* printf("\n\n"); */

        return atdc;
}

static list_t *segment_list_to_atdc_list(list_t *paths,
                                         double d,
                                         double tmax,
                                         double *xmin, 
                                         double *xmax, 
                                         double *vmax,
                                         double *amax)
{
        list_t *atdc_list = NULL;
        int path_count = 0;
        
        for (list_t *l = paths; l != NULL; l = list_next(l)) {
                segment_t *path = list_get(l, segment_t);
                atdc_t *atdc = segments_to_atdc(path, path_count, d, tmax,
                                                xmin, xmax, vmax, amax);
                atdc_list = list_append(atdc_list, atdc);
                path_count++;
        }
        return atdc_list;
}

static list_t *script_to_segment_list(script_t *script, double *pos)
{
        list_t *actions = script->actions;
        list_t *paths = NULL;
        segment_t *prev = NULL;

        while (actions) {
                
                action_t *action = list_get(actions, action_t);
                
                if (action->type == ACTION_MOVE) {

                        /* If the displacement is less then 0.1 mm, skip it. */
                        double d[3];
                        vsub(d, action->data.move.p, pos); 
                        if (norm(d) > 0.0001) {
                                segment_t *segment = new_segment();

                                // chain the segment
                                segment->prev = prev;
                                if (prev) 
                                        prev->next = segment;
                                else 
                                        paths = list_append(paths, segment);
                        
                                prev = segment;

                                segment->section.id = action->data.move.id; 

                                // p0 and p1
                                vcopy(segment->section.p0, pos); 
                                vcopy(segment->section.p1, action->data.move.p); 

                                // d
                                vsub(segment->section.d,
                                     segment->section.p1,
                                     segment->section.p0); 
                        
                                // v0 and v1
                                normalize(segment->section.v0, segment->section.d);
                                smul(segment->section.v0,
                                     segment->section.v0,
                                     action->data.move.v);
                                vcopy(segment->section.v1, segment->section.v0);
                        
                                // a
                                vzero(segment->section.a);

                                // update current position
                                vcopy(pos, action->data.move.p); 
                        }                        
                }
                
                actions = list_next(actions);
        }
        
        return paths;
}

/******************************************************************/

int planner_convert_script(script_t *script, double *position,
                           double *xmin, double *xmax,
                           double *vmax, double *amax,
                           double deviation)
{
//        int err = 0;
        double pos[3];
        
        /* if (controller_position(controller, pos) != 0) */
        /*         return -1; */

        vcopy(pos, position);
        
        script->segments = script_to_segment_list(script, pos);
        if (script->segments == NULL)
                return -1;
        
        script->atdc_list = segment_list_to_atdc_list(script->segments,
                                                      deviation, 120.0,
                                                      xmin, xmax, vmax, amax);

        if (script->atdc_list == NULL)
                return -1;


        if (0) {
                int path_count = 0;
                list_t *atdc_list = script->atdc_list;
                for (list_t *l = atdc_list; l != NULL; l = list_next(l), path_count++) {
                        atdc_t *atdc = list_get(l, atdc_t);
                        int segment_count = 0;
                        for (atdc_t *t0 = atdc; t0; t0 = t0->next, segment_count++) {
                                if (!atdc_is_valid(t0, 120.0, xmin, xmax, vmax, amax)) {
                                        r_warn("planner_convert_script: invalid atdc: "
                                               "path %d, atdc %d",
                                               path_count,
                                               segment_count);
                                }
                        }
                }
        }
        
        return 0;
}

int planner_slice(script_t *script, double period, double maxlen)
{
        list_t *list = NULL;
        for (list_t *l = script->atdc_list; l != NULL; l = list_next(l)) {
                atdc_t *atdc = list_get(l, atdc_t);
                list_t *slices = atdc_list_slice(atdc, period, maxlen);
                if (slices == NULL) {
                        r_err("planner_slice: atdc_list_slice returned NULL");
                        delete_section_list(list);
                        return -1;
                }
                list = list_concat(list, slices);
        }
        script->slices = list;
        return 0;
}
