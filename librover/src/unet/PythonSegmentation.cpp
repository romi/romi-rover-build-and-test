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
#include "unet/PythonSegmentation.h"

namespace romi {

        PythonSegmentation::PythonSegmentation(const std::string& function_name)
                : rpc_(),
                  function_name_(function_name)
        {
        }

        void PythonSegmentation::assert_connected_to_python()
        {
                if (rpc_== nullptr) {
                        r_err("PythonSegmentation: No RPC connection.");
                        throw std::runtime_error("No RPC connection.");
                }
        }

        void PythonSegmentation::connect_to_python()
        {
                // If there's still a rpc connection open, close it
                // now.
                disconnect_from_python();

                // Establish a new RPC connection.
                rpc_ = romi::RcomClient::create("python", 30);
                
                assert_connected_to_python();
        }
        
        void PythonSegmentation::disconnect_from_python()
        {
                if (rpc_) {
                        rpc_ = nullptr;
                }
        }

        bool PythonSegmentation::create_mask(ISession &session, Image &image, Image &mask)
        {
                bool success = false;
                try {
                        try_create_mask(session, image, mask);
                        success = true;
                        
                } catch (std::exception& e) {
                        r_warn("PythonSegmentation::create_mask: caught exception: %s",
                               e.what());
                }
                return success;
        }

        void PythonSegmentation::try_create_mask(ISession &session, Image &image,
                                                 Image &mask)
        {
                store_image(session, image);
                std::string path = get_image_path(session);
                connect_to_python();
                send_python_request(path, kDefaultMaskName);
                disconnect_from_python();
                load_mask(session, mask);
        }

        std::string PythonSegmentation::get_image_path(ISession &session)
        {
                std::filesystem::path dir = session.current_path();
                std::filesystem::path path = dir /= kDefaultImageName;
                session.current_path();
                return path.string();
        }

        void PythonSegmentation::store_image(ISession &session, Image &image)
        {
                if (!session.store_jpg(kDefaultImageName, image)) {
                        throw std::runtime_error("Failed to save the image");
                }
        }

        void PythonSegmentation::send_python_request(const std::string& path,
                                                     const std::string& output_name)
        {
                JsonCpp response;
                romi::RPCError error;
                
                r_debug("PythonSegmentation: @1");
                JsonCpp params = JsonCpp::construct("{\"path\": \"%s\", "
                                                    "\"output-name\": \"%s\"}",
                                                    path.c_str(), output_name.c_str());
                r_debug("PythonSegmentation: @2");
                assert_connected_to_python();

                r_debug("PythonSegmentation: @3");
                rpc_->execute(function_name_, params, response, error);
                
                r_debug("PythonSegmentation: @4");
                if (error.code != 0) {
                        r_warn("Failed to call Python: %s", error.message.c_str());
                        throw std::runtime_error("Failed to call Python");
                        
                } else if (response.get("error").num("code") != 0) {
                        const char *message = response.get("error").str("message");
                        r_warn("Failed to call Python: %s", message);
                        throw std::runtime_error("Failed to call Python");
                }
                r_debug("PythonSegmentation: @5");
        }
                
        void PythonSegmentation::load_mask(ISession &session, Image& mask)
        {
                std::string filename = kDefaultMaskName;
                filename += ".png";
                std::filesystem::path dir = session.current_path();
                std::filesystem::path path = dir /= filename;
                
                r_info("PythonSegmentation: loading mask %s", path.string().c_str());
                
                if (!ImageIO::load(mask, path.string().c_str())) {
                        r_warn("Failed to load the mask at %s", path.string().c_str());
                        throw std::runtime_error("Failed to load the mask");
                }
        }
}
