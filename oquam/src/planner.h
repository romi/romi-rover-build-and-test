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
#include "cnc.h"
#include "script.h"
#include "controller.h"

#ifndef _OQUAM_PLANNER_H_
#define _OQUAM_PLANNER_H_

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************/

typedef struct _section_t section_t;

/*
 * The following the equations should be satisfied:
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

        list_t *actions;
};

section_t *new_section();
void delete_section(section_t *section);

/******************************************************************/

typedef struct _segment_t  segment_t;

struct _segment_t {
        section_t section;
        list_t *actions;
        segment_t *prev;
        segment_t *next;
};

/******************************************************************/

typedef struct _atdc_t atdc_t;

/** The atdc_t structure combines four sections: a straight-line
 * acceleration, a constant-speed travel, a straight-line
 * deceleration, and a curve with constant acceleration. */
struct _atdc_t {
        section_t accelerate;
        section_t travel;
        section_t decelerate;
        section_t curve;

        list_t *actions;
        atdc_t *prev;
        atdc_t *next;
};

/******************************************************************/

typedef struct _planner_t planner_t;

planner_t *new_planner(cnc_t *cnc, controller_t *controller, script_t *script);
void delete_planner(planner_t *planner);

list_t *planner_slice(planner_t *planner, double period, double maxlen);

list_t *planner_get_segments_list(planner_t *planner);
list_t *planner_get_atdc_list(planner_t *planner);

#ifdef __cplusplus
}
#endif

#endif // _OQUAM_PLANNER_H_
