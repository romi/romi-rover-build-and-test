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

#include "ImageSegmentationFactory.h"
#include "svm/SVMSegmentation.h"

namespace romi {
        
        int32_t ImageSegmentationFactory::set_parameter(const char* key,
                                                        json_object_t value,
                                                        void *data)
        {
                IImageSegmentation *obj = (IImageSegmentation *) data;
                obj->set_parameter(key, value);
                return 0;
        }

        IImageSegmentation *ImageSegmentationFactory::create(const char *name,
                                                             JSON params)
        {
                IImageSegmentation *obj = 0;
                if (rstreq(name, "svm")) {
                        obj = new SVMSegmentation();
                } else {
                        r_warn("Failed to find the image segmentation class: %s", name);
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
