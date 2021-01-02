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

#include "Section.h"
#include "v.h"

namespace romi {


        Section::Section()
        {
                zero();
        }
        
        Section::Section(double duration_, double start_time_,
                         const double *p0_, const double *p1_,
                         const double *v0_, const double *v1_,
                         const double *a_)
        {
                set(duration_, start_time_, p0_, p1_, v0_, v1_, a_);
        }
        
        void Section::set(double duration_, double start_time_,
                          const double *p0_, const double *p1_,
                          const double *v0_, const double *v1_,
                          const double *a_)
        {
                duration = duration_;
                start_time = start_time_;
                vcopy(p0, p0_);
                vcopy(p1, p1_);
                vcopy(v0, v0_);
                vcopy(v1, v1_);
                vcopy(a, a_);
        }

        void Section::zero()
        {
                double _zero[3] = {0,0,0};
                set(0, 0, _zero, _zero, _zero, _zero, _zero);
        }

        void Section::get_position_at(double t, double *p)
        {
                double dx_v[3];
                double dx_a[3];
                double dx[3];

                smul(dx_v, v0, t);
                smul(dx_a, a, 0.5 * t * t);
                vadd(dx, dx_v, dx_a);
                vadd(p, p0, dx);
        }

        void Section::get_speed_at(double t, double *v)
        {
                double dv[3];
                smul(dv, a, t);
                vadd(v, v0, dv);
        }

        double Section::end_time()
        {
                return start_time + duration;
        }

        void Section::compute_slice(std::vector<Section>& slices,
                                    double offset, double slice_duration)
        {
                double t0 = offset;
                double t1 = offset + slice_duration;
        
                double slice_p0[3];
                double slice_p1[3];
                double slice_v0[3];
                double slice_v1[3];
        
                get_position_at(t0, slice_p0);
                get_position_at(t1, slice_p1);
                get_speed_at(t0, slice_v0);
                get_speed_at(t1, slice_v1);
                
                slices.push_back(Section(slice_duration,
                                         start_time + offset,
                                         slice_p0, slice_p1,
                                         slice_v0, slice_v1, a));
        }

        void Section::slice(std::vector<Section>& slices, double interval,
                            double max_duration)
        {
                double offset = 0.0;
                double used_interval = interval;
        
                /* The segment has a constant speed there is no need to sample
                 * the speed and position at small intervals. We can therefore
                 * sample at speeds and positions at 'max_duration' instead of
                 * 'interval'. */
                if (norm(a) == 0)
                        used_interval = max_duration;

                while (offset < duration) {
                        double slice_duration = duration - offset;
                        if (slice_duration > used_interval)
                                slice_duration = used_interval;
                        compute_slice(slices, offset, slice_duration);
                        offset += slice_duration;
                }
        }

        double *Section::displacement(double *d)
        {
                vsub(d, p1, p0);
                return d;
        }

        double Section::length()
        {
                double d[3];
                displacement(d);
                return norm(d);
        }

        double *Section::direction(double *d)
        {
                displacement(d);
                normalize(d, d);
                return d;
        }
}
