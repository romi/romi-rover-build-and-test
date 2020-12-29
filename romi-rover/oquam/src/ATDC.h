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

#ifndef _OQUAM_ATDC_H_
#define _OQUAM_ATDC_H_

#include "Section.h"

namespace romi {

        void amax_in_direction(double *amax, double *v, double *new_a);

        /**  The ATDC structure combines four sections: 
         *   1. a straight-line Acceleration (A), 
         *   2. a constant-speed Travel (T), 
         *   3. a straight-line Deceleration (D), 
         *   4. a Curve with constant acceleration (C). 
         */
        class ATDC
        {
        public:
                Section accelerate;
                Section travel;
                Section decelerate;
                Section curve;
        
                ATDC *prev;
                ATDC *next;

                ATDC() : prev(0), next(0) {}

                bool is_valid(double tmax,
                              double *xmin, double *xmax, 
                              double *vmax, double *amax);
                
                void update_start_times(double at);
                void compute_accelerations(double *p0, double *p1,
                                           double *v0, double *v, double *v1,
                                           double *amax);

                void print();
                void print(membuf_t *text);

        private:
                
                void do_compute_accelerations(double *p0, double *p1,
                                              double *v0, double *v, double *v1,
                                              double *amax);
                void curve_only(double *p, double *v);
                
                void compute_acceleration(double *p0, double *v0, double *v1, double *amax);
                void zero_acceleration(double *p, double *v);
                void normal_acceleration(double *p0, double *v0, double *v1, double *amax);
                
                void compute_deceleration(double *p1, double *v0, double *v1, double *amax);
                void zero_deceleration(double *p, double *v);
                void normal_deceleration(double *p1, double *v0, double *v1, double *amax);
                
                void compute_travel(double *p0, double *p1, double *v);                
                void zero_travel(double *p, double *v);
                void normal_travel(double *p0, double *p1, double *v);
                
                void assert_coherent_travel_length();
                void assert_equal_speeds(double *v0, double *v1);
                
                void scale_target_speed(double *p0, double *p1,
                                        double *v0, double *v, double *v1, 
                                        double *amax, double *scaled_v);
                bool points_and_speeds_match();
                bool points_and_speeds_match(Section& first, Section& second);

        };

}

#endif // _OQUAM_ATDC_H_
