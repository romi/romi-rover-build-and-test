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

namespace romi {

/**
 *   segment
 */

        typedef struct _segment_t  segment_t;

/**
 *  \brief A segment is a node in doubly linked list of sections.
 *
 * A segment is a data structure introduced for convenience. It
 * combines a section and a doubly-linked list.
 *
 */
        struct _segment_t {
                /* The section */
                Section section;

                /* The doubly-linked list */
                segment_t *prev;
                segment_t *next;
        };

        segment_t *new_segment();
        void delete_segment(segment_t *segment);

        void segments_print(segment_t *segment, membuf_t *text);


/**
 *   atdc (acceleration-travel-deceleration-curve)
 */

        typedef struct _atdc_t atdc_t;

/** \brief The atdc_t structure combines four sections: 
 *   1. a straight-line Acceleration (A), 
 *   2. a constant-speed Travel (T), 
 *   3. a straight-line Deceleration (D), 
 *   4. a Curve with constant acceleration (C). 
 */
        struct _atdc_t {
                Section accelerate;
                Section travel;
                Section decelerate;
                Section curve;
        
                atdc_t *prev;
                atdc_t *next;
        };

        atdc_t *new_atdc();
        void delete_atdc(atdc_t *atdc);

        void atdc_print(atdc_t *atdc, membuf_t *text);


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


        struct Script {
                /* The list of moves given by the user. */
                std::vector<Move> actions;

                /* An intermediate representation of the move actions to
                 * facilitate to computation of the ADTC below. */
                segment_t *segments;

                /* The list of move actions rewitten as a list of lists of
                 * acceleration-travel-deceleration-curve (ATDC) sections. */
                atdc_t *atdc;

                /* The list of lists of slices or short sections of constant
                 * speed. */
                list_t *slices;

                /** Move to absolute position (x,y,z) in meters at a speed of v m/s.
                 */
                void moveto(double x, double y, double z, double v);

                /** position is the start position (current position of the CNC) */
                bool convert(double *position,
                             double *xmin, double *xmax,
                             double *vmax, double *amax,
                             double deviation, double period, double maxlen);

        
                void convert_to_segments(double *position);

                void convert_to_atdc(double d, double tmax,
                                     double *xmin, double *xmax, 
                                     double *vmax, double *amax);
                void check_max_speeds(double *vmax);
                void check_max_speed(segment_t *s0, double *vmax);
                void copy_segments_to_atdc();
                void slice(double period, double maxlen);
                list_t *slice(atdc_t *atdc, double period, double maxlen);
                list_t *slice(Section *section, double period, double maxlen);

                void clear();
        };


        void slices_print(list_t *slices, membuf_t *text);

}

#endif // _OQUAM_SCRIPT_H_
