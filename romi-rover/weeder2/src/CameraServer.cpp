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

#include "CameraServer.h"

namespace romi {

        CameraServer::CameraServer(Camera &camera, const char *name, const char *topic)
                : _camera(camera), _service(0)
        {
                _service = registry_open_service(name, topic, 0);
                
                if (_service == NULL) {
                        r_err("Failed to create the service");
                        throw std::runtime_error("Failed to create the service");
                };
                
                int err = service_export(_service, "camera.jpg", 0,
                                         "image/jpeg", this, _send_image);
                if (err) {
                        r_err("service_export failed");
                        throw std::runtime_error("service_export failed");
                }
        }
                
        CameraServer::~CameraServer()
        {
                if (_service)
                        registry_close_service(_service);
        }

        void CameraServer::_send_image(void *data,
                                       request_t *request,
                                       response_t *response)
        {
                (void) request;
                CameraServer *server = (CameraServer *) data;
                server->send_image(response);
        }
                
        void CameraServer::send_image(response_t *response)
        {
                if (_camera.grab(_image)) {
                        if (!_image.to_jpeg(response_body(response))) {
                                r_err("CameraServer::send_image: _image.to_jpeg failed");
                                response_set_status(response, HTTP_Status_Internal_Server_Error);
                        }
                        
                } else {
                        r_err("CameraServer::send_image: _camera.grab failed");
                        response_set_status(response, HTTP_Status_Internal_Server_Error);
                }
        }
}
