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

#include <getopt.h>

#include "DebugWeedingSession.h"
#include "som/SelfOrganizedMap.h"
#include "som/Real.h"

#include "test.h"

using namespace romi;

#define REAL double

int main(int argc, char **argv)
{
        DebugWeedingSession session;
        bool print = false;
        double alpha = 0.2;
        double beta = 2.0;
        double epsilon = 0.01;
        
        int option_index;
        static const char *optchars = "dpa:b:";
        static struct option long_options[] = {
                {"dump", no_argument, 0, 'd'},
                {"print", no_argument, 0, 'p'},
                {"alpha", required_argument, 0, 'a'},
                {"beta", required_argument, 0, 'b'},
                {"epsilon", required_argument, 0, 'e'},
                {0, 0, 0, 0}
        };
        
        while (1) {
                int c = getopt_long(argc, argv, optchars, long_options, &option_index);
                if (c == -1) break;
                switch (c) {
                case 'd':
                        session.open_dump();
                        break;
                case 'p':
                        print = true;
                        break;
                case 'a':
                        alpha = atof(optarg);
                        break;
                case 'b':
                        beta = atof(optarg);
                        break;
                case 'e':
                        epsilon = atof(optarg);
                        break;
                }
        }

        // REAL a, b, c;
        // dtor(a, 0.1);
        // dtor(b, 20.0);
        // c = rmul(a, b);
        // double r = rtod(c);
        // printf("%.24f\n", r);

        
        
        std::vector<double> cx;
        std::vector<double> cy;
        for (int city = 0; city < num_cities_test; city++) {
                cx.push_back(cities_test[city].x);
                cy.push_back(cities_test[city].y);
        }
        
        std::vector<double> px;
        std::vector<double> py;
        for (int i = 0; i < test_circle_length; i++) {
                px.push_back(test_circle[i].x);
                py.push_back(test_circle[i].y);
        }

        SelfOrganizedMap<double> som(num_cities_test,
                                     test_circle_length,
                                     alpha, beta, epsilon);


        som.init_cities(&cx[0], &cy[0]);
        som.init_path(&px[0], &py[0]);
        som.compute_path(&session, print);

        session.close_dump();
}
