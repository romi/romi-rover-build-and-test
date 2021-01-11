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
#include "rpc/RemoteStub.h"

namespace romi {

        bool RemoteStub::execute(const char *method,
                                JsonCpp& params,
                                JsonCpp& result)
        {
                rcom::RPCError error;
                
                try {
                        _client.execute(method, params, result, error);

                        if (error.code != 0) {
                                r_err("RemoteStub::execute: %s",
                                      error.message.c_str());
                        }
                        
                } catch (std::exception &e) {
                        
                        r_err("RemoteStub::execute: '%s'", e.what());
                        error.code = 1;
                        error.message = e.what();
                }

                return (error.code == 0);
        }

        bool RemoteStub::execute_with_result(const char *method, JsonCpp& result)
        {
                JsonCpp params;
                return execute(method, params, result);
        }

        bool RemoteStub::execute_with_params(const char *method, JsonCpp& params)
        {
                JsonCpp result;
                return execute(method, params, result);
        }

        bool RemoteStub::execute_simple_request(const char *method)
        {
                JsonCpp params;
                JsonCpp result;
                return execute(method, params, result);
        }
}
