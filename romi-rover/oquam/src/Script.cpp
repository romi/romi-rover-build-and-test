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
#include "print.h"

namespace romi {

        static void define_unit_vectors(double *w0, double *w1,
                                        double *ex, double *ey);
        static bool are_collinear(double *w0, double *w1);
        static bool is_same_direction(double *w0, double *w1);
        static void define_unit_vectors_collinear(double *w0, double *w1,
                                                  double *ex, double *ey);
        static void define_unit_vectors_same_dir(double *w0, double *w1,
                                                 double *ex, double *ey);
        static void define_unit_vectors_opposite_dir(double *w0, double *w1,
                                                     double *ex, double *ey);
        static void define_unit_vectors_default(double *w0, double *w1,
                                                double *ex, double *ey);

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
        
        void Script::compute_curve(Segment *s0, ATDC *t0, double deviation, double *amax)
        {
                if (s0->next == NULL) {
                        no_curve(s0, t0);
                } else {
                        default_curve(s0, s0->next, t0, deviation, amax);
                }
        }

        void Script::no_curve(Segment *s0, ATDC *t0)
        {
                t0->curve.zero();
                vcopy(t0->curve.p0, s0->p1);
                vcopy(t0->curve.p1, s0->p1);
        }
        
        void Script::default_curve(Segment *s0, Segment *s1, ATDC *t0,
                                   double deviation, double *amax)
        {
                // The speeds and their directions before and after the curve.
                double w;
                double w0[3];
                double w1[3];

                w = get_curve_speed_magnitude(s0, s1);
                get_curve_speed_vector(s0, w, w0);
                get_curve_speed_vector(s1, w, w1);
        
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

                double am = amax_in_direction(amax, ey);
                
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

                // Scale the speed if it is too fast, given the
                // acceleration and deviation.
                double vscale = scale_speed(am, deviation, wy0);
                wx0 *= vscale;
                wy0 *= vscale;
                smul(w0, w0, vscale);
                smul(w1, w1, vscale);
        
                // Compute the entry and exit coordinates 
                double x0 = wy0 * wx0 / am;
                double y0 = wy0 * wy0 / am;

                // The distance between the entry/exit points and the
                // junction.
                double distance = sqrt(x0 * x0 + y0 * y0);

                // If the entry or exit point is more than halfway
                // along the segment then we should slow down. We
                // don't want the entry or exit point to be over the
                // half of the segment's length because that would
                // cause problems when chaining together short
                // segments.
                double dscale = scale_distance(distance, s0, s1);
                double sqrt_dscale = sqrt(dscale);

                wy0 *= sqrt_dscale;
                smul(w0, w0, sqrt_dscale);
                smul(w1, w1, sqrt_dscale);
                distance *= dscale;


                double duration = fabs(2.0 * wy0 / am);
                double p0[3];
                double p1[3];
                double a[3];
                
                compute_curve_entry_point(s0, distance, p0);
                compute_curve_exit_point(s1, distance, p1);
                smul(a, ey, am);
                
                t0->curve.set(duration, 0, p0, p1, w0, w1, a);
        }
        
        static void define_unit_vectors(double *w0, double *w1,
                                        double *ex, double *ey)
        {
                if (are_collinear(w0, w1)) {
                        define_unit_vectors_collinear(w0, w1, ex, ey);
                } else {
                        define_unit_vectors_default(w0, w1, ex, ey);
                }
        }

