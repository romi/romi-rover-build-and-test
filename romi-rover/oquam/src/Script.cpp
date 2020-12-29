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
#include <float.h>
#include <stdexcept>
#include <algorithm>
#include <r.h>
#include "Script.h"
#include "v.h"

namespace romi {

        static inline double sign(double a)
        {
                return (a < 0.0)? -1.0 : 1.0;
        }

        Script::~Script()
        {
                if (segments) {
                        Segment *list = segments;
                        while (list != NULL) {
                                Segment *next = list->next;
                                delete list;
                                list = next;
                        }
                }
        
                if (atdc) {
                        ATDC *list = atdc;
                        while (list != NULL) {
                                ATDC *next = list->next;
                                delete list;
                                list = next;
                        }
                }
        }

        void Script::moveto(double x, double y, double z, double v)
        {
                if (v <= 0.0f) {
                        r_warn("script_moveto: speed must be positive");
                        throw std::runtime_error("Script::moveto: invalid speed");
                }
                _moves.push_back(Move(x, y, z, v));
        }

        void Script::slice(ATDC *atdc, double period, double maxlen)
        {
                atdc->accelerate.slice(slices, period, maxlen);
                atdc->travel.slice(slices, period, maxlen);
                atdc->decelerate.slice(slices, period, maxlen);
                atdc->curve.slice(slices, period, maxlen);
        }

        void Script::slice(double period, double maxlen)
        {
                ATDC *s = atdc;
                while (s != NULL) {
                        slice(s, period, maxlen);
                        s = s->next;
                }
        }

        static bool is_collinear(double *w0, double *w1);
        static void define_unit_vectors_collinear(double *w0, double *w1,
                                                  double *ex, double *ey);
        static void define_unit_vectors_same_dir(double *w0, double *w1,
                                                 double *ex, double *ey);
        static void define_unit_vectors_opposite_dir(double *w0, double *w1,
                                                     double *ex, double *ey);
        static void define_unit_vectors_default(double *w0, double *w1,
                                                double *ex, double *ey);

        
        static void define_unit_vectors(double *w0, double *w1,
                                        double *ex, double *ey)
        {
                if (is_collinear(w0, w1)) {
                        define_unit_vectors_collinear(w0, w1, ex, ey);
                
                } else {
                        define_unit_vectors_default(w0, w1, ex, ey);
                }
        }

        static bool is_collinear(double *w0, double *w1)
        {
                /* Test whether the two speeds w0 and w1 are collinear by
                 * checking whether the cross product is zero.  */
                double c[3];
                vcross(c, w0, w1);
                double w = norm(w0);
                double cww = norm(c) / (w * w);
                return (cww < 0.001);
        }

        static void define_unit_vectors_collinear(double *w0, double *w1,
                                                  double *ex, double *ey)
        {
                double n;
                
                if (w1[0] != 0.0)
                        n = w0[0] / w1[0];
                else if (w1[1] != 0.0)
                        n = w0[1] / w1[1];
                else if (w1[2] != 0.0)
                        n = w0[2] / w1[2];
                
                if (n > 0.0) {
                        define_unit_vectors_same_dir(w0, w1, ex, ey);
                } else {
                        define_unit_vectors_opposite_dir(w0, w1, ex, ey);
                }
        }

        static void define_unit_vectors_same_dir(double *w0, double *w1,
                                                 double *ex, double *ey)
        {
                normalize(ex, w0);
                ey[0] = -ex[1];
                ey[1] = ex[0];
                ey[2] = ex[2];
        }

        static void define_unit_vectors_opposite_dir(double *w0, double *w1,
                                                     double *ex, double *ey)
        {
                normalize(ey, w0);
                smul(ey, ey, -1);
                ex[0] = ey[1];
                ex[1] = -ey[0];
                ex[2] = ey[2];
        }
        
        static void define_unit_vectors_default(double *w0, double *w1,
                                                double *ex, double *ey)
        {
                vadd(ex, w1, w0);
                normalize(ex, ex);
                vsub(ey, w1, w0);
                normalize(ey, ey);
        }

