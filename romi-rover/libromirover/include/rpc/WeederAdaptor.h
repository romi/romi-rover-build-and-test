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

#ifndef __ROMI_WEEDER_ADAPTOR_H
#define __ROMI_WEEDER_ADAPTOR_H

#include <string>
#include <IRPCHandler.h>
#include "api/Weeder.h"

namespace romi {

        class WeederAdaptor : public rcom::IRPCHandler
        {
        protected:
                Weeder& _weeder;

                void execute_hoe(rcom::RPCError &error);
                void execute_stop(rcom::RPCError &error);
                void execute_pause(rcom::RPCError &error);
                void execute_continue(rcom::RPCError &error);
                void execute_reset(rcom::RPCError &error);
                void execute_power_up(rcom::RPCError &error);
                void execute_power_down(rcom::RPCError &error);
                void execute_stand_by(rcom::RPCError &error);
                void execute_wake_up(rcom::RPCError &error);

        public:
                WeederAdaptor(Weeder& weeder) : _weeder(weeder) {
                }
                
                virtual ~WeederAdaptor() override = default;

                void execute(const char *method, JsonCpp& params,
                             JsonCpp& result, rcom::RPCError &error) override;
        };
}

#endif // __ROMI_WEEDER_ADAPTOR_H
