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

#include <functional>
#include "unet/UnetImager.h"

namespace romi {
        
        UnetImager::UnetImager(ISession& session, ICamera& camera)
                : PythonUnet(), Imager(session, camera) 
        {
        }

        std::string UnetImager::make_output_name()
        {
                char buffer[64];
                snprintf(buffer, sizeof(buffer), "mask-%06zu", counter_);
                return std::string(buffer);
        }
         
        bool UnetImager::grab()
        {
                bool success = false;
                if (Imager::grab()) {
                        std::string path = make_image_name();
                        std::string name = make_output_name();
                        std::thread t([this, path, name]() {
                                        send_python_request(path, name);
                                });
                        t.detach();
                        success = true;
                }
                return success;
        }
}
