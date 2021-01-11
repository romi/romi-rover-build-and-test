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

#include "FileCamera.h"

namespace romi {

        FileCamera::FileCamera(const char *filename)
                : _filename(filename), _image()
        {
                if (_filename.length() == 0)
                        throw std::runtime_error("FileCamera: Invalid filename");
                
                if (!open()) {
                        r_err("Failed to load the file: %s", _filename.c_str());
                        throw std::runtime_error("FileCamera::open failed");
                }
        }

        bool FileCamera::open()
        {
                return ImageIO::load(_image, _filename.c_str());
        }
        
        bool FileCamera::grab(Image &image)
        {
                _image.copy_to(image);
                return true;
        }

}
