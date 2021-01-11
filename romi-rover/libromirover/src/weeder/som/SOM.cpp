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

#include <iostream>
#include <fstream>
#include "SOM.h"
#include "Real.h"

namespace romi {

        SOM::SOM(JsonCpp& params) : _print(false)
        {
                try {
                        _alpha = params.num("alpha", 0.2);
                        _beta = params.num("beta", 1.2);
                        _epsilon = params.num("epsilon", 0.01);

                        if (params.has("print"))
                                _print = params.boolean("print");

                        assert_settings();
                        
                } catch (JSONError& je) {
                        r_warn("SOM: invalid JSON");
                        throw std::runtime_error("SOM: invalid JSON");
                }
        }
        
        void SOM::assert_settings()
        {
                if (_alpha < 0.01 || _alpha > 10.0
                    || _beta < 0.01 || _beta > 10.0
                    || _epsilon < 0.0001 || _epsilon > 1.0) {
                        r_warn("SOM: invalid settings: alpha %f, beta %f, epsilon %f",
                               _alpha, _beta, _epsilon);
                        throw std::runtime_error("SOM: invalid settings");
                }
        }
                
        bool SOM::trace_path(IFolder &session,
                             Image &mask,
                             double tool_diameter,
                             double meters_to_pixels,
                             Path &path)
        {
                Superpixels slic;

                double d = meters_to_pixels * tool_diameter;
                double n = 1.1 * (double) mask.width() * mask.height() / (d * d);
                int max_cities = (int) n;

                printf(" **** max cities = %d ****\n", max_cities);
                
                Centers centers = slic.calculate_centers(mask, max_cities);

                printf(" **** centers size = %d ****\n", (int)centers.size());

                if (1) {
                        std::vector<double> cx;
                        std::vector<double> cy;
                        for (size_t i = 0; i < centers.size(); i++) {
                                cx.push_back((double) centers[i].first / (double) mask.width());
                                cy.push_back((double) centers[i].second / (double) mask.height());
                        }
                        
                        {
                                std::ofstream file;
                                file.open("centres.txt");
                                for (size_t i = 0; i < centers.size(); i++)
                                        file << cx[i] << "\t" << cy[i] << std::endl;
                                file.close();
                        }
                
                        SelfOrganizedMap<double> som(centers.size(),
                                                     (int) (2.5 * centers.size()),
                                                     _alpha, _beta, _epsilon);
                        
                        som.init_cities(&cx[0], &cy[0]);
                        som.make_circle(0.1);
                        som.compute_path(session, _print);
                        som.get_path(path);
                }
                // else if (0) {
                        
                //         std::vector<float> cx;
                //         std::vector<float> cy;
                //         for (size_t i = 0; i < centers.size(); i++) {
                //                 cx.push_back((float) centers[i].first / (float) mask->width);
                //                 cy.push_back((float) centers[i].second / (float) mask->height);
                //         }

                //         {
                //                 std::ofstream file;
                //                 file.open("centres.txt");
                //                 for (size_t i = 0; i < centers.size(); i++)
                //                         file << cx[i] << "\t" << cy[i] << std::endl;
                //                 file.close();
                //         }
                
                //         SelfOrganizedMap<float> som(_alpha, _beta, _epsilon);
                //         som.trace_path(session, &cx[0], &cy[0], centers.size(), path, _print);

                //         printf(" **** USING FLOAT *******\n");
                //         printf(" **** alpha %f *******\n", _alpha);
                //         printf(" **** beta %f *******\n", _beta);
                //         printf(" **** epsilon %f *******\n", _epsilon);
                        
                // } 
                //         else if (1) {
                        
                //         std::vector<Double> cx;
                //         std::vector<Double> cy;
                //         for (size_t i = 0; i < centers.size(); i++) {
                //                 cx.push_back((double) centers[i].first / (double) mask->width);
                //                 cy.push_back((double) centers[i].second / (double) mask->height);
                //         }

                //         {
                //                 std::ofstream file;
                //                 file.open("centres.txt");
                //                 for (size_t i = 0; i < centers.size(); i++)
                //                         file << cx[i].to_double() << "\t"
                //                              << cy[i].to_double() << std::endl;
                //                 file.close();
                //         }
                
                //         SelfOrganizedMap<Double> som(_alpha, _beta, _epsilon);
                //         som.trace_path(session, &cx[0], &cy[0], centers.size(),
                //                        path, _print);

                //         printf(" **** USING Double class *******\n");
                //         printf(" **** alpha %f *******\n", _alpha);
                //         printf(" **** beta %f *******\n", _beta);
                //         printf(" **** epsilon %f *******\n", _epsilon);
                // }
                
                return true;
        }
}
