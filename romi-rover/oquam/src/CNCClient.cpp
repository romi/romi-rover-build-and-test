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
#include "CNCClient.h"

namespace romi {

        JSON CNCClient::execute(JSON cmd)
        {
                JSON retval;
                
                try {
                        const char *command = cmd.str("command");

                        if (command == 0) {
                                retval = error_status("No command specified");
                        } else if (rstreq(command, "homing")) {
                                 retval = handle_homing(cmd);
                        } else if (rstreq(command, "moveto")) {
                                retval = handle_moveto(cmd);
                        } else if (rstreq(command, "spindle")) {
                                retval = handle_spindle(cmd);
                        } else if (rstreq(command, "travel")) {
                                retval = handle_travel(cmd);
                        } else if (rstreq(command, "get-range")) {
                                retval = handle_get_range(cmd);
                        } else {
                                retval = error_status("Unknown command");
                        }

                } catch (std::exception e) {
                        retval = error_status(e.what());
                }
                
                return retval;
        }

        JSON CNCClient::ok_status()
        {
                return JSON::parse("{\"status\": \"ok\"}");
        }
        
        JSON CNCClient::error_status(const char *message)
        {
                return JSON::construct("{\"status\": \"error\", \"message\": \"%s\"}", message);
        }

        JSON CNCClient::handle_get_range(JSON cmd)
        {
                CNCRange range;
                _cnc.get_range(range);
                return JSON::construct("[[%f,%f],[%f,%f],[%f,%f]]",
                                       range._x[0], range._x[1],
                                       range._y[0], range._y[1],
                                       range._z[0], range._z[1]);
        }
        
        JSON CNCClient::handle_moveto(JSON cmd)
        {
                double x = cmd.num("x");
                double y = cmd.num("y");
                double z = cmd.num("z");
                _cnc.moveto(x, y, z);
                return ok_status();
        }
        
        JSON CNCClient::handle_spindle(JSON cmd)
        {
                double speed = cmd.num("speed");
                _cnc.spindle(speed);
                return ok_status();
        }
        
        JSON CNCClient::handle_travel(JSON cmd)
        {
                Path path;

                double speed = cmd.num("speed", 0.1);
                JSON p = cmd.array("path");
                for (int i = 0; i < p.length(); i++) {
                        Waypoint w;
                        JSON c = p.array(i);
                        w.x = c.num(0);
                        w.y = c.num(1);
                        w.z = c.num(2);
                        path.push_back(w);
                }

                _cnc.travel(path, speed);

                return ok_status();
        }
        
        JSON CNCClient::handle_homing(JSON cmd)
        {
                _cnc.homing();
                return ok_status();
        }

        JSON CNCClient::handle_stop(JSON cmd)
        {
                _cnc.stop_execution();
                return ok_status();
        }

        JSON CNCClient::handle_continue(JSON cmd)
        {
                _cnc.continue_execution();
                return ok_status();
        }

        JSON CNCClient::handle_reset(JSON cmd)
        {
                _cnc.reset();
                return ok_status();
        }
}

