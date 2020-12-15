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
#include <r.h>
#include "RPCNavigationServerAdaptor.h"

namespace romi {
        
        void RPCNavigationServerAdaptor::execute(JSON &cmd, JSON &result)
        {
                r_debug("RPCNavigationServerAdaptor::navigation: execute");

                if (cmd.has("command")) {
                        
                        const char *command = cmd.str("command");
                
                        if (rstreq(command, "moveat")) {
                                execute_moveat(cmd, result);
                        } else if (rstreq(command, "move")) {
                                execute_move(cmd, result);
                        } else if (rstreq(command, "stop")) {
                                execute_stop(cmd, result);
                        } else {
                                error_status(result, "Unknown command");
                        }
                } else {
                        error_status(result, "Missing command");
                }
        }
        
        void RPCNavigationServerAdaptor::ok_status(JSON &result)
        {
                result = JSON::parse("{\"status\": \"ok\"}");
        }
        
        void RPCNavigationServerAdaptor::error_status(JSON &result, const char *message)
        {
                result = JSON::construct("{\"status\": \"error\", "
                                        "\"message\": \"%s\"}",
                                        message);
        }

        void RPCNavigationServerAdaptor::execute_moveat(JSON &cmd, JSON &result)
        {
                r_debug("RPCNavigationServerAdaptor::execute_moveat");
                try {
                        double left = cmd.array("speed").num(0);
                        double right = cmd.array("speed").num(1);
                
                        if (_navigation.moveat(left, right))
                                ok_status(result);
                        else
                                error_status(result, "moveat failed");
                        
                } catch (JSONError &je) {
                        r_debug("RPCNavigationServerAdaptor::execute_moveat: %s",
                                je.what());
                        error_status(result, je.what());
                }
        }

        void RPCNavigationServerAdaptor::execute_move(JSON &cmd, JSON &result)
        {
                r_debug("RPCNavigationServerAdaptor::execute_move");
                
                try {
                        double distance = cmd.num("distance");
                        double speed = cmd.num("speed");
                
                        if (_navigation.move(distance, speed))
                                ok_status(result);
                        else
                                error_status(result, "move failed");
                        
                } catch (JSONError &je) {
                        r_debug("RPCNavigationServerAdaptor::execute_move: %s", je.what());
                        error_status(result, je.what());
                }
        }

        void RPCNavigationServerAdaptor::execute_stop(JSON &cmd, JSON &result)
        {
                r_debug("RPCNavigationServerAdaptor::execute_stop");
                if (_navigation.stop())
                        ok_status(result);
                else
                        error_status(result, "stop failed");
        }
}
