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

#ifndef __ROMI_SELF_ORGANIZED_MAP_H
#define __ROMI_SELF_ORGANIZED_MAP_H

#include <cmath>
#include "../IFolder.h"
#include "fixed.h"

namespace romi {

        //-----------------------------------------------------------
        
        static inline double radd(double const& a, double const& b) {
                return a + b;
        }
        
        static inline double rsub(double const& a, double const& b) {
                return a - b;
        }
        
        static inline double rmul(double const& a, double const& b) {
                return a * b;
        }
        
        static inline double rdiv(double const& a, double const& b) {
                return a / b;
        }
        
        static inline double rexp(double const& a) {
                return std::exp(a);
        }
        
        static inline double rsquare(double const& a) {
                return std::pow(a, 2.0);
        }
        
        static inline double dtor(double r, double a) { 
                (void)(r);
                return a;
        }
        
        static inline double rtod(double a) {
                return a;
        }
        
        static inline double itor(double r, int32_t a) {
                (void)(r);
                return (double) a;
        }
        
        static inline int32_t rtoi(double a) {
                return (int) a;
        }
        
        static inline double rneg(double a) {
                return -a;
        }
        

        //-----------------------------------------------------------
        
        static inline fixed_t radd(fixed_t const& a, fixed_t const& b) {
                return fxadd(a, b);
        }
        
        static inline fixed_t rsub(fixed_t const& a, fixed_t const& b) {
                return fxsub(a, b);
        }
        
        static inline fixed_t rmul(fixed_t const& a, fixed_t const& b) {
                return fxmul(a, b);
        }
        
        static inline fixed_t rdiv(fixed_t const& a, fixed_t const& b) {
                return fxdiv(a, b);
        }
        
        static inline fixed_t rexp(fixed_t const& a) {
                return std::exp(a);
        }
        
        static inline fixed_t rsquare(fixed_t const& a) {
                return rmul(a, a);
        }
        
        static inline fixed_t dtor(fixed_t r, double a) {
                (void)(r);
                return dtofx(a); 
        }
        
        static inline double rtod(fixed_t a) {
                return fxtod(a);
        }
        
        static inline fixed_t itor(fixed_t r, int32_t a) {
                (void)(r);
                return itofx(a);
        }
        
        static inline int32_t rtoi(fixed_t a) {
                return fxtoi(a); 
        }
        
        static inline fixed_t rneg(fixed_t a) {
                return -a;
        }
        
        
        template <typename T> 
        class SelfOrganizedMap
        {
        protected:
#define NE 4096
#define KE 8.0
#define NE_KE -512

                T _alpha;
                T _beta;
                T _epsilon;
                T _k;
                T _dmax2;
                T _exp[NE];
                int _n;
                T _ne_ke;


                int _num_cities;
                int _path_length;
                T *_cx;
                T *_cy;
                T *_px;
                T *_py;
                T *_dfx;
                T *_dfy;
                T *_tx;
                T *_ty;
                T *_dx;
                T *_dy;
                T *_d2;
                T *_w;
                T *_sum;
                T _two;

                void exp_init() {
                        for (int i = 0; i < NE; i++) {
                                double x = (double) -KE * (double) i / (double) NE;
                                double e = std::exp(x);
                                _exp[i] = dtor(_exp[i], e);
                        }
                        _ne_ke = itor(_ne_ke, NE_KE);
                }

                T exp_lookup(T x) {
                        T tmp = rmul(_ne_ke, x);
                        int32_t i = rtoi(tmp);
                        if (i < NE)
                                return _exp[i];
                        else return 0;
                }
                
                T exponential(T x) {
                        //return exp_lookup(x);
                        return rexp(x);
                }

                void update();
                void update_distance_forces();                
                void update_tension();
                void update_positions();
                
                void make_circle(T *cx, T *cy,
                                 std::vector<T> &px,
                                 std::vector<T> &py,
                                 double radius);

                void dump_path(IFolder *session) {
                        std::vector<double> x;
                        std::vector<double> y;
                        for (int i = 0; i < _path_length; i++) {
                                x.push_back(rtod(_px[i]));
                                y.push_back(rtod(_py[i]));
                        }
                        session->dump_interleave("path", _path_length, &x[0], &y[0]);
                }

                void print_path(IFolder *session, int n) {
                        std::vector<double> x;
                        std::vector<double> y;
                        for (int i = 0; i < _path_length; i++) {
                                x.push_back(rtod(_px[i]));
                                y.push_back(rtod(_py[i]));
                        }
                        session->print_path(&x[0], &y[0], _path_length, n);
                }

