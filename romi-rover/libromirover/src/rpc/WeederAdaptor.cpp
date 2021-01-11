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
#include "rpc/MethodsWeeder.h"

namespace romi {

        void WeederAdaptor::try_hoe(rcom::RPCError &error)
        {
                error.code = 0;
                if (!_weeder.hoe()) {
                        error.code = 1;
                        error.message = "Hoe failed";
                }
        }

        void WeederAdaptor::try_stop(rcom::RPCError &error)
        {
                error.code = 0;
                if (!_weeder.stop()) {
                        error.code = 1;
                        error.message = "Stop failed";
                }
        }
        
        void WeederAdaptor::execute(const char *method, JsonCpp& params,
                                    JsonCpp& result, rcom::RPCError &error)
        {
                r_debug("WeederAdaptor::execute");
                
                if (rstreq(method, MethodsWeeder::hoe)) {
                        try_hoe(error);
                        
                } else if (rstreq(method, MethodsWeeder::stop)) {
                        try_hoe(error);
                        
                } else {
                        error.code = rcom::RPCError::MethodNotFound;
                        error.message = "Unknown method";
                }
        }
}