        void Script::compute_curve(Segment *s0, ATDC *t0, double d, double *amax)
        {
                Segment *s1 = s0->next;
                ATDC *t1 = t0->next;

                if (s1 == NULL) {
                        t0->curve.zero();
                        vcopy(t0->curve.p0, s0->section.p1);
                        vcopy(t0->curve.p1, s0->section.p1);
                        return;
                }

                // Set the entry and exit speed of the curve to smallest speed
                // before and after the junction.
                double w = std::min(norm(s0->section.v0), norm(s1->section.v0));

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

                define_unit_vectors(w0, w1, ex, ey);
                wx0 = vdot(w0, ex);
                wy0 = -vdot(w0, ey);

                /* Compute the maximum allowed acceleration in the direction
                   of wy (or ey). */
                double a[3];
                amax_in_direction(amax, ey, a);

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
                double len = std::min(len0, len1);
        
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

                vsub(t0->curve.d, t0->curve.p1, t0->curve.p0);

                /* Copy the next segment's entry speed and entry point to its
                 * accelerate section */
                vcopy(t1->accelerate.v0, t0->curve.v1);
                vcopy(t1->accelerate.p0, p1);
        
                /* The acceleration, in the coordinate space of the CNC
                 * machine. */
                vcopy(t0->curve.a, a);                                   // a
        
                /* The time it takes to follow the speed curve. */
                t0->curve.duration = fabs(2.0 * wy0 / am);                      // t
        }

        /**
         *  Readjusts the speed in case the next segment had to slow down.
         *
         */
        void Script::update_speeds(Segment *s0, ATDC *t0, double *amax)
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
                        t0->curve.duration /= alpha;
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
                
                        update_speeds(s0->prev, t0->prev, amax);
                
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
                        t0->curve.duration /= alpha;
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
        void Script::compute_curve_and_speeds(Segment *s0, ATDC *t0,
                                              double d, double *amax)
        {
                compute_curve(s0, t0, d, amax);
                update_speeds(s0, t0, amax);
        }

        void Script::check_max_speed(Segment *s0, double *vmax)
        {
                double s = 1.0;

                for (int i = 0; i < 3; i++) {
                        if (s0->section.v0[i] != 0.0)
                                s = std::min(s, vmax[i] / fabs(s0->section.v0[i]));
                }
        
                smul(s0->section.v0, s0->section.v0, s);
                smul(s0->section.v1, s0->section.v1, s);
        }

        void Script::compute_curves_and_speeds(double d, double *amax)
        {
                Segment *s0 = segments;
                ATDC *t0 = atdc;
        
                while (s0) {
                        compute_curve_and_speeds(s0, t0, d, amax);
                        s0 = s0->next;
                        t0 = t0->next;
                }
        }

        void Script::compute_accelerations(Segment *s0, ATDC *t0, double *amax)
        {
                double from[3];
                double to[3];
                double start_speed[3];
                double target_speed[3];
                double arrival_speed[3];

                // Make copies because the sections may get
                // reinitialized.
                vcopy(from, t0->accelerate.p0);
                vcopy(to, t0->curve.p0);
                vcopy(start_speed, t0->accelerate.v0);
                vcopy(target_speed, s0->section.v0);
                vcopy(arrival_speed, t0->curve.v0);
                
                t0->compute_accelerations(from, to, start_speed,
                                          target_speed, arrival_speed,
                                          amax);
        }
        
        void Script::compute_accelerations(double *amax)
        {
                Segment *s0 = segments;
                ATDC *t0 = atdc;
        
                while (s0) {
                        compute_accelerations(s0, t0, amax);
                        s0 = s0->next;
                        t0 = t0->next;
                }
        }

        void Script::update_start_times()
        {
                ATDC *t0 = atdc;
                double at = 0.0;
        
                while (t0) {
                        t0->update_start_times(at);
                        at = t0->curve.at + t0->curve.duration;
                        t0 = t0->next;
                }
        }

