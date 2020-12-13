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

#ifndef __ROMI_I_CAMERA_USB_H
#define __ROMI_I_CAMERA_USB_H

#include <string>
#include <r.h>
#include "ICamera.h"
#include "Image.h"
#include "JSON.h"

#include "camera_v4l.h"

namespace romi {

        class CameraUSB : public ICamera {
        protected:
                camera_t* _camera;
                std::string _device;
                uint32_t _width;
                uint32_t _height;
                mutex_t *_mutex;
                thread_t *_thread;
                bool _done;
                
                void close();

                static void _run(void* data) {
                        CameraUSB *camera = (CameraUSB*) data;
                        camera->run();
                }

                void run() {
                        while (!_done) {
                                mutex_lock(_mutex);
                                camera_capture(_camera);
                                mutex_unlock(_mutex);
                                clock_sleep(0.040);
                        }
                }
                        
        public:
                CameraUSB() : _camera(0), _width(0), _height(0), _thread(0), _done(false) {
                        _mutex = new_mutex();
                }
                
                virtual ~CameraUSB() override {
                        if (_thread) {
                                _done = true;
                                thread_join(_thread);
                                delete_thread(_thread);
                        }
                        if (_camera)
                                delete_camera(_camera);
                        if (_mutex)
                                delete_mutex(_mutex);
               }
                
                int set_parameter(const char *name, JSON value) override;
                bool open() override;
                bool grab(Image &image) override;
        };
}

#endif // __ROMI_I_CAMERA_USB_H
