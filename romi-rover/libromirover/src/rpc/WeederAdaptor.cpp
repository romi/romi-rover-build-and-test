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
#include "rpc/WeederAdaptor.h"
#include "rpc/MethodsRover.h"

namespace romi {
        
        void WeederAdaptor::execute(const char *method, JsonCpp& params,
                                    JsonCpp& result, rcom::RPCError &error)
        {
                r_debug("WeederAdaptor::execute");
                
                error.code = 0;
                
                try {

                        if (rstreq(method, MethodsWeeder::hoe)) {
                                execute_hoe(error);
                        
                        } else if (rstreq(method, MethodsWeeder::stop)) {
                                execute_hoe(error);
                        
                        } else if (rstreq(method, MethodsActivity::activity_pause)) {
                                execute_pause(error);
                                
                        } else if (rstreq(method, MethodsActivity::activity_continue)) {
                                execute_continue(error);
                                
                        } else if (rstreq(method, MethodsActivity::activity_reset)) {
                                execute_reset(error);
                                
                        } else if (rstreq(method, MethodsPowerDevice::power_up)) {
                                execute_power_up(error);
                                
                        } else if (rstreq(method, MethodsPowerDevice::power_down)) {
                                execute_power_down(error);
                                
                        } else if (rstreq(method, MethodsPowerDevice::stand_by)) {
                                execute_stand_by(error);
                                
                        } else if (rstreq(method, MethodsPowerDevice::wake_up)) {
                                execute_wake_up(error);
                                
                        } else {
                                error.code = rcom::RPCError::MethodNotFound;
                                error.message = "Unknown method";
                        }
                        
                } catch (std::exception &e) {
                        error.code = rcom::RPCError::InternalError;
                        error.message = e.what();
                }
        }

        void WeederAdaptor::execute_hoe(rcom::RPCError &error)
        {
                r_debug("WeederAdaptor::execute_hoe");
                if (!_weeder.hoe()) {
                        error.code = 1;
                        error.message = "Hoe failed";
                }
        }

        void WeederAdaptor::execute_stop(rcom::RPCError &error)
        {
                r_debug("WeederAdaptor::execute_stop");
                if (!_weeder.stop()) {
                        error.code = 1;
                        error.message = "Stop failed";
                }
        }

        void WeederAdaptor::execute_pause(rcom::RPCError &error)
        {
                r_debug("WeederAdaptor::execute_pause");
                if (!_weeder.pause_activity()) {
                        error.code = 1;
                        error.message = "stop failed";
                }
        }

        void WeederAdaptor::execute_continue(rcom::RPCError &error)
        {
                r_debug("WeederAdaptor::execute_continue");
                if (!_weeder.continue_activity()) {
                        error.code = 1;
                        error.message = "continue failed";
                }
        }

        void WeederAdaptor::execute_reset(rcom::RPCError &error)
        {
                r_debug("WeederAdaptor::execute_reset");
                if (!_weeder.reset_activity()) {
                        error.code = 1;
                        error.message = "reset failed";
                }
        }

        void WeederAdaptor::execute_power_up(rcom::RPCError &error)
        {
                r_debug("WeederAdaptor::power_up");
                if (!_weeder.power_up()) {
                        error.code = 1;
                        error.message = "power up failed";
                }
        }
        
        void WeederAdaptor::execute_power_down(rcom::RPCError &error)
        {
                r_debug("WeederAdaptor::power_down");
                if (!_weeder.power_down()) {
                        error.code = 1;
                        error.message = "power down failed";
                }
        }
        
        void WeederAdaptor::execute_stand_by(rcom::RPCError &error)
        {
                r_debug("WeederAdaptor::stand_by");
                if (!_weeder.stand_by()) {
                        error.code = 1;
                        error.message = "stand_by failed";
                }
        }
        
        void WeederAdaptor::execute_wake_up(rcom::RPCError &error)
        {
                r_debug("WeederAdaptor::wake_up");
                if (!_weeder.wake_up()) {
                        error.code = 1;
                        error.message = "wake_up failed";
                }
        }
}