                void dump_cities(IFolder *session) {
                        std::vector<double> x;
                        std::vector<double> y;
                        for (int i = 0; i < _num_cities; i++) {
                                x.push_back(rtod(_cx[i]));
                                y.push_back(rtod(_cy[i]));
                        }
                        session->dump_interleave("cities", _num_cities, &x[0], &y[0]);
                }
                
        public:
                SelfOrganizedMap(int num_cities, int path_length,
                                 double alpha, double beta, double epsilon) {
                        exp_init();
                        fx_init();

                        _alpha = dtor(_alpha, alpha);
                        _beta = dtor(_beta, beta);
                        _epsilon = dtor(_epsilon, epsilon);

                        _num_cities = num_cities;
                        _path_length = path_length;

                        _cx = new T[num_cities];
                        _cy = new T[num_cities];
                        _px = new T[path_length];
                        _py = new T[path_length];
                        _dfx = new T[path_length];        
                        _dfy = new T[path_length];
                        _tx = new T[path_length];
                        _ty = new T[path_length];
                        _dx = new T[_num_cities * _path_length];
                        _dy = new T[_num_cities * _path_length];
                        _d2 = new T[_num_cities * _path_length];
                        _w = new T[_num_cities * _path_length];
                        _sum = new T[_num_cities];
                        _two = dtor(_two, 2.0);
                }
                
                virtual ~SelfOrganizedMap() {
                        delete[] _cx;
                        delete[] _cy;
                        delete[] _px;
                        delete[] _py;
                        delete[] _dfx;
                        delete[] _dfy;
                        delete[] _tx;
                        delete[] _ty;
                        delete[] _dx;
                        delete[] _dy;
                        delete[] _d2;
                        delete[] _w;
                        delete[] _sum;
                }

                void set_alpha(double alpha) {
                        _alpha = dtor(_alpha, alpha);
                }

                void set_beta(double beta) {
                        _beta = dtor(_beta, beta);
                }

                void set_epsilon(double epsilon) {
                        _epsilon = dtor(_epsilon, epsilon);
                }

                void init_cities(double *cx, double *cy) {
                        for (int i = 0; i < _num_cities; i++) {
                                _cx[i] = dtor(_cx[i], cx[i]);
                                _cy[i] = dtor(_cy[i], cy[i]);
                        }
                }

                void init_path(double *px, double *py) {
                        for (int i = 0; i < _path_length; i++) {
                                _px[i] = dtor(_px[i], px[i]);
                                _py[i] = dtor(_py[i], py[i]);
                        }
                }

                void make_circle(double radius) {
                        double x0 = 0.0; 
                        double y0 = 0.0;
                        for (int i = 0; i < _num_cities; i++) {
                                x0 += rtod(_cx[i]); 
                                y0 += rtod(_cy[i]);
                        }
                        x0 /= (double) _num_cities;
                        y0 /= (double) _num_cities;

                        for (int i = 0; i < _path_length; i++) {
                                double theta = ((double) i * 2.0 * M_PI
                                                / (double) _path_length);
                                double x = x0 + radius * std::cos(theta); 
                                double y = y0 + radius * std::sin(theta);
                                _px[i] = dtor(_px[i], x); 
                                _px[i] = dtor(_px[i], y);
                        }
                }

                void get_path(Path &path) {
                        for (int i = 0; i < _path_length; i++) {
                                double x = rtod(_px[i]);
                                double y = rtod(_py[i]);
                                Waypoint p(x, y);
                                path.push_back(p);
                        }
                }
                
                bool compute_path(IFolder *session, bool print = false);
        };


        template <typename T>
        bool SelfOrganizedMap<T>::compute_path(IFolder *session, bool print)
        {
                int N = 10000;

                _k = 0.2;
                _dmax2 = 10000.0;

                dump_cities(session);
                dump_path(session);

                T eps2 = rmul(_epsilon, _epsilon);
                
                for (_n = 1; _n < N; _n++) {
                        update();

                        dump_path(session);
                        if (print) print_path(session, _n);
                        
                        if (_dmax2 < eps2)
                                break;
                }
                
                bool success = (_dmax2 < eps2);

                r_info("SelfOrganizedMap: n=%d, success=%s\n", _n, success? "yes":"no");
                
                return success;
        }
        
        template <typename T>
        void SelfOrganizedMap<T>::update()
        {
                update_distance_forces();
                update_tension();        

                if ((_n % 25) == 0) {
                        T KA = 0;
                        KA = dtor(KA, 0.99);
                        T threshold = 0;
                        threshold = dtor(threshold, 0.01);
                        T newk = rmul(_k, KA);
                        if (newk < threshold)
                                _k= threshold;
                        else _k = newk;
                }

                update_positions();
        }

