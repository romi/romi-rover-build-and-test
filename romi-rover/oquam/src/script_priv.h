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

#ifndef _OQUAM_SCRIPT_PRIV_H_
#define _OQUAM_SCRIPT_PRIV_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 *   action
 */

enum {
        ACTION_MOVE,
};

typedef struct _controller_t controller_t;

typedef struct _action_t action_t;

struct _action_t {
        unsigned char type;

        union {
                struct {
                        double p[3];
                        double v;
                } move;
                
        } data;
};

action_t *new_action(int type);
action_t *action_clone(action_t *a);
action_t *new_move(double x, double y, double z, double v);
void delete_action(action_t *action);


/**
 *   section
 */

typedef struct _section_t section_t;

/**
 *  \brief A section represents a component of the longer path that is
 *  traveled with a constant (including zero) acceleration.
 *
 * The following the properties should be satisfied:
 *    v1 = v0 + a.t
 *    d = p1 - p0
 *    d = v0.t + at²/2
 */
struct _section_t {
        // The absolute positions at the beginning and end of the
        // section in meters.
        double p0[3];
        double p1[3];

        // The duration of this section, if known.
        double t;
        
        // The absolute start time of this section, if known. The
        // start time is measured from the beginning of the continuous
        // path to which this segment belongs.
        double at;

        // The speed at the start of this section, in m/s.
        double v0[3];

        // The speed at the end of this section. 
        double v1[3]; 

        // The acceleation of this section, in m/s².
        double a[3];
        
        // The relative position at the end of the section, in meters.
        double d[3];
};

section_t *new_section();
void delete_section(section_t *section);

void section_print(section_t *section, const char *prefix);

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
        section_t section;

        /* The doubly-linked list */
        segment_t *prev;
        segment_t *next;
};

segment_t *new_segment();
void delete_segment(segment_t *segment);

void segments_print(segment_t *segment);


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
        section_t accelerate;
        section_t travel;
        section_t decelerate;
        section_t curve;
        
        atdc_t *prev;
        atdc_t *next;
};

atdc_t *new_atdc();
void delete_atdc(atdc_t *atdc);

void atdc_print(atdc_t *atdc);

/**
 *   block
 */

enum {
        BLOCK_MOVE = 0,
        BLOCK_MOVETO,
        BLOCK_MOVEAT,
};

/** 
 * \brief A low-level instruction block for the controller.
 */
typedef struct _block_t {
        uint8_t type;
        int16_t data[4];
} block_t;


/**
 *   script
 */

typedef struct _script_t {
        /* The list of actions given by the user. */
        list_t *actions;

        /* An intermediate representation of the move actions to
         * facilitate to computation of the ADTC below. */
        segment_t *segments;

        /* The list of move actions rewitten as a list of lists of
         * acceleration-travel-deceleration-curve (ATDC) sections. */
        atdc_t *atdc;

        /* The list of lists of slices or short sections of constant
         * speed. */
        list_t *slices;

        /* The low-level instructions */
        block_t *block;
        int32_t num_blocks;
        int32_t block_length;

} script_t;

void script_clear(script_t *script);
int script_push_block(script_t* script, block_t *block);

#ifdef __cplusplus
}
#endif

#endif // _OQUAM_SCRIPT_PRIV_H_
