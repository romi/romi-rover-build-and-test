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
#ifndef __ROMI_RPC_MOTORDRIVER_ADAPTOR_H
#define __ROMI_RPC_MOTORDRIVER_ADAPTOR_H

#include "IRPCHandler.h"
#include "IMotorDriver.h"

namespace romi {
        
        class RPCMotorDriverAdaptor : public rcom::IRPCHandler
        {
        protected:
                IMotorDriver &_driver;

                JSON ok_status() {
                        return JSON::parse("{\"status\": \"ok\"}");
                }
        
                JSON error_status(const char *message) {
                        return JSON::construct("{\"status\": \"error\", "
                                               "\"message\": \"%s\"}",
                                               message);
                }

                JSON execute_moveat(JSON &cmd) {
                        double left = cmd.array("speed").num(0);
                        double right = cmd.array("speed").num(1);
                        if (_driver.moveat(left, right))
                                return ok_status();
                        else
                                return error_status("moveat failed");
                }
                
        public:
                RPCMotorDriverAdaptor(IMotorDriver &driver) : _driver(driver) {}
                virtual ~RPCMotorDriverAdaptor() override = default;

                JSON execute(JSON cmd) override {
                        JSON response;
                        const char *command = cmd.str("command");
                        if (rstreq(command, "moveat")) {
                                response = execute_moveat(cmd);
                        } else {
                                response = error_status("Unknown command");
                        }
                        return response;
                }
        };
}

#endif // __ROMI_RPC_MOTORDRIVER_ADAPTOR_H
