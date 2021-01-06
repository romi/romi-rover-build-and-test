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

        USBCamera::USBCamera(const char *device, size_t width, size_t height)
                : _camera(0), _device(device),
                  _thread(0), _done(false)
        {
                if (_device.length() == 0)
                        throw std::runtime_error("USBCamera: Invalid device");
                        
                if (width <= 10 || width > 10000)
                        throw std::runtime_error("USBCamera: Invalid width");
                        
                if (height <= 10 || height > 10000)
                        throw std::runtime_error("USBCamera: Invalid height");
                
                if (!open(width, height))
                        throw std::runtime_error("USBCamera::open failed");
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

        bool USBCamera::open(size_t width, size_t height)
        {
                bool success = false;
                
                r_info("USBCamera::open: %s, %ux%u", _device.c_str(),
                       width, height);

                _camera = new_camera(_device.c_str(), IO_METHOD_MMAP,
                                     width, height);
                if (_camera != 0) {
                        start_capture_thread();
                        success = true;
                } else {
                        r_err("USBCamera::open: failed to create the camera");
                }
                
                return success;
        }

        void USBCamera::start_capture_thread()
        {
                _thread = new_thread(USBCamera::_run, this);
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
                        int width = camera_width(_camera);
                        int height = camera_height(_camera);
                        _image.import(Image::RGB, rgb, width, height);
                        
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
