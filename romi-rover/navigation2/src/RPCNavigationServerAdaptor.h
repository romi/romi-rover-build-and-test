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
#ifndef __ROMI_RPC_NAVIGATION_SERVER_ADAPTOR_H
#define __ROMI_RPC_NAVIGATION_SERVER_ADAPTOR_H

#include "IRPCHandler.h"
#include "INavigation.h"

namespace romi {
        
        class RPCNavigationServerAdaptor : public rcom::IRPCHandler
        {
        protected:
                INavigation &_navigation;

                void ok_status(JSON &result);
                void error_status(JSON &result, const char *message);
                void execute_moveat(JSON &cmd, JSON &result);
                void execute_move(JSON &cmd, JSON &result);
                void execute_stop(JSON &cmd, JSON &result);
                
        public:
                RPCNavigationServerAdaptor(INavigation &navigation)
                        : _navigation(navigation) {}
                virtual ~RPCNavigationServerAdaptor() override = default;

                void execute(JSON &cmd, JSON &result);
        };
}

#endif // __ROMI_RPC_NAVIGATION_SERVER_ADAPTOR_H