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
#ifndef __ROMI_CNC_ADAPTER_H
#define __ROMI_CNC_ADAPTER_H

#include "CNC.h"
#include "IRPCHandler.h"

namespace romi {
        
        class CNCAdaptor : public rcom::IRPCHandler
        {
        protected:
                CNC &_cnc;
                
                void execute_get_range(JsonCpp& params, JsonCpp& result,
                                       rcom::RPCError &error);
                void execute_moveto(JsonCpp& params, JsonCpp& result,
                                    rcom::RPCError &error);
                void execute_spindle(JsonCpp& params, JsonCpp& result,
                                     rcom::RPCError &error);
                void execute_travel(JsonCpp& params, JsonCpp& result,
                                    rcom::RPCError &error);
                void execute_homing(JsonCpp& params, JsonCpp& result,
                                    rcom::RPCError &error);
                void execute_pause(JsonCpp& params, JsonCpp& result,
                                   rcom::RPCError &error);
                void execute_continue(JsonCpp& params, JsonCpp& result,
                                      rcom::RPCError &error);
                void execute_reset(JsonCpp& params, JsonCpp& result,
                                   rcom::RPCError &error);

        public:
                CNCAdaptor(CNC &cnc) : _cnc(cnc) {}
                virtual ~CNCAdaptor() override = default;
                
                void execute(const char *method, JsonCpp& params,
                             JsonCpp& result, rcom::RPCError &error) override;
        };
}

#endif // __ROMI_CNC_ADAPTER_H
