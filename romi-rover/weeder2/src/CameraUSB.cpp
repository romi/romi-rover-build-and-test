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

#include "CameraUSB.h"

namespace romi {
        
        bool CameraUSB::open() {
                bool success = false;
                if (_camera == 0 && _device.length() > 0
                    && _width > 0 && _height > 0) {
                        _camera = new_camera(_device.c_str(), IO_METHOD_MMAP, _width, _height);
                        if (_camera != 0) {
                                _thread = new_thread(CameraUSB::_run, this);
                                success = (_thread != 0);
                        }
                }
                return success;
        }

        int CameraUSB::set_parameter(const char *name, JSON value)
        {
                if (_camera != 0) {
                        throw std::runtime_error("CameraUSB::set_parameter called AFTER open");
                } else if (rstreq(name, "device")) {
                        _device = value.str();
                } else if (rstreq(name, "width")) {
                        _width = (uint32_t) value.num();
                        if (_width <= 10 || _width > 10000)
                                throw std::runtime_error("CameraUSB: Invalid width");
                } else if (rstreq(name, "height")) {
                        _height = (uint32_t) value.num();
                        if (_height <= 10 || _height > 10000)
                                throw std::runtime_error("CameraUSB: Invalid height");
                }
                return 0;
        }
        
        bool CameraUSB::grab(Image &image)
        {
                bool success = false;
                if (_camera != 0) {
                        r_debug("CameraUSB: camera_capture");
                        if (camera_capture(_camera) == 0) {
                                r_debug("CameraUSB: camera_getimagebuffer");
                                uint8_t *rgb = camera_getimagebuffer(_camera);
                                r_debug("CameraUSB: import_rgb");
                                success = image.import_rgb(rgb, _width, _height);

                        } else {
                                r_warn("CameraUSB: camera_capture failed");
                        }
                } else {
                        r_warn("CameraUSB: No camera link");
                }
                return success;
        }
}
