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

#include "CameraFactory.h"
#include "FileCamera.h"
#include "USBCamera.h"

namespace romi {

        int32_t CameraFactory::set_parameter(const char* key,
                                             json_object_t value,
                                             void *data)
        {
                Camera *obj = (Camera *) data;
                return obj->set_parameter(key, value);
        }

        Camera *CameraFactory::create(const char *name, JsonCpp config)
        {
                Camera *camera = 0;
                if (rstreq(name, "file-camera")) {
                        camera = new FileCamera();
                } else if (rstreq(name, "usb-camera")) {
                        camera = new USBCamera();
                } else {
                        r_warn("Failed to find the image class: %s", name);
                }

                if (camera != 0) {
                        if (config.foreach(set_parameter, camera) != 0
                            || !camera->open()) {
                                delete camera;
                                camera = 0;
                        }
                }
                return camera;
        }
}
