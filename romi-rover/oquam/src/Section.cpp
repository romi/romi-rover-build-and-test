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


static bool is_nan(const char *name, const char *param, double *value, int index);
static bool out_of_range(const char *name, const char *param, double value,
                         double minimum, double maximum, double epsilon);
static bool is_valid_position(const char *name, const char *param,
                              double *p, double *xmin, double *xmax);
static bool is_valid_vector(const char *name, const char *param, double *v, double *vmax);


Section::Section(double duration_, double at_,
                 double *p0_, double *p1_,
                 double *v0_, double *v1_,
                 double *a_)
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

Section *Section::compute_slice(double offset, double dt)
{
        double tmp[3];
        
        double v0_[3];
        smul(v0_, a, offset);
        vadd(v0_, v0, v0_);
                
        double p0_[3];
        smul(tmp, v0, offset);
        smul(p0_, a, 0.5 * offset * offset);
        vadd(p0_, p0_, tmp);
        vadd(p0_, p0_, p0);
                
        double v1_[3];
        smul(v1_, a, offset+dt);
        vadd(v1_, v0, v1_);
                
        double p1_[3];
        smul(tmp, v0, offset+dt);
        smul(p1_, a, 0.5 * (offset + dt) * (offset + dt));
        vadd(p1_, p1_, tmp);
        vadd(p1_, p1_, p0);
                
        return new Section(dt, at + offset, p0_, p1_, v0_, v1_, a);
}

list_t *Section::slice(double period, double maxlen)
{
        list_t *slices = NULL;
        double T = period;
        
        /* The segment has a constant speed. Sample at distances
         * 'maxlen' instead of 'period'. */
        if (norm(a) == 0)
                T = maxlen;

        double offset = 0.0;

        //r_debug("t=%f", t);
        
        while (offset < duration) {
                double dt = duration - offset;
                if (dt > T)
                        dt = T;

                Section *s = compute_slice(offset, dt);
                slices = list_append(slices, s);
                
                offset += dt;
        }
        
        //r_err("section_slice: return %p", slices);
        return slices;
}

void Section::print(membuf_t *text, const char *prefix)
{
        membuf_printf(text, "%s: ", prefix);
        membuf_printf(text, "at=%0.6f, t=%0.6f; ", at, duration);
        membuf_printf(text, "d=(%0.4f,%0.4f,%0.4f); ",
                      d[0], d[1], d[2]);
        membuf_printf(text, "p0(%0.4f,%0.4f,%0.4f)-p1(%0.4f,%0.4f,%0.4f); ",
                      p0[0], p0[1], p0[2],
                      p1[0], p1[1], p1[2]);
        membuf_printf(text, "v0(%0.3f,%0.3f,%0.3f)-v1(%0.3f,%0.3f,%0.3f); ",
                      v0[0], v0[1], v0[2],
                      v1[0], v1[1], v1[2]);
        membuf_printf(text, "a(%0.3f,%0.3f,%0.3f)",
               a[0], a[1], a[2]);
        membuf_printf(text, "\n");
}

bool Section::is_valid(const char *name, double tmax,
                       double *xmin, double *xmax, 
                       double *vmax, double *amax)
{
        return (has_valid_start_time(name)
                && has_valid_duration(name, tmax)
                && has_valid_positions(name, xmin, xmax)
                && has_valid_speeds(name, vmax)
                && has_valid_acceleration(name, amax));
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
        if (out_of_range(name, "duration", duration, 0.0, tmax, 0.0)) {
                valid = false;
        }
        return valid;
}

bool Section::has_valid_positions(const char *name, double *xmin, double *xmax)
{
        return (is_valid_position(name, "p0", p0, xmin, xmax)
                && is_valid_position(name, "p1", p1, xmin, xmax));
}

bool Section::has_valid_speeds(const char *name, double *vmax)
{
        return (is_valid_vector(name, "v0", v0, vmax)
                && is_valid_vector(name, "v1", v1, vmax));
}

bool Section::has_valid_acceleration(const char *name, double *amax)
{
        return is_valid_vector(name, "a", a, amax);
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

static bool out_of_range(const char *name, const char *param, double value,
                         double minimum, double maximum, double epsilon)
{
        bool in_range = false;
        if (value < minimum - epsilon
            || value > maximum + epsilon) {
                r_warn("Section (%s): %s is out of range: %.6f < %.6f < %.6f",
                       name, param, value, minimum, maximum);
        } else {
                in_range = true;
        }
        return in_range;
}

static bool is_valid_position(const char *name, const char *param,
                              double *p, double *xmin, double *xmax)
{
        for (int i = 0; i < 3; i++) {
                if (is_nan(name, param, p, i)) {
                        return false;
                } else if (out_of_range(name, param, p[i], xmin[i], xmax[i], 0.001)) {
                        return false;
                }
        }
        return true;
}

static bool is_valid_vector(const char *name, const char *param, double *v, double *vmax)
{
        for (int i = 0; i < 3; i++) {
                if (is_nan(name, param, v, i)) {
                        return false;
                } else if (out_of_range(name, param, v[i], -vmax[i], vmax[i], 0.01)) {
                        return false;
                }
        }
        return true;
}
