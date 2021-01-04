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

#ifndef __ROMI_CAMERA_FILE_H
#define __ROMI_CAMERA_FILE_H

#include <string>
#include "ImageIO.h"
#include "Camera.h"

namespace romi {

        class CameraFile : public Camera {
        protected:
                std::string _filename;
                Image _image;
                
        public:
                CameraFile() : _image() {}
                
                CameraFile(const char *filename) : _filename(filename), _image() {
                        open();
                }
                
                virtual ~CameraFile() override = default;
                
                int set_parameter(const char *name, JsonCpp value) override {
                        int r = 0;
                        if (rstreq(name, "file"))
                                _filename = value.str();
                        return r;
                }

                bool open() override {
                        bool success = false;
                        try {
                                success = ImageIO::load(_image, _filename.c_str());

                        } catch (std::runtime_error &e) {
                                r_err("Failed to load the file: %s", _filename.c_str());
                        }
                        return success;
                }
        
                bool grab(Image &image) override {
                        image = _image;
                        return true;
                }
        };
}

#endif // __ROMI_CAMERA_FILE_H
