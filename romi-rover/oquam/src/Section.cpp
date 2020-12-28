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

Section::Section(double t_, double at_,
                 double *p0_, double *p1_,
                 double *v0_, double *v1_,
                 double *a_)
{
        duration = t_;
        at = at_;
        vcopy(p0, p0_);
        vcopy(p1, p1_);
        vcopy(v0, v0_);
        vcopy(v1, v1_);
        vcopy(a, a_);
        vsub(d, p1, p0);
}

list_t *Section::slice(double period, double maxlen)
{
        list_t *slices = NULL;
        double tmp[3];        
        double T = period;
        
        /* The segment has a constant speed. Sample at distances
         * 'maxlen' instead of 'period'. */
        if (norm(a) == 0)
                T = maxlen;

        double elapsed_time = 0.0;

        //r_debug("t=%f", t);
        
        while (elapsed_time < duration) {
                double dt = duration - elapsed_time;
                if (dt > T)
                        dt = T;
                
                double v0[3];
                smul(v0, a, elapsed_time);
                vadd(v0, v0, v0);
                
                double p0[3];
                smul(tmp, v0, elapsed_time);
                smul(p0, a, 0.5 * elapsed_time * elapsed_time);
                vadd(p0, p0, tmp);
                vadd(p0, p0, p0);
                
                double v1[3];
                smul(v1, a, elapsed_time+dt);
                vadd(v1, v0, v1);
                
                double p1[3];
                smul(tmp, v0, elapsed_time+dt);
                smul(p1, a, 0.5 * (elapsed_time+dt) * (elapsed_time+dt));
                vadd(p1, p1, tmp);
                vadd(p1, p1, p0);
                
                Section *s = new Section(dt, at + elapsed_time,
                                         p0, p1, v0, v1, a);

                slices = list_append(slices, s);
                
                elapsed_time += dt;
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
        if (duration > tmax) {
                r_warn("Section::is_valid (%s): t too long", name);
                return false;
        }
        if (duration < 0.0) {
                r_warn("Section::is_valid (%s): t < 0", name);
                return false;
        }
        if (at < 0.0) {
                r_warn("Section::is_valid (%s): at < 0", name);
                return false;
        }
        
        double dx[3];
        double dmax;

        vsub(dx, xmax, xmin);
        dmax = norm(dx);
        
        for (int i = 0; i < 3; i++) {
                if (isnan(p0[i])) {
                        r_warn("Section::is_valid (%s): p0[%d] is NaN", name, i);
                        return false;
                }
                if (isnan(p1[i])) {
                        r_warn("Section::is_valid (%s): p1[%d] is NaN", name, i);
                        return false;
                }
                if (isnan(v0[i])) {
                        r_warn("Section::is_valid (%s): v0[%d] is NaN", name, i);
                        return false;
                }
                if (isnan(v1[i])) {
                        r_warn("Section::is_valid (%s): v1[%d] is NaN", name, i);
                        return false;
                }
                if (isnan(a[i])) {
                        r_warn("Section::is_valid (%s): a[%d] is NaN", name, i);
                        return false;
                }
                if (isnan(d[i])) {
                        r_warn("Section::is_valid (%s): d[%d] is NaN", name, i);
                        return false;
                }
                
                if (p0[i] < xmin[i] - 0.001) {
                        r_warn("Section::is_valid (%s): p0[%d] is too small: %.12f < %.12f",
                               name, i, p0[i], xmin[i]);
                        return false;
                }
                if (p0[i] > xmax[i] + 0.001) {
                        r_warn("Section::is_valid (%s): p0[%d] is too large: %.12f > %.12f",
                               name, i, p0[i], xmax[i]);
                        return false;
                }
                if (p1[i] < xmin[i] - 0.001) {
                        r_warn("Section::is_valid (%s): p1[%d] is too small: %.12f < %.12f",
                               name, i, p1[i], xmin[i]);
                        return false;
                }
                if (p1[i] > xmax[i] + 0.001) {
                        r_warn("Section::is_valid (%s): p1[%d] is too large: %.12f > %.12f",
                               name, i, p1[i], xmax[i]);
                        return false;
                }
                if (fabs(v0[i]) > vmax[i] + 0.01) {
                        r_warn("Section::is_valid (%s): v0[%d] is too large: %.12f > %.12f",
                               name, i, fabs(v0[i]), vmax[i]);
                        return false;
                }
                if (fabs(v1[i]) > vmax[i] + 0.01) {
                        r_warn("Section::is_valid (%s): v1[%d] is too large: %.12f > %.12f",
                               name, i, fabs(v1[i]), vmax[i]);
                        return false;
                }
                if (fabs(a[i]) > amax[i] + 0.001) {
                        r_warn("Section::is_valid (%s): a[%d] is too large: %.12f > %.12f",
                               name, i, fabs(a[i]), amax[i]);
                        return false;
                }
                if (fabs(d[i]) > dmax + 0.01) {
                        r_warn("Section::is_valid (%s): d[%d] is too large: %.12f > %.12f",
                               name, i, fabs(d[i]), dmax);
                        return false;
                }
        }

        double v = norm(vmax); 
        double length_amax = norm(amax);
        if (norm(v0) + 0.0001 > v) {
                r_warn("Section::is_valid (%s): v0 is too large");
                return false;
        }
        if (norm(v1) + 0.0001 > v) {
                r_warn("Section::is_valid (%s): v1 is too large");
                return false;
        }
        if (norm(a) + 0.0001 > length_amax) {
                r_warn("Section::is_valid (%s): a is too large");
                return false;
        }
        
        return true;
}