        static bool are_collinear(double *w0, double *w1)
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
                if (is_same_direction(w0, w1)) {
                        define_unit_vectors_same_dir(w0, w1, ex, ey);
                } else {
                        define_unit_vectors_opposite_dir(w0, w1, ex, ey);
                }
        }

        static bool is_same_direction(double *w0, double *w1)
        {
                // It should already be established that w0 and w1 are
                // collinear. We check the factor 'n' so that
                // w0=n.w1. If n>0 then w0 and w1 point in the same
                // direction.
                double n = 1.0;
                if (w1[0] != 0.0)
                        n = w0[0] / w1[0];
                else if (w1[1] != 0.0)
                        n = w0[1] / w1[1];
                else if (w1[2] != 0.0)
                        n = w0[2] / w1[2];
                return (n > 0.0);
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

        double Script::get_curve_speed_magnitude(Segment *s0, Segment *s1)
        {
                // Set the entry and exit speed of the curve to smallest speed
                // before and after the junction.
                return std::min(norm(s0->v), norm(s1->v));
        }
        
        void Script::get_curve_speed_vector(Segment *segment, double magnitude, double *w)
        {
                double dir[3];
                segment->direction(dir);
                smul(w, dir, magnitude);
        }
        
        void Script::compute_curve_entry_point(Segment *segment, double distance, double *p)
        {
                double dir[3];
                segment->direction(dir);
                // Compute a vector along the segment with length 'distance'
                smul(p, dir, distance);
                // Substract the vector from the end point. 
                vsub(p, segment->p1, p);
        }
        
        void Script::compute_curve_exit_point(Segment *segment, double distance, double *p)
        {
                double dir[3];
                segment->direction(dir);
                // Compute a vector along the segment with length 'distance'
                smul(p, dir, distance);
                // Add the vector to the start point. 
                vadd(p, segment->p0, p);
        }
        
        double Script::scale_speed(double am, double deviation, double speed)
        {
                // Check for maximum velocity at the entry point,
                // given the maximum acceleration and deviation. If
                // the current speed is higher than this maximum speed
                // then the entry velocity must be scaled
                // proportionally.
                double max_speed = sqrt(2.0 * am * deviation);
                double scale = 1.0;
        
                if (fabs(speed) > max_speed)
                        scale = max_speed / speed;

                return scale;
        }

        double Script::scale_distance(double distance, Segment *s0, Segment *s1)
        {
                double scale = 1.0;
                double len = shortest_length(s0, s1);
                if (distance > len / 2.0) {
                        scale = len / (2.0 * distance);
                }
                return scale;
        }

        double Script::shortest_length(Segment *s0, Segment *s1)
        {
                double len0 = s0->length();
                double len1 = s1->length();
                return std::min(len0, len1);
        }

        bool Script::update_speeds_if_needed(ATDC *t, double *amax)
        {
                bool must_backpropagate = false;
                
                /* Compare the speeds at the start of the segments and at the
                   entry of the curve. Check whether there is enough space to
                   accelerate/decelerate. If not, adapt one of the two
                   speeds. */
                
                /* The available path length */
                double segment_length = vdist(t->accelerate.p0, t->curve.p0);
                double required_length = required_acceleration_path_length(t, amax);

                if (required_length > segment_length) {
                        must_backpropagate = adjust_speeds(t, amax);
                }

                return must_backpropagate;
        }
        
        double Script::required_acceleration_path_length(ATDC *t, double *amax)
        {
                double length = 0.0;
                double displacement[3];
                
                vsub(displacement, t->curve.p0, t->accelerate.p0);

                if (norm(displacement) == 0) {
                        length = required_acceleration_path_length(t->accelerate.v0,
                                                                   t->curve.v0,
                                                                   amax);
                } else {
                        length = required_acceleration_path_length(t->accelerate.v0,
                                                                   t->curve.v0,
                                                                   displacement,
                                                                   amax);
                }

                return length;
        }
        
        double Script::required_acceleration_path_length(double *v0, double *v1, double *amax)
        {
                double v0n = norm(v0);
                double v1n = norm(v1);
                double a = norm(amax);
                return required_acceleration_path_length(v0n, v1n, a);
        }

        double Script::required_acceleration_path_length(double *v0, double *v1,
                                                         double *d, double *amax)
        {
                double v0n = norm(v0);
                double v1n = norm(v1);
                double a = amax_in_direction(amax, d);
                return required_acceleration_path_length(v0n, v1n, a);
        }

        double Script::required_acceleration_path_length(double v0, double v1, double a)
        {
                double dv = v1 - v0;
                double dt = fabs(dv / a);
                double dx = v0 * dt + sign(dv) * 0.5 * a * dt * dt;
                return dx;
        }

        bool Script::adjust_speeds(ATDC *t, double *amax)
        {
                /* We know that the segment length is smaller than the
                 * required length to accelerate or slow down to the
                 * desired speed. */

                double v0 = norm(t->accelerate.v0);
                double v1 = norm(t->curve.v0);

                bool must_backpropagate = false;
                
                if (v0 > v1) {
                        /* The speed at the entry of this segment is
                           too high and the segment is not long enough
                           to slow down to the maximum entry speed of
                           the curve at the end of the segment. We
                           must reduce the speed at the start of this
                           segment. */
                        reduce_entry_speed(t, amax);
                        must_backpropagate = true;
                        
                } else if (v0 < v1) {
                        /* The segment is too short to accelerate up
                           to the desired entry speed of the
                           curve. Reduce the curve's entry speed.  */
                        reduce_exit_speed(t, amax);
                }
                
                return must_backpropagate;
        }

        void Script::reduce_entry_speed(ATDC *t, double *amax)
        {
                /*  v1 < v0 and a > 0 and t > 0
                    v1 = v0 - at  => t = (v0 - v1) / a    (1)
                    L = v0.t - at²/2                      (2)
                    (1,2) => L = (v1² - v0²) / 2a
                          => v0 = sqrt(v1² + 2aL)
                 */
                double displacement[3];
                vsub(displacement, t->curve.p0, t->accelerate.p0);
                
                double length = norm(displacement);
                double v0 = norm(t->accelerate.v0);
                double v1 = norm(t->curve.v0);
                double a = 0.0;
                if (norm(displacement) > 0.0) 
                        a = amax_in_direction(amax, displacement);
                double v0s = sqrt(v1 * v1 + 2 * a * length);
                double factor = v0s / v0;
                smul(t->accelerate.v0, t->accelerate.v0, factor);
        }

        void Script::reduce_exit_speed(ATDC *t, double *amax)
        {
                /*  L = v0.t + at²/2                      (1)
                    v1 = v0 + at  => t = (v1 - v0) / a    (2)
                    (1,2) => L = (v1² - v0²) / 2a
                          => v1 = sqrt(v0² + 2aL)
                 */
                double displacement[3];
                vsub(displacement, t->curve.p0, t->accelerate.p0);
                
                double length = norm(displacement);
                double v0 = norm(t->accelerate.v0);
                double v1 = norm(t->curve.v0);
                double a = amax_in_direction(amax, displacement);
                double v1s = sqrt(v0 * v0 + 2.0 * a * length);
                double factor = v1s / v1;
                
                slow_down_curve(t, factor);
        }

        void Script::back_propagate_speed_change(Segment *s0, ATDC *t, double *amax)
        {
                /* This fuctions is called recursively to propagate
                 * changes from the end of the path back to the
                 * front. We may have been called because the next
                 * segment had to slow down. If so, update the exit
                 * speed of the current segement. */

                if (update_curve_speed_if_needed(t)
                    && update_speeds_if_needed(t, amax)) {
                        assert_not_first(s0, t);
                        back_propagate_speed_change(s0->prev, t->prev, amax);
                }                        
        }

        void Script::assert_not_first(Segment *s, ATDC *t)
        {
                if (s->prev == NULL || t->prev == NULL) {
                        r_err("check_curve_speed_forward: pushing speed change "
                              "backwards but we're at the first segment?!");
                        throw std::runtime_error("assert_not_first failed");
                }
        }
        
        bool Script::update_curve_speed_if_needed(ATDC *t)
        {
                bool changed = false;
                if (t->next) {
                        double v1 = norm(t->curve.v0);
                        double v0_next = norm(t->next->accelerate.v0);
                        if (v1 != v0_next) {
                                // The next segment lowered its entry speed. Slow down the
                                // curve.
                                slow_down_curve(t, v0_next / v1);
                                changed = true;
                        }
                }
                return changed;
        }
        
        void Script::slow_down_curve(ATDC *t, double factor)
        {
                smul(t->curve.v0, t->curve.v0, factor);
                smul(t->curve.v1, t->curve.v1, factor);
                smul(t->curve.a, t->curve.a, factor * factor);
                t->curve.duration /= factor;
        }
        
        /**
         *  Computes the curve at the end of the segment. This function may
         *  adjust the entry speed of the segment to assure that the curve
         *  isn't entered with too high a speed.
         *
         *  s0: The current segment.
         *  amax: The maximum acceleration that the machine tolerates. 
         */
        void Script::compute_curve_and_speeds(Segment *s, ATDC *t,
                                              double deviation, double *amax)
        {
                compute_curve(s, t, deviation, amax);
                
                if (update_speeds_if_needed(t, amax)) {
                        assert_not_first(s, t);
                        back_propagate_speed_change(s->prev, t->prev, amax);
                }
                
                initialize_next_acceleration(t);
        }
        
        void Script::initialize_next_acceleration(ATDC *t0)
        {
                ATDC *t1 = t0->next;
                if (t1) {
                        vcopy(t1->accelerate.v0, t0->curve.v1);
                        vcopy(t1->accelerate.p0, t0->curve.p1);
                }
        }

        void Script::check_max_speed(Segment *segment, double *vmax)
        {
                double s = 1.0;

                for (int i = 0; i < 3; i++) {
                        if (segment->v[i] != 0.0)
                                s = std::min(s, vmax[i] / fabs(segment->v[i]));
                }
        
                smul(segment->v, segment->v, s);
        }

        void Script::compute_curves_and_speeds(double deviation, double *amax)
        {
                Segment *segment = segments;
                ATDC *t = atdc;
        
                while (segment) {
                        compute_curve_and_speeds(segment, t, deviation, amax);
                        segment = segment->next;
                        t = t->next;
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
                vcopy(target_speed, s0->v);
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
                        at = t0->curve.end_time();
                        t0 = t0->next;
                }
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

        void Script::init_segment_positions(Segment& segment, Move& move)
        {
                vcopy(segment.p0, _position); 
                vcopy(segment.p1, move.p); 
        }

        void Script::init_segment_speed(Segment& segment, Move& move)
        {
                double d[3];
                vsub(d, segment.p1, segment.p0);
                normalize(d, d);
                smul(segment.v, d, move.v);
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

                                init_segment_positions(*segment, move);
                                init_segment_speed(*segment, move);

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
}
