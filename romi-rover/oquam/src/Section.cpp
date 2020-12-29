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


        static bool is_nan(const char *name, const char *param, double *value, int index);
        static bool is_out_of_range(const char *name, const char *param, double value,
                                 double minimum, double maximum, double epsilon);
        static bool is_valid_position(const char *name, const char *param,
                                      double *p, const double *xmin, const double *xmax);
        static bool is_valid_vector(const char *name, const char *param, double *v, const double *vmax);


        Section::Section()
        {
                zero();
        }
        
        Section::Section(double duration_, double at_,
                         const double *p0_, const double *p1_,
                         const double *v0_, const double *v1_,
                         const double *a_)
        {
                duration = duration_;
                at = at_;
                vcopy(p0, p0_);
                vcopy(p1, p1_);
                vcopy(v0, v0_);
                vcopy(v1, v1_);
                vcopy(a, a_);
                vsub(d, p1, p0);
        }

        void Section::zero()
        {
                duration = 0.0;
                at = 0.0;
                vzero(p0);
                vzero(p1);
                vzero(v0);
                vzero(v1);
                vzero(a);
                vzero(d);
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

        void Section::compute_slice(std::vector<Section>& slices, double offset, double slice_duration)
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
                
                slices.push_back(Section(slice_duration, at + offset,
                                         slice_p0, slice_p1,
                                         slice_v0, slice_v1, a));
        }

        void Section::slice(std::vector<Section>& slices, double interval, double max_duration)
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

        void Section::print(membuf_t *text, const char *prefix)
        {
                membuf_printf(text, "%s: ", prefix);
                membuf_printf(text, "start=%0.6f, duration=%0.6f; ", at, duration);
                membuf_printf(text, "from=(%0.4f,%0.4f,%0.4f), ", p0[0], p0[1], p0[2]);
                membuf_printf(text, "to=(%0.4f,%0.4f,%0.4f), ", p1[0], p1[1], p1[2]);
                membuf_printf(text, "distance=(%0.4f,%0.4f,%0.4f), ", d[0], d[1], d[2]);
                membuf_printf(text, "v0=(%0.3f,%0.3f,%0.3f), ", v0[0], v0[1], v0[2]);
                membuf_printf(text, "v1=(%0.3f,%0.3f,%0.3f), ", v1[0], v1[1], v1[2]);
                membuf_printf(text, "a=(%0.3f,%0.3f,%0.3f)", a[0], a[1], a[2]);
                membuf_printf(text, "\n");
        }

        void Section::print(const char *prefix)
        {
                printf("%s: ", prefix);
                printf("start=%0.6f, duration=%0.6f; ", at, duration);
                printf("from=(%0.4f,%0.4f,%0.4f), ", p0[0], p0[1], p0[2]);
                printf("to=(%0.4f,%0.4f,%0.4f), ", p1[0], p1[1], p1[2]);
                printf("distance=(%0.4f,%0.4f,%0.4f), ", d[0], d[1], d[2]);
                printf("v0=(%0.3f,%0.3f,%0.3f), ", v0[0], v0[1], v0[2]);
                printf("v1=(%0.3f,%0.3f,%0.3f), ", v1[0], v1[1], v1[2]);
                printf("a=(%0.3f,%0.3f,%0.3f)", a[0], a[1], a[2]);
                printf("\n");
        }

        bool Section::is_valid(const char *name, double tmax,
                               const double *xmin, const double *xmax, 
                               const double *vmax, const double *amax)
        {
                return (has_valid_start_time(name)
                        && has_valid_duration(name, tmax)
                        && has_valid_positions(name, xmin, xmax)
                        && has_valid_speeds(name, vmax)
                        && has_valid_acceleration(name, amax)
                        && is_coherent(name));
        }

        bool Section::has_valid_start_time(const char *name)
        {
                bool valid = false;
                if (at < 0.0) {
                        r_warn("Section (%s): at < 0", name);
                } else {
                        valid = true;
                }
                return valid;
        }

        bool Section::has_valid_duration(const char *name, double tmax)
        {
                bool valid = true;
                if (is_out_of_range(name, "duration", duration, 0.0, tmax, 0.0)) {
                        valid = false;
                }
                return valid;
        }

        bool Section::has_valid_positions(const char *name, const double *xmin, const double *xmax)
        {
                return (is_valid_position(name, "p0", p0, xmin, xmax)
                        && is_valid_position(name, "p1", p1, xmin, xmax));
        }

        bool Section::has_valid_speeds(const char *name, const double *vmax)
        {
                return (is_valid_vector(name, "v0", v0, vmax)
                        && is_valid_vector(name, "v1", v1, vmax));
        }

        bool Section::has_valid_acceleration(const char *name, const double *amax)
        {
                return is_valid_vector(name, "a", a, amax);
        }

        bool Section::is_coherent(const char *name)
        {
                return has_coherent_distance(name)
                        && has_coherent_acceleration_1(name)
                        && has_coherent_acceleration_2(name);
        }

        bool Section::has_coherent_distance(const char *name)
        {
                double e[3];
                vsub(e, p1, p0);
                vsub(e, d, e);
                bool ok = norm(e) < 0.001;
                if (!ok) {
                        r_warn("Section (%s): |(p1-p0)-d|>0.001", name);
                }
                return ok;
        }

        bool Section::has_coherent_acceleration_1(const char *name)
        {
                double dv[3];
                smul(dv, a, duration);
                
                double v[3];
                vadd(v, v0, dv);
                
                double e[3];
                vsub(e, v, v1);
                
                bool ok = norm(e) < 0.001;
                if (!ok) {
                        r_warn("Section (%s): |(v0+a.t)-v1|>0.001", name);
                        r_warn("Section (%s): v1=(%f,%f,%f), v0+a.t=(%f,%f,%f)",
                               name, v1[0], v1[1], v1[2], v[0], v[1], v[2]);
                }
                return ok;
        }

        bool Section::has_coherent_acceleration_2(const char *name)
        {
                double dx_v[3];
                smul(dx_v, v0, duration);
                
                double dx_a[3];
                smul(dx_a, a, 0.5 * duration * duration);
                
                double dx[3];
                vadd(dx, dx_v, dx_a);

                double p[3];
                vadd(p, p0, dx);
                
                double e[3];
                vsub(e, p, p1);
                
                bool ok = norm(e) < 0.001;
                if (!ok) {
                        r_warn("Section (%s): |(p0+v0.t+a.t²/2)-p1|>0.001", name);
                        r_warn("Section (%s): p1=(%f,%f,%f), p=(%f,%f,%f)",
                               name, p1[0], p1[1], p1[2], p[0], p[1], p[2]);
                }
                return ok;
        }


        static bool is_nan(const char *name, const char *param, double *value, int index)
        {
                bool nan = false;
                if (::isnan(value[index])) {
                        r_warn("Section (%s): %s[%d] is NaN", name, param, index);
                        nan = true;
                }
                return nan;
        }

        static bool is_out_of_range(const char *name, const char *param, double value,
                                 double minimum, double maximum, double epsilon)
        {
                bool out_of_range = true;
                if (value < minimum - epsilon
                    || value > maximum + epsilon) {
                        r_warn("Section (%s): %s is out of range: %.6f < value=%.6f < %.6f",
                               name, param, minimum, value, maximum);
                } else {
                        out_of_range = false;
                }
                return out_of_range;
        }

        static bool is_valid_position(const char *name, const char *param,
                                      double *p, const double *xmin, const double *xmax)
        {
                for (int i = 0; i < 3; i++) {
                        if (is_nan(name, param, p, i)) {
                                return false;
                        } else if (is_out_of_range(name, param, p[i], xmin[i], xmax[i], 0.001)) {
                                return false;
                        }
                }
                return true;
        }

        static bool is_valid_vector(const char *name, const char *param, double *v, const double *vmax)
        {
                for (int i = 0; i < 3; i++) {
                        if (is_nan(name, param, v, i)) {
                                return false;
                        } else if (is_out_of_range(name, param, v[i], -vmax[i], vmax[i], 0.01)) {
                                return false;
                        }
                }
                return true;
        }
}
