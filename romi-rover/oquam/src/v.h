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


#ifdef __cplusplus

#include <math.h>

namespace oquam {

        class V3
        {
        public:
                double _v[3];

                V3() {
                        _v[0] = _v[1] = _v[2] = 0.0;
                }

                V3(double a) {
                        _v[0] = a;
                        _v[1] = a;
                        _v[2] = a;
                }

                V3(double a, double b, double c) {
                        _v[0] = a;
                        _v[1] = b;
                        _v[2] = c;
                }

                V3(double *a) {
                        _v[0] = a[0];
                        _v[1] = a[1];
                        _v[2] = a[2];
                }
        
                V3(const V3 &a) {
                        _v[0] = a._v[0];
                        _v[1] = a._v[1];
                        _v[2] = a._v[2];
                }

                V3 operator+(V3 const &a) { 
                        V3 r; 
                        r._v[0] = _v[0] + a._v[0]; 
                        r._v[1] = _v[1] + a._v[1]; 
                        r._v[2] = _v[2] + a._v[2]; 
                        return r; 
                } 

                V3 operator-(V3 const &a) { 
                        V3 r; 
                        r._v[0] = _v[0] - a._v[0]; 
                        r._v[1] = _v[1] - a._v[1]; 
                        r._v[2] = _v[2] - a._v[2]; 
                        return r; 
                } 
        
                V3 operator*(V3 const &a) { 
                        V3 r; 
                        r._v[0] = _v[0] * a._v[0]; 
                        r._v[1] = _v[1] * a._v[1]; 
                        r._v[2] = _v[2] * a._v[2]; 
                        return r; 
                } 

                V3 operator/(V3 const &a) { 
                        V3 r; 
                        r._v[0] = _v[0] / a._v[0]; 
                        r._v[1] = _v[1] / a._v[1]; 
                        r._v[2] = _v[2] / a._v[2]; 
                        return r; 
                } 
        
                V3 operator+(double s) { 
                        V3 r; 
                        r._v[0] = _v[0] + s;
                        r._v[1] = _v[1] + s;
                        r._v[2] = _v[2] + s;
                        return r; 
                } 

                V3 operator-(double s) { 
                        V3 r; 
                        r._v[0] = _v[0] - s;
                        r._v[1] = _v[1] - s;
                        r._v[2] = _v[2] - s;
                        return r; 
                } 

                V3 operator*(double s) { 
                        V3 r; 
                        r._v[0] = _v[0] * s;
                        r._v[1] = _v[1] * s;
                        r._v[2] = _v[2] * s;
                        return r; 
                } 

                V3 operator/(double s) { 
                        V3 r; 
                        r._v[0] = _v[0] / s;
                        r._v[1] = _v[1] / s;
                        r._v[2] = _v[2] / s;
                        return r; 
                } 

                bool operator==(const V3& v) {
                        return (_v[0] == v._v[0]
                                && _v[1] == v._v[1]
                                && _v[2] == v._v[2]);
                }

                V3 abs() { 
                        V3 r; 
                        r._v[0] = fabs(_v[0]);
                        r._v[1] = fabs(_v[1]);
                        r._v[2] = fabs(_v[2]);
                        return r; 
                } 

                V3 sqrt() { 
                        V3 r; 
                        r._v[0] = ::sqrt(_v[0]);
                        r._v[1] = ::sqrt(_v[1]);
                        r._v[2] = ::sqrt(_v[2]);
                        return r; 
                } 

                double max() { 
                        double r = _v[0];
                        if (_v[1] > r)
                                r = _v[1];
                        if (_v[2] > r)
                                r = _v[2];
                        return r; 
                } 

                double min() { 
                        double r = _v[0];
                        if (_v[1] < r)
                                r = _v[1];
                        if (_v[2] < r)
                                r = _v[2];
                        return r; 
                } 

                double norm() { 
                        double r = _v[0] * _v[0];
                        r += _v[1] * _v[1]; 
                        r += _v[2] * _v[2]; 
                        return ::sqrt(r); 
                } 

                V3 normalize() { 
                        V3 r = *this; 
                        r = r / norm();
                        return r;
                } 

                double *ptr() { 
                        return _v;
                } 

        };
}

#endif

#endif // _OQUAM_V_H_
