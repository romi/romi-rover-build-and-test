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
#include "Camera.h"
#include <mutex>

namespace romi {

        class CameraProxy : public Camera {
        protected:
                std::string _name;
                std::string _resource;
                std::mutex _m;

                void assert_name();
                void assert_ressource();
                void download_jpg_data(response_t **response);
                void convert_jpg_data(response_t *response, Image& image);
                
        public:
                CameraProxy() {}
                virtual ~CameraProxy() override = default;
                
                int set_parameter(const char *name, JsonCpp value) override;
                bool open() override;
                bool grab(Image &image) override;
        };
}

#endif // __ROMI_CAMERA_PROXY_H
