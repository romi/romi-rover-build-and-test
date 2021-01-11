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

#include "oquam/print.h"
#include "oquam/is_valid.h"

namespace romi {

        static bool is_nan(const char *name, const char *param, double *value, int index)
        {
                bool nan = false;
                if (::isnan(value[index])) {
                        r_warn("is_valid: section (%s): %s[%d] is NaN", name, param, index);
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
                        r_warn("is_valid: section (%s): %s is out of range: "
                               "%.6f < value=%.6f < %.6f",
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
                        } else if (is_out_of_range(name, param, p[i],
                                                   xmin[i], xmax[i], 0.001)) {
                                return false;
                        }
                }
                return true;
        }

        static bool is_valid_vector(const char *name, const char *param,
                                    double *v, const double *vmax)
        {
                for (int i = 0; i < 3; i++) {
                        if (is_nan(name, param, v, i)) {
                                return false;
                        } else if (is_out_of_range(name, param, v[i],
                                                   -vmax[i], vmax[i], 0.01)) {
                                return false;
                        }
                }
                return true;
        }

        static bool has_valid_positions(Section& section, const char *name,
                                        const double *xmin, const double *xmax)
        {
                return (is_valid_position(name, "p0", section.p0, xmin, xmax)
                        && is_valid_position(name, "p1", section.p1, xmin, xmax));
        }

        static bool has_valid_speeds(Section& section, const char *name,
                                     const double *vmax)
        {
                return (is_valid_vector(name, "v0", section.v0, vmax)
                        && is_valid_vector(name, "v1", section.v1, vmax));
        }

        static bool has_valid_acceleration(Section& section, const char *name,
                                           const double *amax)
        {
                return is_valid_vector(name, "a", section.a, amax);
        }

        static bool has_coherent_acceleration_1(Section& section, const char *name)
        {
                double dv[3];
                smul(dv, section.a, section.duration);
                
                double v[3];
                vadd(v, section.v0, dv);
                
                double e[3];
                vsub(e, v, section.v1);
                
                bool ok = vnorm(e) < 0.001;
                if (!ok) {
                        r_warn("is_valid: section (%s): |(v0+a.t)-v1|>0.001", name);
                        r_warn("is_valid: section (%s): v1=(%f,%f,%f), v0+a.t=(%f,%f,%f)",
                               name,
                               section.v1[0], section.v1[1], section.v1[2],
                               v[0], v[1], v[2]);
                }
                return ok;
        }

        static bool has_coherent_acceleration_2(Section& section, const char *name)
        {
                double dx_v[3];
                smul(dx_v, section.v0, section.duration);
                
                double dx_a[3];
                smul(dx_a, section.a, 0.5 * section.duration * section.duration);
                
                double dx[3];
                vadd(dx, dx_v, dx_a);

                double p[3];
                vadd(p, section.p0, dx);
                
                double e[3];
                vsub(e, p, section.p1);
                
                bool ok = vnorm(e) < 0.001;
                if (!ok) {
                        r_warn("is_valid: section (%s): |(p0+v0.t+a.t²/2)-p1|>0.001", name);
                        r_warn("is_valid: section (%s): p1=(%f,%f,%f), p=(%f,%f,%f)",
                               name,
                               section.p1[0], section.p1[1], section.p1[2],
                               p[0], p[1], p[2]);
                }
                return ok;
        }

        static bool is_coherent(Section& section, const char *name)
        {
                return has_coherent_acceleration_1(section, name)
                        && has_coherent_acceleration_2(section, name);
        }

        static bool has_valid_start_time(Section& section, const char *name)
        {
                bool valid = false;
                if (section.start_time < 0.0) {
                        r_warn("is_valid: section (%s): start_time=%f < 0",
                               name, section.start_time);
                } else {
                        valid = true;
                }
                return valid;
        }

        static bool has_valid_duration(Section& section, const char *name, double tmax)
        {
                bool valid = true;
                if (is_out_of_range(name, "duration", section.duration, 0.0, tmax, 0.0)) {
                        r_warn("is_valid: section (%s): duration=%f > max=%f",
                               name, section.duration, tmax);
                        valid = false;
                }
                return valid;
        }
        
        bool is_valid(Section& section, const char *name, double tmax,
                      const double *xmin, const double *xmax, 
                      const double *vmax, const double *amax)
        {
                return (has_valid_start_time(section, name)
                        && has_valid_duration(section, name, tmax)
                        && has_valid_positions(section, name, xmin, xmax)
                        && has_valid_speeds(section, name, vmax)
                        && has_valid_acceleration(section, name, amax)
                        && is_coherent(section, name));
        }

        static bool points_and_speeds_match(Section& first, Section& second,
                                            const char *first_name,
                                            const char *second_name)
        {
                bool match = (vnear(first.p1, second.p0, 0.001)
                              && vnear(first.v1, second.v0, 0.001));
                if (!match) {
                        r_err("is_valid: consecutive points and speeds don't match: "
                              "%s -> %s", first_name, second_name);
                }
                return match;
        }

        static bool points_and_speeds_match(ATDC& atdc)
        {
                bool match = true;
                if (atdc.prev) {
                        match = points_and_speeds_match(atdc.prev->curve, atdc.accelerate,
                                                        "curve", "accelerate");
                } 
                if (match) {
                        match = (points_and_speeds_match(atdc.accelerate, atdc.travel,
                                                         "accelerate", "travel")
                                 && points_and_speeds_match(atdc.travel, atdc.decelerate,
                                                            "travel", "decelerate")
                                 && points_and_speeds_match(atdc.decelerate, atdc.curve,
                                                            "decelerate", "curve"));
                }
                return match;
        }
        
        bool is_valid(ATDC& atdc, double tmax,
                      double *xmin, double *xmax, 
                      double *vmax, double *amax)
        {
                return (is_valid(atdc.accelerate, "accelerate", tmax, xmin, xmax, vmax, amax)
                        && is_valid(atdc.travel, "travel", tmax, xmin, xmax, vmax, amax)
                        && is_valid(atdc.decelerate, "decelerate", tmax, xmin, xmax, vmax, amax)
                        && is_valid(atdc.curve, "curve", tmax, xmin, xmax, vmax, amax)
                        && points_and_speeds_match(atdc));
        }

        static bool start_positions_match(SmoothPath& script)
        {
                bool matches = true;
                if (script.count_atdc() > 0) {
                        v3 start_position = script.get_start_position();
                        matches = veq(start_position.values(),
                                      script.get_atdc(0).accelerate.p0);
                }
                if (!matches) {
                        r_warn("is_valid: start positions don't match");
                }
                return matches;
        }
        
        bool is_valid(SmoothPath& script, double tmax,
                      double *xmin, double *xmax, 
                      double *vmax, double *amax)
        {
                bool valid = true;
                for (size_t i = 0; i < script.count_atdc(); i++) {
                        if (!is_valid(script.get_atdc(i), tmax, xmin, xmax, vmax, amax)) {
                                valid = false;
                                r_warn("is_valid: invalid atdc: atdc %d", i);
                                print(script.get_atdc(i));
                        }
                }
                if (valid) {
                        valid = start_positions_match(script);
                }
                return valid;
        }

        bool is_valid(SmoothPath& script, double tmax, CNCRange& range, 
                      double *vmax, double *amax)
        {
                return is_valid(script, tmax, range.min.values(),
                                range.max.values(), vmax, amax);
        }
        

}
