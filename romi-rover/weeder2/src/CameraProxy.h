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
#include <rcom.h>
#include "ICamera.h"

namespace romi {

        class CameraProxy : public ICamera {
        protected:
                std::string _name;
                std::string _resource;
                
        public:
                CameraProxy() {}
                virtual ~CameraProxy() override = default;
                
                int set_parameter(const char *name, JSON value) override {
                        int r = 0;
                        if (rstreq(name, "name")) {
                                _name = value.str();
                        } else if (rstreq(name, "resource")) {
                                _resource = value.str();
                        }
                        return r;
                }

                bool open() override {
                        // Check that the download actually works.
                        Image image;
                        return grab(image);
                }
                
                bool grab(Image &image) {
                        bool success = false;
                        response_t *response = NULL;
                        
                        if (_name.size() > 0 && _resource.size() > 0) {
                                int err = client_get_data(_name.c_str(),
                                                          _resource.c_str(),
                                                          &response);
                                if (err == 0) {
                                        membuf_t *body = response_body(response);
                                        const unsigned char *data;
                                        data = (const unsigned char *) membuf_data(body);
                                        try {
                                                image.load_from_mem(data, membuf_len(body));
                                                success = true;
                                        
                                        } catch (std::exception& e) {
                                                delete_response(response);
                                                r_err("CameraProxy:: %s", e.what());
                                        }
                                        delete_response(response);
       
                                } else {
                                        r_err("CameraProxy: Failed to get the "
                                              "camera image");
                                }
                        } else {
                                r_err("CameraProxy: Name or resource of remote "
                                      "camera is missing");
                        }
                        
                        return success;
                }
        };
}

#endif // __ROMI_CAMERA_PROXY_H