        bool Script::check_validity(double tmax,
                                    double *xmin, double *xmax, 
                                    double *vmax, double *amax)
        {
                bool valid = true;
                ATDC *t0 = atdc;
                int atdc_count = 0;
        
                while (t0) {
                        if (!t0->is_valid(tmax, xmin, xmax, vmax, amax)) {
                                valid = false;
                                r_warn("compute_accelerations: invalid atdc: "
                                       "atdc %d", atdc_count);
                                t0->print();
                        }
                        t0 = t0->next;
                        atdc_count++;
                }
                
                return valid;
        }

        void Script::check_max_speeds(double *vmax)
        {
                for (Segment *segment = segments; segment != NULL; segment = segment->next)
                        check_max_speed(segment, vmax);
        }

        void Script::copy_segments_to_atdc()
        {
                ATDC *first = NULL;
                ATDC *prev = NULL;
        
                for (Segment *segment = segments; segment != NULL; segment = segment->next) {
                        ATDC *atdc = new ATDC();
                
                        // Append to double-linked list
                        if (first == NULL)
                                first = atdc;
                        if (prev != NULL) {
                                prev->next = atdc;
                                atdc->prev = prev;
                        }
                        prev = atdc;
                }

                atdc = first;
        }

        void Script::convert_to_atdc(double d, double *vmax, double *amax)
        {
                check_max_speeds(vmax);
                copy_segments_to_atdc();
                compute_curves_and_speeds(d, amax);
                compute_accelerations(amax);
                update_start_times();
        }

        void Script::convert_to_segments()
        {
                Segment *first = NULL;
                Segment *prev = NULL;

                for (size_t i = 0; i < _moves.size(); i++) {
                
                        Move& move = _moves[i];
                
                        /* If the displacement is less then 0.1 mm, skip it. */
                        double d[3];
                        vsub(d, move.p, _position); 
                        if (norm(d) > 0.0001) {
                                Segment *segment = new Segment();

                                // chain the segment
                                segment->prev = prev;
                                if (prev == NULL) 
                                        first = segment;
                                else 
                                        prev->next = segment;
                        
                                prev = segment;

                                // p0 and p1
                                vcopy(segment->section.p0, _position); 
                                vcopy(segment->section.p1, move.p); 

                                // d
                                vsub(segment->section.d,
                                     segment->section.p1,
                                     segment->section.p0); 
                        
                                // v0 and v1
                                normalize(segment->section.v0, segment->section.d);
                                smul(segment->section.v0,
                                     segment->section.v0,
                                     move.v);
                                vcopy(segment->section.v1, segment->section.v0);
                        
                                // a
                                vzero(segment->section.a);

                                // update current position
                                vcopy(_position, move.p); 
                        }                        
                }
        
                segments = first;
        }

        void Script::set_position(double *p)
        {
                vcopy(_position, p);
        }

        void Script::convert(double *vmax, double *amax,
                             double deviation, double period,
                             double maxlen)
        {
                convert_to_segments();
                convert_to_atdc(deviation, vmax, amax);
                slice(period, maxlen);
        }

        void Script::print_slices(membuf_t *text)
        {
                for (size_t k = 0; k < slices.size(); k++) {
                        slices[k].print(text, "Slice");
                }
        }

        void Script::print_segments(membuf_t *text)
        {
                Segment *s = segments;
                while (s != NULL) {
                        s->section.print(text, "S");
                        s = s->next;
                }
        }

        void Script::print_atdc(ATDC *atdc, int index, membuf_t *text)
        {
                membuf_printf(text, "%d:\n", index);
                atdc->print(text);
        }
        
        void Script::print_atdc(membuf_t *text)
        {
                int count = 1;
                ATDC *s = atdc;
                while (s != NULL) {
                        print_atdc(s, count++, text);
                        s = s->next;
                        if (s) {
                                membuf_printf(text, "-\n");
                        }
                }
        }
}
