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
#include "rpc/RemoteWeeder.h"
#include "rpc/MethodsRover.h"

namespace romi {
        
        RemoteWeeder::RemoteWeeder(rcom::IRPCHandler &client) : RemoteStub(client)
        {
        }
        
        bool RemoteWeeder::hoe()
        {
                r_debug("RemoteWeeder::hoe");
                return execute_simple_request(MethodsWeeder::hoe);
        }
        
        bool RemoteWeeder::stop()
        {
                r_debug("RemoteWeeder::hoe");
                return execute_simple_request(MethodsWeeder::stop);
        }

        bool RemoteWeeder::pause_activity()
        {
                r_debug("RemoteWeeder::stop_activity");
                return execute_simple_request(MethodsActivity::activity_pause);
        }

        bool RemoteWeeder::continue_activity()
        {
                r_debug("RemoteWeeder::continue_activity");
                return execute_simple_request(MethodsActivity::activity_continue);
        }

        bool RemoteWeeder::reset_activity()
        {
                r_debug("RemoteWeeder::reset");
                return execute_simple_request(MethodsActivity::activity_reset);
        }

        bool RemoteWeeder::power_up()
        {
                r_debug("RemoteWeeder::power_up");
                return execute_simple_request(MethodsPowerDevice::power_up);
        }
        
        bool RemoteWeeder::power_down()
        {
                r_debug("RemoteWeeder::power_down");
                return execute_simple_request(MethodsPowerDevice::power_down);
        }
        
        bool RemoteWeeder::stand_by()
        {
                r_debug("RemoteWeeder::stand_by");
                return execute_simple_request(MethodsPowerDevice::stand_by);
        }
        
        bool RemoteWeeder::wake_up()
        {
                r_debug("RemoteWeeder::wake_up");
                return execute_simple_request(MethodsPowerDevice::wake_up);
        }
}
