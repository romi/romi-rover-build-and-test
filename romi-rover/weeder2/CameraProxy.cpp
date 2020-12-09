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
#include "CameraProxy.h"

namespace romi {

        Image *CameraProxy::grab()
        {
                response_t *response = 0;
                Image *image = 0;
                
                int err = client_get_data(_name.c_str(), _resource.c_str(), &response);
                if (err != 0)
                        throw std::runtime_error("Failed to obtain the camera image");

                try {
                        membuf_t *body = response_body(response);
                        image = new Image((const unsigned char *)membuf_data(body),
                                          membuf_len(body));
                } catch (std::exception e) {
                        // FIXME
                        delete_response(response);
                        throw e;
                }
                
                delete_response(response);
                return image;
        }
}
