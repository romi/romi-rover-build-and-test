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
#include "v.h"
#include "ATDC.h"

namespace romi {

        /* Compute the maximum allowed acceleration in the direction
           of v. */
        double amax_in_direction(double *amax, double *v)
        {
                double e[3];
                normalize(e, v);

                // double s = DBL_MAX;
                // for (int i = 0; i < 3; i++) {
                //         if (e[i] != 0.0)
                //                 s = std::min(s, amax[i] / fabs(e[i]));
                // }
                // smul(new_a, e, s);
                
                double a[3];
                vmul(a, e, amax);
                return norm(a);
        }

        void assert_equal_speeds(double *v0, double *v1)
        {
                double dv[3];
                vsub(dv, v1, v0);
                if (norm(dv) > 0.00001) {
                        r_err("section with zero length but different speeds");
                        throw std::runtime_error("section with zero length "
                                                 "but different speeds");
                }
        }

        void ATDC::update_start_times(double at)
        {
                accelerate.start_time = at;             
                travel.start_time = accelerate.end_time();
                decelerate.start_time = travel.end_time();
                curve.start_time = decelerate.end_time();
        }
        
        void ATDC::compute_accelerations(double *p0, double *p1,
                                         double *v0, double *v, double *v1,
                                         double *amax)
        {
                double dx[3];
                //vsub(dx, curve.p0, accelerate.p0);
                vsub(dx, p1, p0);

                if (norm(dx) == 0.0) {
                        assert_equal_speeds(v0, v1);
                        curve_only(p0, v0);
                
                } else {
                        do_compute_accelerations(p0, p1, v0, v, v1, amax);
                }
        }
        
        void ATDC::curve_only(double *p, double *v)
        {
                accelerate.zero();
                travel.zero();
                decelerate.zero();

                vcopy(accelerate.p0, p);
                vcopy(accelerate.p1, p);
                vcopy(accelerate.v0, v);
                vcopy(accelerate.v1, v);
                
                vcopy(travel.p0, p);
                vcopy(travel.p1, p);
                vcopy(travel.v0, v);
                vcopy(travel.v1, v);
                
                vcopy(decelerate.p0, p);
                vcopy(decelerate.p1, p);
                vcopy(decelerate.v0, v);
                vcopy(decelerate.v1, v);
        }

        void ATDC::do_compute_accelerations(double *p0, double *p1,
                                            double *v0, double *v, double *v1,
                                            double *amax)
        {
                double v_target[3];
                scale_target_speed(p0, p1, v0, v, v1, amax, v_target);                
                compute_acceleration(p0, v0, v_target, amax);
                compute_deceleration(p1, v_target, v1, amax);
                assert_coherent_travel_length();
                compute_travel(accelerate.p1, decelerate.p0, v_target);
        }

        void ATDC::assert_coherent_travel_length()
        {
                // This is just a sanity check: the travel lenght
                // should not be less than zero (except for rounding
                // errors) because the target speed was set to avoid
                // this. If the lenght is less than zero, throw an
                // exception.
                
                double dx[3];
                double len, len_a, len_d, len_t;

                vsub(dx, curve.p0, accelerate.p0);
                len = norm(dx);
                len_a = accelerate.length();
                len_d = decelerate.length();
                len_t = len - len_a - len_d;

                if (len_t < -0.001) {
                        r_err("atdc_compute_accelerations: negative travel length");
                        throw std::runtime_error("negative travel length");
                }
        }
        
        void ATDC::compute_acceleration(double *p0, double *v0,
                                        double *v1, double *amax)
        {
                double dv[3];
                vsub(dv, v1, v0);
                        
                if (norm(dv) == 0) {
                        zero_acceleration(p0, v0);
                } else {
                        normal_acceleration(p0, v0, v1, amax);
                }
        }
        
        void ATDC::zero_acceleration(double *p, double *v)
        {
                accelerate.duration = 0;
                vzero(accelerate.a); 
                vcopy(accelerate.p0, p);
                vcopy(accelerate.p1, p);
                vcopy(accelerate.v0, v);
                vcopy(accelerate.v1, v);
        }

        void ATDC::normal_acceleration(double *p0, double *v0,
                                       double *v1, double *amax)
        {
                double dx[3];
                double dv[3];
                double dt[3];
                double tmp[3];
                
                vcopy(accelerate.p0, p0);
                vcopy(accelerate.v0, v0);
                vcopy(accelerate.v1, v1);
                
                vsub(dv, accelerate.v1, accelerate.v0);
                vdiv(dt, dv, amax);
                vabs(dt, dt);
                accelerate.duration = vmax(dt);
                
                sdiv(accelerate.a, dv, accelerate.duration);
                
                smul(dx, accelerate.v0, accelerate.duration);
                smul(tmp, accelerate.a, 0.5 * accelerate.duration * accelerate.duration);
                vadd(dx, dx, tmp);
                vadd(accelerate.p1, accelerate.p0, dx);
        }
        
