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

#include <cv/ImageIO.h>
#include <rpc/RcomClient.h>
#include "unet/Unet.h"

namespace romi {

        Unet::Unet() : counter_(0)
        {
        }
                
        bool Unet::create_mask(Image &image, Image &mask)
        {
                bool success = false;
                try {
                        try_create_mask(image, mask);
                        success = true;
                        
                } catch (std::exception& e) {
                        r_warn("Unet::create_mask: caught exception: %s", e.what());
                }
                return success;
        }

        void Unet::try_create_mask(Image &image, Image &mask)
        {
                std::string image_path = create_image_path();
                std::string mask_path = create_mask_path();
                
                store_image(image, image_path);
                send_python_request(image_path, mask_path);
                load_mask(mask, mask_path);
        }

        void Unet::store_image(Image &image, std::string& path)
        {
                if (!ImageIO::store_jpg(image, path.c_str())) {
                        r_warn("Failed to save the image to %s", path.c_str());
                        throw std::runtime_error("Failed to save the image");
                }
        }

        std::string Unet::create_image_path()
        {
                return create_path("/tmp", "unet-image", ++counter_);
        }

        std::string Unet::create_mask_path()
        {
                return create_path("/tmp", "unet-mask", ++counter_);
        }

        std::string Unet::create_path(const std::string& dir,
                                      const std::string& prefix,
                                      int index)
        {
                char path[256];
                snprintf(path, sizeof(path), "%s/%s-%05d.jpg",
                         dir.c_str(), prefix.c_str(), index);
                return std::string(path);
        }

        void Unet::send_python_request(const std::string& image_path,
                                       const std::string& mask_path)
        {
                JsonCpp response;
                romi::RPCError error;
                
                JsonCpp params = JsonCpp::construct("{\"image\": \"%s\", "
                                                    "\"mask\": \"%s\"}",
                                                    image_path.c_str(),
                                                    mask_path.c_str());
        
                auto rpc = romi::RcomClient::create("python", 30);
                rpc->execute("unet", params, response, error);
        
                if (error.code != 0) {
                        r_warn("Failed to call Python: %s", error.message.c_str());
                        throw std::runtime_error("Failed to call Python");
                        
                } else if (response.get("error").num("code") != 0) {
                        const char *message = response.get("error").str("message");
                        r_warn("Failed to call Python: %s", message);
                        throw std::runtime_error("Failed to call Python");
                }
        }

        void Unet::load_mask(Image& mask, std::string& mask_path)
        {
                if (!ImageIO::load(mask, mask_path.c_str())) {
                        r_warn("Failed to load the mask at %s", mask_path.c_str());
                        throw std::runtime_error("Failed to load the mask");
                }
        }
}
