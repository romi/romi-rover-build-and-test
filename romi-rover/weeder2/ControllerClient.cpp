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
#include <stdexcept>
#include "ControllerClient.h"

namespace romi {
        
        ControllerClient::ControllerClient(const char *name, const char *topic)
        {
                _link = registry_open_messagelink(name, topic,
                                                  (messagelink_onmessage_t) NULL, NULL);
                if (_link == 0)
                        throw std::runtime_error("Failed to create the messagelink");
        }
        
        ControllerClient::~ControllerClient()
        {
                if (_link)
                        registry_close_messagelink(_link);
        }

        void ControllerClient::assure_ok(JsonCpp reply)
        {
                r_debug("ControllerClient::assure_ok");
                const char *status = reply.str("status");
                if (rstreq(status, "error")) {
                        const char *message = reply.str("message");
                        r_err("ControllerClient::assure_ok: message: %s", message);
                        throw std::runtime_error(message);
                }
        }

        JsonCpp ControllerClient::execute(JsonCpp cmd)
        {
                r_debug("ControllerClient::execute");
                JsonCpp reply = messagelink_send_command(_link, cmd.ptr());
                assure_ok(reply);
                return reply;
        }
}
