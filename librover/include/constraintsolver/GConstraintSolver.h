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

#ifndef __ROMI_CONSTRAINT_SOLVER_H
#define __ROMI_CONSTRAINT_SOLVER_H


#pragma GCC system_header

//#pragma GCC diagnostic push
//#pragma GCC diagnostic ignored "-Wsign-conversion"
//#pragma GCC diagnostic ignored "-Weffc++"
//#pragma GCC diagnostic ignored "-Wvariadic-macros"

#include <ortools/constraint_solver/constraint_solver.h>
#include <ortools/constraint_solver/routing.h>

#include <ortools/constraint_solver/routing_index_manager.h>
#include <ortools/constraint_solver/routing_parameters.h>

#include "session/ISession.h"
#include "weeder/IPathPlanner.h"
#include "../som/Superpixels.h"

namespace romi {
        
        class GConstraintSolver : public IPathPlanner
        {

        public:
                explicit GConstraintSolver(nlohmann::json& params);
                ~GConstraintSolver() override = default;
                
                Path trace_path(ISession& session, Centers& centers, Image& mask) override;
                
        private:
                Path compute_path(std::vector<std::vector<int>>& locations,
                                  const Image& mask);
                
                Path build_path(const operations_research::RoutingIndexManager &manager,
                                const operations_research::RoutingModel &routing,
                                const operations_research::Assignment &solution,
                                const std::vector<std::vector<int>> &locations);
                bool print_;
        };
}

#endif
