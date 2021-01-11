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
#include <RPCError.h>
#include "rpc/RemoteNavigation.h"
#include "rpc/MethodsRover.h"

namespace romi {

        bool RemoteNavigation::moveat(double left, double right)
        {
                JsonCpp params = JsonCpp::construct("{'speed':[%0.3f,%0.3f]}",
                                              left, right);
                return execute_with_params(MethodsNavigation::moveat, params);
        }

        bool RemoteNavigation::move(double distance, double speed)
        {
                JsonCpp params = JsonCpp::construct("{'distance':%0.3f,"
                                              "'speed':%0.3f}",
                                              distance, speed);
                return execute_with_params(MethodsNavigation::move, params);
        }
        
        bool RemoteNavigation::stop()
        {
                r_debug("RemoteNavigation::stop");
                return execute_simple_request(MethodsNavigation::stop);
        }

        bool RemoteNavigation::pause_activity()
        {
                r_debug("RemoteNavigation::pause_activity");
                return execute_simple_request(MethodsActivity::activity_pause);
        }
        
        bool RemoteNavigation::continue_activity()
        {
                r_debug("RemoteNavigation::continue_activity");
                return execute_simple_request(MethodsActivity::activity_continue);
        }
        
        bool RemoteNavigation::reset_activity()
        {
                r_debug("RemoteNavigation::reset");
                return execute_simple_request(MethodsActivity::activity_reset);
        }
        
}
