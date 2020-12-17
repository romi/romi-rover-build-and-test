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

#include "ImageCropperFactory.h"
#include "ImageCropper.h"

namespace romi {
        
        int32_t ImageCropperFactory::set_parameter(const char* key,
                                                   json_object_t value,
                                                   void *data)
        {
                IImageCropper *obj = (IImageCropper *) data;
                return obj->set_parameter(key, value);
        }

        IImageCropper *ImageCropperFactory::create(const char *name,
                                                   CNCRange &range,
                                                   JsonCpp params)
        {
                IImageCropper *obj = 0;
                if (rstreq(name, "imagecropper")) {
                        obj = new ImageCropper();
                        obj->set_range(range);
                } else {
                        r_warn("Failed to find the image cropper class: %s", name);
                }

                if (obj != 0) {
                        if (params.foreach(set_parameter, obj) != 0) {
                                delete obj;
                                obj = 0;
                        }
                }
                return obj;
        }
}
