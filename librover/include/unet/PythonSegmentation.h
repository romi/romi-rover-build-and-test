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

#ifndef __ROMI_PYTHON_SEGMENTATION_H
#define __ROMI_PYTHON_SEGMENTATION_H

#include <string>
#include <rpc/RcomClient.h>
#include "weeder/IImageSegmentation.h"

namespace romi {

        class PythonSegmentation : public IImageSegmentation
        {
        protected:

                static constexpr const char *kDefaultImageName = "segmentation-image.jpg";
                static constexpr const char *kDefaultMaskName = "segmentation-mask";

                std::unique_ptr<IRPCClient> rpc_;
                
                std::string function_name_;
                
                void try_create_mask(ISession &session, Image &image, Image &mask);
                void store_image(ISession &session, Image &image);
                void send_python_request(const std::string& image_path,
                                         const std::string& output_name);
                void load_mask(ISession &session, Image& mask);
                std::string get_image_path(ISession &session);
                
                void assert_connected_to_python();
                void connect_to_python();
                void disconnect_from_python();
                
        public:
                PythonSegmentation(const std::string& function_name);
                ~PythonSegmentation() override = default;
                
                bool create_mask(ISession &session, Image &image, Image &mask) override;
        };
}

#endif // __ROMI_PYTHON_SVM_H
