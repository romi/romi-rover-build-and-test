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

#ifndef _OQUAM_V_H_
#define _OQUAM_V_H_

#ifdef __cplusplus
extern "C" {
#endif

double *smul(double *w, const double *v, double s);
double *sdiv(double *w, const double *v, double s);
double *sadd(double *w, const double *v, double s);

double *vadd(double *r, const double *a, const double *b);
double *vsub(double *r, const double *a, const double *b);
double *vmul(double *r, const double *a, const double *b);
double *vdiv(double *r, const double *a, const double *b);
double *vcross(double *r, const double *a, const double *b);

double vmax(const double *a);
double vmin(const double *a);

double *vabs(double *r, const double *a);
double *vsqrt(double *r, const double *a);
double *vcopy(double *r, const double *a);
double *vzero(double *r);
double *vset(double *r, double v);

double norm(const double *v);
double *normalize(double *w, const double *v);

int *vaddi(int *r, const int *a, const int *b);
int *vsubi(int *r, const int *a, const int *b);

int *vconvfi(int *r, const double *a);
double *vconvif(double *r, const int *a);

int veq(const double *a, const double *b);

#ifdef __cplusplus
}
#endif

#endif // _OQUAM_V_H_
