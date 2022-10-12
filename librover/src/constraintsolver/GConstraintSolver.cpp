/*
  romi-rover

  Copyright (C) 2019 Sony Computer Science Laboratories
  Author(s) Peter Hanappe / Douglas Boari

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
#include <util/Logger.h>
#include <cv/cv.h>
#include "constraintsolver/GConstraintSolver.h"

namespace romi {

        GConstraintSolver::GConstraintSolver(nlohmann::json &params) : print_(false)
        {
                if (params.contains("print"))
                        print_ = params["print"];
        }


        Path GConstraintSolver::trace_path(ISession &session, Centers& centers, Image& mask)
        {
                std::vector<std::vector<int>> locations;

                for (auto & center : centers) {
                        std::vector<int> location;
                        location.push_back((int) center.first);
                        location.push_back((int) center.second);
                        locations.push_back(location);
                }

                {
                        rcom::MemBuffer buffer;
                        for (auto & center : centers)
                                buffer.printf("%zu\t%zu\n", center.first, center.second);
                        session.store_txt("centers.txt", buffer.tostring());
                }
                
                return compute_path(locations, mask);
        }

        static const int64_t kLargeDistance = 10000;
        
        static int64_t compute_distance(double x0, double y0,
                                        double x1, double y1,
                                        const Image& mask)
        {
                int64_t distance;
                bool line_crosses = false;
                
                //(void) mask;
                line_crosses = romi::segment_crosses_white_area(mask,
                                                                v3(x0, y0, 0),
                                                                v3(x1, y1, 0));
                
                if (line_crosses) {
                        distance = kLargeDistance;
                } else {
                        distance = (int64_t) std::hypot(x1 - x0, y1 - y0);
                }
                return distance;
        }
        
        static std::vector<std::vector<int64_t>>
        compute_distance_matrix(const std::vector<std::vector<int>> &locations,
                                const Image& mask)
        {
                std::vector<std::vector<int64_t>> distances =
                        std::vector<std::vector<int64_t>>(
                                locations.size(),
                                std::vector<int64_t>(locations.size(), int64_t{0}));
                
                for (size_t fromNode = 0; fromNode < locations.size(); fromNode++) {
                        for (size_t toNode = 0; toNode < locations.size(); toNode++) {
                                if (fromNode != toNode) {
                                        double x0 = locations[fromNode][0];
                                        double y0 = locations[fromNode][1];
                                        double x1 = locations[toNode][0];
                                        double y1 = locations[toNode][1];
                                        int64_t d = compute_distance(x0, y0, x1, y1, mask);
                                        distances[fromNode][toNode] = d;
                                }
                        }
                }
                return distances;
        }

        Path GConstraintSolver::build_path(const operations_research::RoutingIndexManager &manager,
                                           const operations_research::RoutingModel &routing,
                                           const operations_research::Assignment &solution,
                                           const std::vector<std::vector<int>> &locations)
        {
                r_info("TSP route calculated in %lld ms\r\n", routing.solver()->wall_time());
                int64_t print_index = routing.Start(0);
                std::stringstream output_route;
                Path path;

                while (!routing.IsEnd(print_index)) {
                        auto location_index = (size_t) manager.IndexToNode(print_index).value();
                        double x = (double) locations[location_index][0];
                        double y = (double) locations[location_index][1];
                        path.push_back(v3(x, y, 0));
                        print_index = solution.Value(routing.NextVar(print_index));

                }
                r_info("Distance %lld\r\n", solution.ObjectiveValue());
                return path;
        }

        Path GConstraintSolver::compute_path(std::vector<std::
                                             vector<int>>& locations,
                                             const Image& mask)
        {
                const int num_vehicles = 1;
                const operations_research::RoutingIndexManager::NodeIndex depot{0};
                operations_research::RoutingIndexManager manager((int)locations.size(),
                                                                 num_vehicles, depot);
                operations_research::RoutingModel routing(manager);

                const auto distance_matrix = compute_distance_matrix(locations, mask);
                const int transit_callback_index = routing.RegisterTransitCallback(
                        [&distance_matrix, &manager](int64_t from_index, int64_t to_index) -> int64_t {
                                // Convert from routing variable Index
                                // to distance matrix NodeIndex.
                                auto from_node = (size_t)manager.IndexToNode(from_index).value();
                                auto to_node = (size_t)manager.IndexToNode(to_index).value();
                                return distance_matrix[from_node][to_node];
                        });

                routing.SetArcCostEvaluatorOfAllVehicles(transit_callback_index);
                operations_research::RoutingSearchParameters searchParameters
                        = operations_research::DefaultRoutingSearchParameters();
                searchParameters.set_first_solution_strategy(
                        operations_research::FirstSolutionStrategy::SAVINGS);
                const operations_research::Assignment *solution
                        = routing.SolveWithParameters(searchParameters);
                if (solution == nullptr)
                        throw std::runtime_error("Contraint Solver failed to find path.");

                return build_path(manager, routing, *solution, locations);
        }
}

