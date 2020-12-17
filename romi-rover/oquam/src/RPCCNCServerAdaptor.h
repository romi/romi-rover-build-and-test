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
#ifndef __ROMI_RPC_CNC_SERVER_ADAPTER_H
#define __ROMI_RPC_CNC_SERVER_ADAPTER_H

#include "ICNC.h"
#include "IRPCHandler.h"

namespace romi {
        
        class RPCCNCServerAdaptor : public rcom::IRPCHandler
        {
        protected:
                ICNC &_cnc;
                
                void handle_get_range(JSON &params, JSON &result, rcom::RPCError &error);
                void handle_moveto(JSON &params, JSON &result, rcom::RPCError &error);
                void handle_spindle(JSON &params, JSON &result, rcom::RPCError &error);
                void handle_travel(JSON &params, JSON &result, rcom::RPCError &error);
                void handle_homing(JSON &params, JSON &result, rcom::RPCError &error);
                void handle_stop(JSON &params, JSON &result, rcom::RPCError &error);
                void handle_continue(JSON &params, JSON &result, rcom::RPCError &error);
                void handle_reset(JSON &params, JSON &result, rcom::RPCError &error);

        public:
                RPCCNCServerAdaptor(ICNC &cnc) : _cnc(cnc) {}
                virtual ~RPCCNCServerAdaptor() override = default;
                
                void execute(const char *method, JSON &params,
                             JSON &result, rcom::RPCError &error) override;
        };
}

#endif // __ROMI_RPC_CNC_SERVER_ADAPTER_H
