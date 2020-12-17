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
#include "RPCCNCServerAdaptor.h"

namespace romi {

        void RPCCNCServerAdaptor::execute(const char *method, JSON &params,
                                          JSON &result, rcom::RPCError &error)
        {
                r_debug("RPCCNCServerAdaptor::execute");

                error.code = 0;
                
                try {

                        if (method == 0) {
                                error.code = rcom::RPCError::MethodNotFound;
                                error.message = "No method specified";
                                
                        } else if (rstreq(method, "homing")) {
                                handle_homing(params, result, error);
                                 
                        } else if (rstreq(method, "moveto")) {
                                handle_moveto(params, result, error);
                                
                        } else if (rstreq(method, "spindle")) {
                                handle_spindle(params, result, error);
                                
                        } else if (rstreq(method, "travel")) {
                                handle_travel(params, result, error);
                                
                        } else if (rstreq(method, "get-range")) {
                                handle_get_range(params, result, error);
                                
                        } else {
                                error.code = rcom::RPCError::MethodNotFound;
                                error.message = "Unknown method";
                        }

                } catch (std::exception &e) {
                        error.code = rcom::RPCError::InternalError;
                        error.message = e.what();
                }
        }

        void RPCCNCServerAdaptor::handle_get_range(JSON &params, JSON &result, rcom::RPCError &error)
        {
                r_debug("RPCCNCServerAdaptor::handle_get_range");
                CNCRange range;
                if (_cnc.get_range(range)) {
                        result = JSON::construct("[[%f,%f],[%f,%f],[%f,%f]]",
                                                 range._x[0], range._x[1],
                                                 range._y[0], range._y[1],
                                                 range._z[0], range._z[1]);
                } else {
                        r_err("RPCCNCServerAdaptor::handle_get_range failed");
                        error.code = 1;
                        error.message = "get_range failed";
                }
        }
        
        void RPCCNCServerAdaptor::handle_moveto(JSON &params, JSON &result, rcom::RPCError &error)
        {
                r_debug("RPCCNCServerAdaptor::handle_moveto");

                {
                        char buffer[256];
                        json_tostring(params.ptr(), buffer, 256);
                        r_debug("RPCCNCServerAdaptor::handle_moveto: %s", buffer);
                }

                try {
                        double x = params.num("x", ICNC::UNCHANGED);
                        double y = params.num("y", ICNC::UNCHANGED);
                        double z = params.num("z", ICNC::UNCHANGED);
                        double v = params.num("speed", 0.2);
                        
                        r_debug("RPCCNCServerAdaptor::handle_moveto: %f, %f, %f", x, y, z);
                        _cnc.moveto(x, y, z, v);

                } catch (JSONError &je) {
                        r_err("RPCCNCServerAdaptor::handle_moveto failed: %s", je.what());
                        error.code = 1;
                        error.message = je.what();
                }
        }
        
        void RPCCNCServerAdaptor::handle_spindle(JSON &params, JSON &result, rcom::RPCError &error)
        {
                r_debug("RPCCNCServerAdaptor::handle_spindle");
                
                try {
                        double speed = params.num("speed");
                        if (!_cnc.spindle(speed)) {
                                error.code = 1;
                                error.message = "spindle failed";
                        }

                } catch (JSONError &je) {
                        r_err("RPCCNCServerAdaptor::handle_spindle failed: %s", je.what());
                        error.code = 1;
                        error.message = je.what();
                }
        }
        
        void RPCCNCServerAdaptor::handle_travel(JSON &params, JSON &result, rcom::RPCError &error)
        {
                r_debug("RPCCNCServerAdaptor::handle_travel");
                
                try {
                        Path path;
                        double speed = params.num("speed", 0.1);
                        JSON p = params.array("path");
                        for (int i = 0; i < p.length(); i++) {
                                Waypoint w;
                                JSON c = p.array(i);
                                w.x = c.num(0);
                                w.y = c.num(1);
                                w.z = c.num(2);
                                path.push_back(w);
                        }
                        
                        if (!_cnc.travel(path, speed)) {
                                error.code = 1;
                                error.message = "travel failed";
                        }

                } catch (JSONError &je) {
                        r_err("RPCCNCServerAdaptor::handle_spindle failed: %s", je.what());
                        error.code = 1;
                        error.message = je.what();
                }
        }
        
        void RPCCNCServerAdaptor::handle_homing(JSON &params, JSON &result, rcom::RPCError &error)
        {
                r_debug("RPCCNCServerAdaptor::handle_homing");
                
                if (!_cnc.homing()) {
                        error.code = 1;
                        error.message = "homing failed";
                }
        }

        void RPCCNCServerAdaptor::handle_stop(JSON &params, JSON &result, rcom::RPCError &error)
        {
                r_debug("RPCCNCServerAdaptor::handle_stop");
                if (!_cnc.stop_execution()) {
                        error.code = 1;
                        error.message = "stop failed";
                }
        }

        void RPCCNCServerAdaptor::handle_continue(JSON &params, JSON &result, rcom::RPCError &error)
        {
                r_debug("RPCCNCServerAdaptor::handle_continue");
                if (!_cnc.continue_execution()) {
                        error.code = 1;
                        error.message = "continue failed";
                }
        }

        void RPCCNCServerAdaptor::handle_reset(JSON &params, JSON &result, rcom::RPCError &error)
        {
                r_debug("RPCCNCServerAdaptor::handle_reset");
                if (!_cnc.reset()) {
                        error.code = 1;
                        error.message = "reset failed";
                }
        }
}

