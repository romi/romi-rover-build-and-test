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

#ifndef _OQUAM_SECTION_H_
#define _OQUAM_SECTION_H_

#include <r.h>

/**
 *  \brief A section represents a component of the longer path that is
 *  traveled with a constant acceleration (including zero
 *  acceleration).
 *
 * The following the properties should be satisfied:
 *    v1 = v0 + a.duration
 *    d = p1 - p0
 *    d = v0.duration + a.duration²/2
 */
struct Section {
        // The absolute positions at the beginning and end of the
        // section in meters.
        double p0[3];
        double p1[3];

        // The duration of this section, if known.
        double duration;
        
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

        Section(double duration, double at,
                double *p0, double *p1,
                double *v0, double *v1,
                double *a);

        void get_position_at(double t_from_start, double *p);
        void get_speed_at(double t_from_start, double *v);
        
        bool is_valid(const char *name, double tmax,
                      double *xmin, double *xmax, 
                      double *vmax, double *amax);
        bool has_valid_start_time(const char *name);
        bool has_valid_duration(const char *name, double tmax);
        bool has_valid_positions(const char *name, double *xmin, double *xmax);
        bool has_valid_speeds(const char *name, double *vmax);
        bool has_valid_acceleration(const char *name, double *amax);

        list_t *slice(double interval, double max_duration);
        Section *compute_slice(double start_time, double dt);
        void print(membuf_t *text, const char *prefix);
};


#endif // _OQUAM_SECTION_H_
