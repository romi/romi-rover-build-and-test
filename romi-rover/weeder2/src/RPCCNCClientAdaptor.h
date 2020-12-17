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
#ifndef __ROMI_RPC_CNC_CLIENT_ADAPTOR_H
#define __ROMI_RPC_CNC_CLIENT_ADAPTOR_H

#include <stdexcept>
#include "IRPCHandler.h"
#include "ICNC.h"

namespace romi {

        class RPCCNCClientAdaptor : public ICNC
        {
        protected:
                rcom::IRPCHandler *_client;

                bool execute(const char *method, JSON &params, JSON &result);
                bool execute_with_params(const char *method, JSON &params);
                bool execute_with_result(const char *method, JSON &result);
                bool execute_simple_request(const char *method);

        public:
                RPCCNCClientAdaptor(rcom::IRPCHandler *rpc_handler)
                        : _client(rpc_handler) {
                        if (_client == 0) {
                                throw std::runtime_error("RPCCNCClientAdaptor: client=0");
                        }
                }
                virtual ~RPCCNCClientAdaptor() = default;

                bool get_range(CNCRange &range) override;
                bool moveto(double x, double y, double z,
                            double relative_speed = 0.1) override;
                bool spindle(double speed) override;
                bool travel(Path &path, double relative_speed = 0.1) override;
                bool homing() override;

                bool stop_execution() override;
                bool continue_execution() override;
                bool reset() override;
        };
}

#endif // __ROMI_RPC_CNC_CLIENT_ADAPTOR_H
