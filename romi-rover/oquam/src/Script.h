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
         *  convenience. It combines a section and a doubly-linked
         *  list.
         */
        struct Segment {
                Section section;
                Segment *prev;
                Segment *next;

                Segment() : prev(0), next(0) {}
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
                
                void print_segments(membuf_t *text);
                void print_atdc(membuf_t *text);
                void print_slices(membuf_t *text);

                bool check_validity(double tmax,
                                    double *xmin, double *xmax, 
                                    double *vmax, double *amax);
                
        protected:
                
                void set_position(double *p);
                void convert_to_segments();
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
                void compute_curve(Segment *s0, ATDC *t0, double d, double *amax);
                void update_speeds(Segment *s0, ATDC *t0, double *amax);
                void compute_accelerations(Segment *s0, ATDC *t0, double *amax);
                void compute_accelerations(double *amax);
                void update_start_times();
                
                void print_atdc(ATDC *atdc, int index, membuf_t *text);
        };
}

#endif // _OQUAM_SCRIPT_H_