        template <typename T>
        void SelfOrganizedMap<T>::update_distance_forces()
        {
                T coeff = rmul(_two, rsquare(_k));

                int index = 0;
                for (int city = 0; city < _num_cities; city++)  {
                        for (int node = 0; node < _path_length; node++)  {             
                                _dx[index] = rsub(_cx[city], _px[node]);
                                _dy[index] = rsub(_cy[city], _py[node]);
                                index++;
                        }
                }

                index = 0;
                for (int city = 0; city < _num_cities; city++)  {
                        for (int node = 0; node < _path_length; node++)  {
                                T dx2 = rmul(_dx[index], _dx[index]);
                                T dy2 = rmul(_dy[index], _dy[index]);
                                _d2[index] = radd(dx2, dy2);
                                T tmp = rneg(_d2[index]);
                                tmp = rdiv(tmp, coeff);
                                _w[index] = exponential(tmp);
                                index++;
                        }
                }

                index = 0;
                for (int city = 0; city < _num_cities; city++)  {
                        _sum[city] = 0.0;
                        for (int node = 0; node < _path_length; node++)  {
                                _sum[city] = radd(_sum[city], _w[index]);
                                index++;
                        }
                }

                index = 0;
                for (int city = 0; city < _num_cities; city++)  {
                        for (int node = 0; node < _path_length; node++)  {
                                _w[index] = rdiv(_w[index], _sum[city]);
                                index++;
                        }
                }
                
                index = 0;
                memset(_dfx, 0, _path_length * sizeof(T));        
                memset(_dfy, 0, _path_length * sizeof(T));        
                for (int city = 0; city < _num_cities; city++) {
                        for (int node = 0; node < _path_length; node++) {
                                T tmp = rmul(_dx[index], _w[index]);
                                _dfx[node] = radd(_dfx[node], tmp);
                                
                                tmp = rmul(_dy[index], _w[index]);
                                _dfy[node] = radd(_dfy[node], tmp);
                                index++;
                        }
                }

                /////
                T d2max = 0.0;
                index = 0;
                for (int city = 0; city < _num_cities; city++)  {
                        T d2min_path;
                        d2min_path = dtor(d2min_path, 1000000.0);
                        for (int node = 0; node < _path_length; node++)  {
                                if (_d2[index] < d2min_path)
                                        d2min_path = _d2[index];
                                index++;
                        }
                        if (d2min_path > d2max)
                                d2max = d2min_path;
                }
                _dmax2 = d2max;
        }

        template <typename T>
        void SelfOrganizedMap<T>::update_tension()
        {
                T tmp;
                T minus_two;
                minus_two = itor(minus_two, -2);

                tmp = rmul(minus_two, _px[0]);
                tmp = radd(_px[1], tmp);
                tmp = radd(tmp, _px[_path_length-1]);
                _tx[0] = tmp;

                tmp = rmul(minus_two, _py[0]);
                tmp = radd(_py[1], tmp);
                tmp = radd(tmp, _py[_path_length-1]);
                _ty[0] = tmp;

                for (int node = 1; node < _path_length - 1; node++)  {
                        tmp = rmul(minus_two, _px[node]);
                        tmp = radd(_px[node-1], tmp);
                        tmp = radd(tmp, _px[node+1]);
                        _tx[node] = tmp;

                        tmp = rmul(minus_two, _py[node]);
                        tmp = radd(_py[node-1], tmp);
                        tmp = radd(tmp, _py[node+1]);
                        _ty[node] = tmp;
                }


                tmp = rmul(minus_two, _px[_path_length-1]);
                tmp = radd(_px[0], tmp);
                tmp = radd(tmp, _px[_path_length-2]);
                _tx[_path_length-1] = tmp;

                tmp = rmul(minus_two, _py[_path_length-1]);
                tmp = radd(_py[0], tmp);
                tmp = radd(tmp, _py[_path_length-2]);
                _ty[_path_length-1] = tmp;
        }

        template <typename T>
        void SelfOrganizedMap<T>::update_positions()
        {
                T a, b;
                
                for (int node = 0; node < _path_length; node++)  {
                        a = rmul(_alpha, _dfx[node]);
                        b = rmul(_beta, _k);
                        b = rmul(b, _tx[node]);
                        a = radd(a, b);
                        _px[node] = radd(_px[node], a);
                        
                        a = rmul(_alpha, _dfy[node]);
                        b = rmul(_beta, _k);
                        b = rmul(b, _ty[node]);
                        a = radd(a, b);
                        _py[node] = radd(_py[node], a);
                }
        }
}

#endif // __ROMI_SELF_ORGANIZED_MAP_H
