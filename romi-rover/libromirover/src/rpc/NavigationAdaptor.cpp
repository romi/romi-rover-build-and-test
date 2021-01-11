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
#include "rpc/NavigationAdaptor.h"
#include "rpc/MethodsRover.h"

namespace romi {
        
        void NavigationAdaptor::execute(const char *method, JsonCpp &params,
                                        JsonCpp &result, rcom::RPCError &error)
        {
                r_debug("NavigationAdaptor::execute");

                error.code = 0;
                
                try {
                        
                        if (rstreq(method, MethodsNavigation::moveat)) {
                                execute_moveat(params, result, error);
                                
                        } else if (rstreq(method, MethodsNavigation::move)) {
                                execute_move(params, result, error);
                                
                        } else if (rstreq(method, MethodsNavigation::stop)) {
                                execute_stop(params, result, error);
                                
                        } else if (rstreq(method, MethodsActivity::activity_pause)) {
                                execute_pause(params, result, error);
                                
                        } else if (rstreq(method, MethodsActivity::activity_continue)) {
                                execute_continue(params, result, error);
                                
                        } else if (rstreq(method, MethodsActivity::activity_reset)) {
                                execute_reset(params, result, error);
                                
                        } else {
                                error.code = rcom::RPCError::MethodNotFound;
                                error.message = "Unknown command";
                        }
                        
                } catch (std::exception &e) {
                        r_err("NavigationAdaptor::execute: caught exception: %s",
                              e.what());
                        error.code = rcom::RPCError::InternalError;
                        error.message = e.what();
                }
        }

        void NavigationAdaptor::execute_moveat(JsonCpp &params, JsonCpp &result,
                                               rcom::RPCError &error)
        {
                r_debug("NavigationAdaptor::execute_moveat");
                try {
                        double left = params.array("speed").num(0);
                        double right = params.array("speed").num(1);
                
                        if (!_navigation.moveat(left, right)) {
                                error.code = 1;
                                error.message = "moveat failed";
                        }
                        
                } catch (JSONError &je) {
                        r_debug("NavigationAdaptor::execute_moveat: %s",
                                je.what());
                        error.code = rcom::RPCError::ParseError;
                        error.message = "Invalid json";
                }
        }

        void NavigationAdaptor::execute_move(JsonCpp &params, JsonCpp &result,
                                             rcom::RPCError &error)
        {
                r_debug("NavigationAdaptor::execute_move");
                
                try {
                        double distance = params.num("distance");
                        double speed = params.num("speed");
                
                        if (!_navigation.move(distance, speed)) {
                                error.code = 1;
                                error.message = "move failed";
                        }
                        
                } catch (JSONError &je) {
                        r_debug("NavigationAdaptor::execute_move: %s", je.what());
                        error.code = rcom::RPCError::ParseError;
                        error.message = "Invalid json";
                }
        }

        void NavigationAdaptor::execute_stop(JsonCpp &params, JsonCpp &result,
                                             rcom::RPCError &error)
        {
                r_debug("NavigationAdaptor::execute_stop");
                
                if (!_navigation.stop()) {
                        error.code = 1;
                        error.message = "stop failed"; // Good luck with that!
                }
        }

        void NavigationAdaptor::execute_pause(JsonCpp& params, JsonCpp& result,
                                     rcom::RPCError &error)
        {
                r_debug("NavigationAdaptor::execute_pause");
                if (!_navigation.pause_activity()) {
                        error.code = 1;
                        error.message = "stop failed";
                }
        }

        void NavigationAdaptor::execute_continue(JsonCpp& params, JsonCpp& result,
                                         rcom::RPCError &error)
        {
                r_debug("NavigationAdaptor::execute_continue");
                if (!_navigation.continue_activity()) {
                        error.code = 1;
                        error.message = "continue failed";
                }
        }

        void NavigationAdaptor::execute_reset(JsonCpp& params, JsonCpp& result,
                                      rcom::RPCError &error)
        {
                r_debug("NavigationAdaptor::execute_reset");
                if (!_navigation.reset_activity()) {
                        error.code = 1;
                        error.message = "reset failed";
                }
        }
}
