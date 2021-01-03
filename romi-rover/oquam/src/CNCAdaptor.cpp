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
#include "CNCAdaptor.h"

namespace romi {

        void CNCAdaptor::execute(const char *method, JsonCpp& params,
                                 JsonCpp& result, rcom::RPCError &error)
        {
                r_debug("CNCAdaptor::execute");

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
                                
                        } else if (rstreq(method, "stop")) {
                                handle_stop(params, result, error);
                                
                        } else if (rstreq(method, "continue")) {
                                handle_continue(params, result, error);
                                
                        } else if (rstreq(method, "reset")) {
                                handle_reset(params, result, error);
                                
                        } else {
                                error.code = rcom::RPCError::MethodNotFound;
                                error.message = "Unknown method";
                        }

                } catch (std::exception &e) {
                        error.code = rcom::RPCError::InternalError;
                        error.message = e.what();
                }
        }

        void CNCAdaptor::handle_get_range(JsonCpp& params, JsonCpp& result,
                                          rcom::RPCError &error)
        {
                r_debug("CNCAdaptor::handle_get_range");
                CNCRange range;
                if (_cnc.get_range(range)) {
                        result = JsonCpp::construct("[[%f,%f],[%f,%f],[%f,%f]]",
                                                    range.min.x(), range.max.x(),
                                                    range.min.y(), range.max.y(),
                                                    range.min.z(), range.max.z());
                } else {
                        r_err("CNCAdaptor::handle_get_range failed");
                        error.code = 1;
                        error.message = "get_range failed";
                }
        }
        
        void CNCAdaptor::handle_moveto(JsonCpp& params, JsonCpp& result,
                                       rcom::RPCError &error)
        {
                r_debug("CNCAdaptor::handle_moveto");

                {
                        char buffer[256];
                        json_tostring(params.ptr(), buffer, 256);
                        r_debug("CNCAdaptor::handle_moveto: %s", buffer);
                }

                if (!params.has("x") && !params.has("y") && !params.has("z")) {
                        r_err("CNCAdaptor::handle_moveto failed: missing parameters");
                        error.code = rcom::RPCError::InvalidParams;
                        error.message = "missing x, y, or z parameters";
                        
                } else {
                        
                        double x = params.num("x", CNC::UNCHANGED);
                        double y = params.num("y", CNC::UNCHANGED);
                        double z = params.num("z", CNC::UNCHANGED);
                        double v = params.num("speed", 0.2);
                        
                        r_debug("CNCAdaptor::handle_moveto: %f, %f, %f", x, y, z);
                                
                        if (!_cnc.moveto(x, y, z, v)) {
                                error.code = 1;
                                error.message = "moveto failed";
                        }
                }
        }
        
        void CNCAdaptor::handle_spindle(JsonCpp& params, JsonCpp& result,
                                        rcom::RPCError &error)
        {
                r_debug("CNCAdaptor::handle_spindle");
                
                try {
                        double speed = params.num("speed");
                        if (!_cnc.spindle(speed)) {
                                error.code = 1;
                                error.message = "spindle failed";
                        }

                } catch (JSONError &je) {
                        r_err("CNCAdaptor::handle_spindle failed: %s", je.what());
                        error.code = rcom::RPCError::InvalidParams;
                        error.message = je.what();
                }
        }
        
        void CNCAdaptor::handle_travel(JsonCpp& params, JsonCpp& result,
                                       rcom::RPCError &error)
        {
                r_debug("CNCAdaptor::handle_travel");
                
                try {
                        Path path;
                        double speed = params.num("speed", 0.1);
                        JsonCpp p = params.array("path");
                        for (int i = 0; i < p.length(); i++) {
                                v3 pt;
                                JsonCpp a = p.array(i);
                                pt.set(a.num(0), a.num(1), a.num(2));
                                path.push_back(pt);
                        }

                        if (!_cnc.travel(path, speed)) {
                                error.code = 1;
                                error.message = "travel failed";
                        }

                } catch (JSONError &je) {
                        r_err("CNCAdaptor::handle_spindle failed: %s", je.what());
                        error.code = rcom::RPCError::InvalidParams;
                        error.message = je.what();
                }
        }
        
        void CNCAdaptor::handle_homing(JsonCpp& params, JsonCpp& result,
                                       rcom::RPCError &error)
        {
                r_debug("CNCAdaptor::handle_homing");
                
                if (!_cnc.homing()) {
                        error.code = 1;
                        error.message = "homing failed";
                }
        }

        void CNCAdaptor::handle_stop(JsonCpp& params, JsonCpp& result,
                                     rcom::RPCError &error)
        {
                r_debug("CNCAdaptor::handle_stop");
                if (!_cnc.stop_execution()) {
                        error.code = 1;
                        error.message = "stop failed";
                }
        }

        void CNCAdaptor::handle_continue(JsonCpp& params, JsonCpp& result,
                                         rcom::RPCError &error)
        {
                r_debug("CNCAdaptor::handle_continue");
                if (!_cnc.continue_execution()) {
                        error.code = 1;
                        error.message = "continue failed";
                }
        }

        void CNCAdaptor::handle_reset(JsonCpp& params, JsonCpp& result,
                                      rcom::RPCError &error)
        {
                r_debug("CNCAdaptor::handle_reset");
                if (!_cnc.reset()) {
                        error.code = 1;
                        error.message = "reset failed";
                }
        }
}

