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

#include "ImageIO.h"
#include "CameraProxy.h"

namespace romi {

        int CameraProxy::set_parameter(const char *name, JsonCpp value)
        {
                int r = 0;
                if (rstreq(name, "name")) {
                        _name = value.str();
                } else if (rstreq(name, "resource")) {
                        _resource = value.str();
                }
                return r;
        }

        bool CameraProxy::open()
        {
                // Check that the download actually works.
                Image image;
                return grab(image);
        }
                
        bool CameraProxy::grab(Image &image)
        {
                std::lock_guard<std::mutex> sync(_m);
                
                bool success = false;
                response_t *response = NULL;

                try {
                        assert_name();
                        assert_ressource();
                        download_jpg_data(&response);
                        convert_jpg_data(response, image);
                        success = true;

                } catch (std::runtime_error& re) {
                        r_err("CameraProxy::grab: %s", re.what());
                }

                if (response != 0)
                        delete_response(response);
                
                return success;
        }
        
        void CameraProxy::assert_name()
        {
                if (_name.size() == 0) {
                        r_err("CameraProxy: Name of remote is missing");
                        throw std::runtime_error("Missing name");
                }
        }
        
        void CameraProxy::assert_ressource()
        {
                if (_resource.size() == 0) {
                        r_err("CameraProxy: Remote ressource is missing");
                        throw std::runtime_error("Missing ressource");
                }
        }
        
        void CameraProxy::download_jpg_data(response_t **response)
        {
                int err = client_get_data(_name.c_str(),
                                          _resource.c_str(),
                                          response);
                if (err != 0) {
                        r_err("CameraProxy: Failed to download the camera image");
                        throw std::runtime_error("Download failed");
                }
        }
        
        void CameraProxy::convert_jpg_data(response_t *response, Image& image)
        {
                membuf_t *body = response_body(response);
                const uint8_t *data = (const uint8_t *) membuf_data(body);
                bool success = ImageIO::load_jpg(image, data, membuf_len(body));
                if (!success) {
                        throw std::runtime_error("Conversion failed");
                }
        }
}
