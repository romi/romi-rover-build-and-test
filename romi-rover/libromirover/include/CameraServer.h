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

#ifndef __ROMI_CAMERA_SERVER_H
#define __ROMI_CAMERA_SERVER_H

#include <rcom.h>
#include "api/Camera.h"

namespace romi {

        class CameraServer {
        protected:
                Camera &_camera;
                service_t *_service;
                Image _image;

                static void _send_image(void *data,
                                        request_t *request,
                                        response_t *response);
                
                void send_image(response_t *response);
                
        public:
                
                CameraServer(Camera &camera, const char *name, const char *topic);
                virtual ~CameraServer();
        };
}

#endif // __ROMI_CAMERA_SERVER_H
