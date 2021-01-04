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

#include "USBCamera.h"

namespace romi {
        
        using SynchonizedCodeBlock = std::lock_guard<std::mutex>;

        USBCamera::USBCamera()
                : _camera(0), _width(0), _height(0), _thread(0), _done(false) {
        }

        USBCamera::~USBCamera()
        {
                if (_thread) {
                        _done = true;
                        thread_join(_thread);
                        delete_thread(_thread);
                }
                if (_camera)
                        delete_camera(_camera);
        }

        int USBCamera::set_parameter(const char *name, JsonCpp value)
        {
                if (_camera != 0) {
                        throw std::runtime_error("USBCamera::set_parameter called "
                                                 "AFTER open");                        
                }
                
                if (rstreq(name, "device")) {
                        _device = value.str();
                        
                } else if (rstreq(name, "width")) {
                        _width = (uint32_t) value.num();
                        if (_width <= 10 || _width > 10000)
                                throw std::runtime_error("USBCamera: Invalid width");
                        
                } else if (rstreq(name, "height")) {
                        _height = (uint32_t) value.num();
                        if (_height <= 10 || _height > 10000)
                                throw std::runtime_error("USBCamera: Invalid height");
                }
                return 0;
        }

        bool USBCamera::open()
        {
                bool success = false;
                if (_camera == 0 && _device.length() > 0
                    && _width > 0 && _height > 0) {
                        
                        _camera = new_camera(_device.c_str(), IO_METHOD_MMAP,
                                             _width, _height);
                        if (_camera != 0) {
                                _thread = new_thread(USBCamera::_run, this);
                                success = true;
                        }
                }
                return success;
        }

        void USBCamera::_run(void* data)
        {
                USBCamera *camera = (USBCamera*) data;
                camera->run();
        }

        void USBCamera::run()
        {
                while (!_done) {
                        grab_from_camera();
                        clock_sleep(0.020);
                }
        }

        void USBCamera::grab_from_camera()
        {
                SynchonizedCodeBlock synchonized(_mutex);
                
                if (camera_capture(_camera) == 0) {
                        uint8_t *rgb = camera_getimagebuffer(_camera);
                        _image.import(Image::RGB, rgb, _width, _height);
                } else {
                        r_warn("USBCamera: camera_capture failed");
                }
        }
        
        bool USBCamera::grab(Image &image)
        {
                SynchonizedCodeBlock synchonized(_mutex);
                _image.copy_to(image);
                return true;
        }
}
