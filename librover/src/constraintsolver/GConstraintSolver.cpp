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
#include "constraintsolver/GConstraintSolver.h"

namespace romi {

    GConstraintSolver::GConstraintSolver(JsonCpp &params) : print_(false) {
        if (params.has("print"))
            print_ = params.boolean("print");
    }


    bool GConstraintSolver::trace_path(ISession &session,
                                       Image &mask,
                                       double tool_diameter,
                                       double meters_to_pixels,
                                       Path &path) {

        std::cout << session.current_path() << std::endl;
        std::cout << path.size() << std::endl;
        Superpixels slic;

        double d = meters_to_pixels * tool_diameter;
        double n = 1.1 * static_cast<double>(mask.width()) * static_cast<double>(mask.height()) / (d * d);
        int max_cities = (int) n;

        printf(" **** max cities = %d ****\n", max_cities);

        Centers centers = slic.calculate_centers(mask, max_cities);

        printf(" **** centers size = %d ****\n", (int) centers.size());

        std::vector<std::vector<int>> locations;

        for (auto & center : centers) {
            std::vector<int> location;
            location.push_back((int) center.first);
            location.push_back((int) center.second);
            locations.push_back(location);
        }

        {
            std::ofstream file;
            file.open("centres.txt");
            for (auto & center : centers)
                file << center.first << "\t" << center.second << std::endl;
            file.close();
        }
        compute_path(locations, path, mask);

        return true;
    }

    std::vector<std::vector<int64_t>> ComputeEuclideanDistanceMatrix(
            const std::vector<std::vector<int>> &locations) {
        std::vector<std::vector<int64_t>> distances =
                std::vector<std::vector<int64_t>>(
                        locations.size(), std::vector<int64_t>(locations.size(), int64_t{0}));
        for (size_t fromNode = 0; fromNode < locations.size(); fromNode++) {
            for (size_t toNode = 0; toNode < locations.size(); toNode++) {
                if (fromNode != toNode)
                    distances[fromNode][toNode] = static_cast<int64_t>(
                            std::hypot((locations[toNode][0] - locations[fromNode][0]),
                                       (locations[toNode][1] - locations[fromNode][1])));
            }
        }
        return distances;
    }

    void GConstraintSolver::build_path(const operations_research::RoutingIndexManager &manager,
                       const operations_research::RoutingModel &routing,
                       const operations_research::Assignment &solution,
                       const std::vector<std::vector<int>> &locations,
                       Path& path,
                       const Image &mask) {

        std::cout << "Problem solved in " << routing.solver()->wall_time() << "ms\r\n";

        int64_t print_index = routing.Start(0);
        std::stringstream output_route;
        while (routing.IsEnd(print_index) == false) {
            auto location_index = (size_t) manager.IndexToNode(print_index).value();
            double x = locations[location_index][0] / (double) mask.width();
            double y = locations[location_index][1] / (double) mask.height();
            path.push_back(v3(x, y, 0));
            output_route << "{" << x << ", " << y << "},\r\n";
            print_index = solution.Value(routing.NextVar(print_index));
        }
        std::cout << "Route: \r\n" << output_route.str();
    }

    void GConstraintSolver::compute_path(std::vector<std::vector<int>>& locations, Path &path, const Image& mask) {

        const int num_vehicles = 1;
        const operations_research::RoutingIndexManager::NodeIndex depot{0};
        operations_research::RoutingIndexManager manager((int)locations.size(), num_vehicles, depot);
        operations_research::RoutingModel routing(manager);

        const auto distance_matrix = ComputeEuclideanDistanceMatrix(locations);
        const int transit_callback_index = routing.RegisterTransitCallback(
                [&distance_matrix, &manager](int64_t from_index, int64_t to_index) -> int64_t {
                    // Convert from routing variable Index to distance matrix NodeIndex.
                    auto from_node = (size_t)manager.IndexToNode(from_index).value();
                    auto to_node = (size_t)manager.IndexToNode(to_index).value();
                    return distance_matrix[from_node][to_node];
                });

        routing.SetArcCostEvaluatorOfAllVehicles(transit_callback_index);
        operations_research::RoutingSearchParameters searchParameters = operations_research::DefaultRoutingSearchParameters();
        searchParameters.set_first_solution_strategy(
                operations_research::FirstSolutionStrategy::PATH_CHEAPEST_ARC);
        const operations_research::Assignment *solution = routing.SolveWithParameters(searchParameters);
        build_path(manager, routing, *solution, locations, path, mask);
    }
}

