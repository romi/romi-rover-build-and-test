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

#ifndef __ROMI_UNET_H
#define __ROMI_UNET_H

#include <string> 
#include "weeder/IImageSegmentation.h"

namespace romi {

        class Unet : public IImageSegmentation
        {
        protected:
                int counter_;

                void try_create_mask(Image &image, Image &mask);
                void store_image(Image &image, std::string& path);
                std::string create_image_path();
                std::string create_mask_path();
                std::string create_path(const std::string& dir,
                                        const std::string& prefix,
                                        int index);
                void send_python_request(const std::string& image_path,
                                         const std::string& mask_path);
                void load_mask(Image& mask, std::string& path);
                
        public:
                Unet();
                ~Unet() override = default;
                
                bool create_mask(Image &image, Image &mask) override;
        };
}

#endif // __ROMI_UNET_H
