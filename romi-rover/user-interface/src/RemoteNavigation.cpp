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
#include "RemoteNavigation.h"
#include "RPCError.h"

namespace romi {

        void RemoteNavigation::execute(const char *method,
                                       JsonCpp& params,
                                       rcom::RPCError &error)
        {
                JsonCpp result; // Not returned to callers. 

                _client.execute(method, params, result, error);
                
                if (error.code != 0) {
                        r_err("RemoteNavigation::execute: %s",
                              error.message.c_str());
                }                
        }

        bool RemoteNavigation::moveat(double left, double right)
        {
                rcom::RPCError error;
                JsonCpp params = JsonCpp::construct("{'speed':[%0.3f,%0.3f]}",
                                              left, right);
                execute("moveat", params, error);

                return (error.code == 0);
        }

        bool RemoteNavigation::move(double distance, double speed)
        {
                rcom::RPCError error;
                JsonCpp params = JsonCpp::construct("{'distance':%0.3f,"
                                              "'speed':%0.3f}",
                                              distance, speed);
                execute("move", params, error);
                
                return (error.code == 0);
        }
        
        bool RemoteNavigation::stop()
        {
                rcom::RPCError error;
                JsonCpp params; // = No params
                execute("stop", params, error);
                return (error.code == 0);
        }
}
