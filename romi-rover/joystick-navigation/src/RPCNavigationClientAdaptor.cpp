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
#include "RPCNavigationClientAdaptor.h"
#include "RPCError.h"

namespace romi {

        bool RPCNavigationClientAdaptor::execute(JSON &cmd)
        {
                bool success = false;
                JSON result;

                try {
                        _client.execute(cmd, result);
                        
                        success = _client.is_status_ok(result);
                        if (!success) {
                                r_err("RPCNavigationClientAdaptor::execute: %s",
                                      _client.get_error_message(result));
                        }
                        
                } catch (rcom::RPCError &e) {
                        r_err("RPCNavigationClientAdaptor::execute: '%s'", e.what());
                }

                return success;
        }

        bool RPCNavigationClientAdaptor::moveat(double left, double right)
        {
                JSON cmd = JSON::construct("{'command':'moveat',"
                                           "'speed':[%0.3f,%0.3f]}",
                                           left, right);
                return execute(cmd);
        }

        bool RPCNavigationClientAdaptor::move(double distance, double speed)
        {
                JSON cmd = JSON::construct("{'command':'move','distance':%0.3f,"
                                         "'speed':%0.3f}", distance, speed);
                return execute(cmd);
        }
        
        bool RPCNavigationClientAdaptor::stop()
        {
                JSON cmd("{'command':'stop'}");
                return execute(cmd);
        }
}
