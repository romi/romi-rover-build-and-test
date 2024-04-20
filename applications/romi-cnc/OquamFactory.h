/*
  romi-rover

  Copyright (C) 2019-2020 Sony Computer Science Laboratories
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

#ifndef __OQUAM_FACTORY_H
#define __OQUAM_FACTORY_H

#include <memory>
#include <RomiSerialClient.h>
#include <RSerial.h>

#include "configuration/IOptions.h"
#include "oquam/ICNCController.h"
#include <json.hpp>

namespace romi {

        class OquamFactory
        {
        protected:
                std::unique_ptr<ICNCController> _controller;
                
                void instantiate_controller(IOptions &options, nlohmann::json &config);
                void instantiate_controller(const std::string& controller_classname,
                                            IOptions &options, nlohmann::json &config);
                std::string get_controller_classname_in_config(nlohmann::json &config);
                void instantiate_fake_controller();
                void instantiate_stepper_controller(IOptions &options, nlohmann::json &config);
                std::string get_stepper_controller_device(IOptions &options,
                                                          nlohmann::json &config);
                std::string get_stepper_controller_device_in_config(nlohmann::json &config);

        public:

                OquamFactory();
                virtual ~OquamFactory() = default;
                
                ICNCController& create_controller(IOptions& options, nlohmann::json& config);

        };
}

#endif // __OQUAM_FACTORY_H
