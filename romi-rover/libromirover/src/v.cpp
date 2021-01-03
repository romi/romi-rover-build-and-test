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
#include <math.h>
#include <algorithm>
#include "v.h"

namespace romi {

        double vnorm(const double *v)
        {
                return sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
        }

        double *vabs(double *r, const double *a)
        {
                r[0] = fabs(a[0]);
                r[1] = fabs(a[1]);
                r[2] = fabs(a[2]);
                return r;
        }

        double *vcopy(double *r, const double *a)
        {
                r[0] = a[0];
                r[1] = a[1];
                r[2] = a[2];
                return r;
        }

        double *vzero(double *r)
        {
                r[0] = 0.0;
                r[1] = 0.0;
                r[2] = 0.0;
                return r;
        }

        double *vset(double *r, double v)
        {
                r[0] = v;
                r[1] = v;
                r[2] = v;
                return r;
        }

        double *sadd(double *w, const double *v, double s)
        {
                w[0] = v[0] + s;
                w[1] = v[1] + s;
                w[2] = v[2] + s;
                return w;
        }

        double *smul(double *w, const double *v, double s)
        {
                w[0] = v[0] * s;
                w[1] = v[1] * s;
                w[2] = v[2] * s;
                return w;
        }

        double *sdiv(double *w, const double *v, double s)
        {
                w[0] = v[0] / s;
                w[1] = v[1] / s;
                w[2] = v[2] / s;
                return w;
        }

        double vmax(const double *a)
        {
                double r = a[0];
                if (r < a[1])
                        r = a[1];
                if (r < a[2])
                        r = a[2];
                return r;
        }

        double vmin(const double *a)
        {
                double r = a[0];
                if (r > a[1])
                        r = a[1];
                if (r > a[2])
                        r = a[2];
                return r;
        }

        double *vadd(double *r, const double *a, const double *b)
        {
                r[0] = a[0] + b[0];
                r[1] = a[1] + b[1];
                r[2] = a[2] + b[2];
                return r;
        }

        double *vsub(double *r, const double *a, const double *b)
        {
                r[0] = a[0] - b[0];
                r[1] = a[1] - b[1];
                r[2] = a[2] - b[2];
                return r;
        }

        double *vmul(double *r, const double *a, const double *b)
        {
                r[0] = a[0] * b[0];
                r[1] = a[1] * b[1];
                r[2] = a[2] * b[2];
                return r;
        }

        double *vdiv(double *r, const double *a, const double *b)
        {
                r[0] = a[0] / b[0];
                r[1] = a[1] / b[1];
                r[2] = a[2] / b[2];
                return r;
        }

        double vdot(const double *a, const double *b)
        {
                return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
        }

        double *vcross(double *r, const double *a, const double *b)
        {
                r[0] =   a[1] * b[2] - a[2] * b[1];
                r[1] = -(a[0] * b[2] - a[2] * b[0]);
                r[2] =   a[0] * b[1] - a[1] * b[0];
                return r;
        }

        double *normalize(double *w, const double *v)
        {
                double L = vnorm(v);
                w[0] = v[0] / L;
                w[1] = v[1] / L;
                w[2] = v[2] / L;
                return w;
        }

        bool veq(const double *a, const double *b)
        {
                return ((a[0] == b[0]) && (a[1] == b[1]) && (a[2] == b[2]));
        }

        double vdist(double *a, double *b)
        {
                double d[3];
                return vnorm(vsub(d, a, b));
        }

        bool vnear(double *a, double *b, double epsilon)
        {
                return vdist(a, b) <= epsilon;
        }

        double *vclamp(double *r, const double *v, const double *lo, const double *hi)
        {
                r[0] = std::clamp(v[0], lo[0], hi[0]);
                r[1] = std::clamp(v[1], lo[1], hi[1]);
                r[2] = std::clamp(v[2], lo[2], hi[2]);
                return r;
        }
}