        void ATDC::compute_deceleration(double *p1, double *v0, double *v1, double *amax)
        {
                double dv[3];
                vsub(dv, v0, v1);
                        
                if (norm(dv) == 0) {
                        zero_deceleration(p1, v1);
                } else {
                        normal_deceleration(p1, v0, v1, amax);
                }
        }

        void ATDC::zero_deceleration(double *p, double *v)
        {
                decelerate.duration = 0;
                vzero(decelerate.a); 
                vcopy(decelerate.p0, p);
                vcopy(decelerate.p1, p);
                vcopy(decelerate.v0, v);
                vcopy(decelerate.v1, v);
        }

        void ATDC::normal_deceleration(double *p1, double *v0, double *v1, double *amax)
        {
                double dx[3];
                double dv[3];
                double dt[3];
                double tmp[3];

                vcopy(decelerate.p1, p1);
                vcopy(decelerate.v0, v0);
                vcopy(decelerate.v1, v1);
                
                vsub(dv, decelerate.v0, decelerate.v1);
                vdiv(dt, dv, amax);
                vabs(dt, dt);
                
                decelerate.duration = vmax(dt);
                
                sdiv(decelerate.a, dv, -decelerate.duration);
                
                smul(dx, decelerate.v0, decelerate.duration);
                smul(tmp, decelerate.a, 0.5 * decelerate.duration * decelerate.duration);
                vadd(dx, dx, tmp);
                vsub(decelerate.p0, decelerate.p1, dx);
        }

        void ATDC::compute_travel(double *p0, double *p1, double *v)
        {
                double dx[3];
                vsub(dx, p1, p0);
                
                if (norm(dx) < 0.00001) {
                        zero_travel(p0, v);
                } else {
                        normal_travel(p0, p1, v);
                }
        }

        void ATDC::zero_travel(double *p, double *v)
        {
                travel.duration = 0.0; 
                vcopy(travel.p0, p);
                vcopy(travel.p1, p);
                vcopy(travel.v0, v);
                vcopy(travel.v1, v);
                vzero(travel.a);
        }

        void ATDC::normal_travel(double *p0, double *p1, double *v)
        {
                vcopy(travel.p0, p0);
                vcopy(travel.p1, p1);
                vcopy(travel.v0, v);
                vcopy(travel.v1, v);
                vzero(travel.a);
                
                double len = travel.length();
                double vn = norm(v);
                travel.duration = len / vn; 
        }

        void assert_speeds(double v0, double v_target, double v1)
        {
                if (v_target < v0 || v_target < v1) {
                        // Should not happen! Throw an exception if it does.
                        r_err("scale_target_speed: v_target < v0 || v_target < v1");
                        throw std::runtime_error("scale_target_speed: v_target < v0 || v_target < v1");
                }
        }

        void ATDC::scale_target_speed(double *p0, double *p1,
                                      double *v0, double *v, double *v1, 
                                      double *amax,
                                      double *scaled_v)
        {
                double dx[3];
                vsub(dx, p1, p0);

                // The length of the segment
                double len = norm(dx);

                double v_target = norm(v);
                double v0n = norm(accelerate.v0);
                double v1n = norm(curve.v0);

                assert_speeds(v0n, v_target, v1n);
                
                // The maximum acceleration in the direction of the segment
                double am = amax_in_direction(amax, dx);

                /* 
                   The maximum speed that can be reached by
                   accelerating and decelerating. This can be found as
                   follows:

                   dt0 is the time needed to accelerate from v0 to
                   maximum speed (vm). dt1 is the time needed to
                   decelerate from vm to v1.

                   dt0 = (vm - v0) / am                              (1)
                   dt1 = (vm - v1) / am                              (2)
                   
                   dx0 = v0.dt0 + 0.5a.dt0²
                   dx1 = vt.dt1 - 0.5a.dt1²

                   len = dx0 + dx1 
                   => len = v0.dt0 + 0.5a.dt0² + vt.dt1 - 0.5a.dt1²  (3)
                   => len = vm²/a - v0²/2a - v1²/2a          (substituting 1+2 in 3) 
                   => vm = sqrt(a.len + v0²/2 + v1²/2)
                   
                */
                double v_max = sqrt(am * len + 0.5 * v0n * v0n + 0.5 * v1n * v1n);

                if (v_max < v_target) {
                        double scale = v_max / v_target;
                        smul(scaled_v, v, scale);
                } else {
                        vcopy(scaled_v, v);
                }
        }
}
