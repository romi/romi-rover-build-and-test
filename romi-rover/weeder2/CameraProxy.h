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

#ifndef __ROMI_CAMERA_PROXY_H
#define __ROMI_CAMERA_PROXY_H

#include <string>
#include "ICamera.h"
#include "Image.h"

namespace romi {

        class CameraProxy : public ICamera {
        protected:
                std::string _name;
                std::string _resource;
                
        public:
                CameraProxy(const char *name, const char *resource)
                        : _name(name), _resource(resource) {}
                
                virtual ~CameraProxy() override = default;
                
                void grab(Image &image) {
                        response_t *response = NULL;
                        int err = client_get_data("camera", "camera.jpg", &response);
                        if (err == 0) {
                                membuf_t *body = response_body(response);
                                const unsigned char *data;
                                data = (const unsigned char *) membuf_data(body);
                                try {
                                        image.load_from_mem(data, membuf_len(body));
                                } catch (std::exception e) {
                                        delete_response(response);
                                        throw e;
                                }
                                delete_response(response);
       
                        } else {
                                r_err("Failed to get the camera image");
                                throw std::runtime_error("Failed to get the image");
                        }
                }
        };
}

#endif // __ROMI_CAMERA_PROXY_H
