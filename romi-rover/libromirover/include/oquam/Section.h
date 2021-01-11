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
#include <vector>

namespace romi {

        /**
         *  A section represents a component of the longer path that is
         *  traveled with a constant acceleration (including zero
         *  acceleration).
         *
         *  The following the properties should be satisfied:
         *    v1 = v0 + a * duration
         *    p1 = p0 + v0 * duration + a * duration² / 2
         */
        class Section {
        public:
                // The absolute positions at the beginning and end of the
                // section in meters.
                double p0[3];
                double p1[3];

                // The duration of this section, if known.
                double duration;
        
                // The absolute start time of this section, if known. The
                // start time is measured from the beginning of the continuous
                // path to which this segment belongs.
                double start_time;

                // The speed at the start of this section, in m/s.
                double v0[3];

                // The speed at the end of this section. 
                double v1[3]; 

                // The acceleation of this section, in m/s².
                double a[3];

                Section();
                
                Section(double duration, double start_time,
                        const double *p0, const double *p1,
                        const double *v0, const double *v1,
                        const double *a);

                void set(double duration, double start_time,
                         const double *p0, const double *p1,
                         const double *v0, const double *v1,
                         const double *a);

                void slice(std::vector<Section>& slices, double interval,
                           double max_duration);
                void get_position_at(double t_from_start, double *p);
                void get_speed_at(double t_from_start, double *v);

                void zero();
                double length();
                double *direction(double *d);
                double *displacement(double *d);
                double end_time();

        protected:
                
                void compute_slice(std::vector<Section>& slices, double start_time, double dt);
        };
}

#endif // _OQUAM_SECTION_H_
