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

double *smul(double *w, double *v, double s);
double *sdiv(double *w, double *v, double s);

double *vadd(double *r, double *a, double *b);
double *vsub(double *r, double *a, double *b);
double *vmul(double *r, double *a, double *b);
double *vdiv(double *r, double *a, double *b);

double vmax(double *a);
double vmin(double *a);

double *vabs(double *r, double *a);
double *vcopy(double *r, double *a);
double *vzero(double *r);
double *vset(double *r, double v);

double norm(double *v);
double *normalize(double *w, double *v);

int *vaddi(int *r, int *a, int *b);
int *vsubi(int *r, int *a, int *b);

int *vconvfi(int *r, double *a);
double *vconvif(double *r, int *a);

        
#ifdef __cplusplus
}
#endif

#endif // _OQUAM_V_H_
