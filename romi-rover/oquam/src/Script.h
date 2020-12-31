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

#ifndef _OQUAM_SCRIPT_H_
#define _OQUAM_SCRIPT_H_

#include <r.h>
#include <vector>
#include "Section.h"
#include "ATDC.h"
#include "v.h"

namespace romi {
        
        struct Move {
                double p[3];
                double v;

                Move(double x, double y, double z, double speed) {
                        p[0] = x;
                        p[1] = y;
                        p[2] = z;
                        v = speed;
                }
        };

        /** A segment is a data structure introduced for
         *  convenience. It converts the data of the move requests
         *  into vectors. It is used to build up the ATDC list.
         */
        struct Segment {
                double p0[3];
                double p1[3];
                double v[3];
                
                //Section section;
                
                Segment *prev;
                Segment *next;

                Segment() : prev(0), next(0) {}

                double *displacement(double *d) {
                        vsub(d, p1, p0);
                        return d;
                }

                double length() {
                        double d[3];
                        displacement(d);
                        return norm(d);
                }
                
                double *direction(double *d) {
                        displacement(d);
                        normalize(d, d);
                        return d;
                }

        };
        
        class Script
        {
        protected:
                
                /* The list of moves given by the user. */
                std::vector<Move> _moves;
                double _position[3];
                
        public:
                
                /* An intermediate representation of the move actions to
                 * facilitate to computation of the ADTC below. */
                Segment *segments;

                /* The list of move actions rewitten as a list of lists of
                 * acceleration-travel-deceleration-curve (ATDC) sections. */
                ATDC *atdc;

                /* The list of lists of slices or short sections of constant
                 * speed. */
                //list_t *slices;
                std::vector<Section> slices;
                
                Script(double *start_position) : segments(0), atdc(0), slices(0) {
                        set_position(start_position);
                }
                
                virtual ~Script();
                
                /* Move to absolute position (x,y,z) in meters at an
                 * absolute speed of v, in m/s.
                 */
                void moveto(double x, double y, double z, double v);

                void convert(double *vmax,
                             double *amax,
                             double deviation,
                             double period,
                             double maxlen);

                size_t count_moves() {
                        return _moves.size();
                }
                
                Move& get_move(size_t index) {
                        return _moves[index];
                }
                
        protected:
                
                void set_position(double *p);
                void convert_to_segments();
                void init_segment_positions(Segment& segment, Move& move);
                void init_segment_speed(Segment& segment, Move& move);
                void convert_to_atdc(double d, double *vmax, double *amax);
                void check_max_speeds(double *vmax);
                void check_max_speed(Segment *s0, double *vmax);
                void copy_segments_to_atdc();
                void slice(double period, double maxlen);
                void slice(ATDC *atdc, double period, double maxlen);
                void slice(Section *section, double period, double maxlen);
                
                void compute_curves_and_speeds(double d, double *amax);
                
                void compute_curve_and_speeds(Segment *s0, ATDC *t0,
                                              double d, double *amax);
                
                void compute_curve(Segment *s0, ATDC *t0, double deviation, double *amax);
                
                void no_curve(Segment *s0, ATDC *t0);
                
                void default_curve(Segment *s0, Segment *s1, ATDC *t0, 
                                   double deviation, double *amax);
                
                void initialize_next_acceleration(ATDC *t0);
                bool update_speeds_if_needed(ATDC *t0, double *amax);

                // Returns true is the changes should be
                // back-propagated to the previous sections
                bool adjust_speeds(ATDC *t, double *amax);
                
                bool update_curve_speed_if_needed(ATDC *t);
                void has_next_segment_slowed_down(ATDC *t0);
                void reduce_entry_speed(ATDC *t, double *amax);
                void reduce_exit_speed(ATDC *t, double *amax);
                void back_propagate_speed_change(Segment *s0, ATDC *t0, double *amax);
                void slow_down_curve(ATDC *t0, double factor);
                void assert_not_first(Segment *s, ATDC *t);
                void compute_accelerations(Segment *s0, ATDC *t0, double *amax);
                void compute_accelerations(double *amax);
                void update_start_times();
                double get_curve_speed_magnitude(Segment *s0, Segment *s1);
                void get_curve_speed_vector(Segment *s, double magnitude, double *w);
                void compute_curve_entry_point(Segment *s, double distance, double *p);
                void compute_curve_exit_point(Segment *s, double distance, double *p);
                double scale_speed(double am, double deviation, double speed);
                double scale_distance(double distance, Segment *s0, Segment *s1);
                double shortest_length(Segment *s0, Segment *s1);
                double required_acceleration_path_length(ATDC *t, double *amax);
                double required_acceleration_path_length(double *v0, double *v1, double *d, double *amax);
                double required_acceleration_path_length(double v0, double v1, double a);
        };
}

#endif // _OQUAM_SCRIPT_H_
