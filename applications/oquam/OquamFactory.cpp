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
#include <stdexcept>
#include <oquam/FakeCNCController.h>
#include <oquam/StepperController.h>
#include <rover/RoverOptions.h>
#include <util/Logger.h>
#include <util/RomiSerialLog.h>
#include "OquamFactory.h"

using namespace std;

namespace romi {
        
        OquamFactory::OquamFactory() : _controller()
        {}
        
        ICNCController& OquamFactory::create_controller(IOptions &options,
                                                       nlohmann::json &config)
        {
                if (_controller == nullptr)
                        instantiate_controller(options, config);
                return *_controller;
        }

        void OquamFactory::instantiate_controller(IOptions &options, nlohmann::json &config)
        {
                std::string classname = get_controller_classname_in_config(config);
                instantiate_controller(classname, options, config);
        }

        std::string OquamFactory::get_controller_classname_in_config(nlohmann::json &config)
        {
                std::string controller_classname ;
                try {
                        controller_classname = config["oquam"]["controller-classname"];
                        
                } catch (nlohmann::json::exception& je) {
                        r_warn("Failed to get the value for "
                               "oquam.controller-classname: %s", je.what());
                        throw std::runtime_error("No controller classname defined");
                }
                return controller_classname;
        }
        
        void OquamFactory::instantiate_controller(const std::string& classname,
                                                  IOptions &options,
                                                  nlohmann::json &config)
        {
                r_info("create_controller: Creating an instance of '%s'", classname.c_str());
                
                if (classname == FakeCNCController::ClassName) {
                        instantiate_fake_controller();

                } else if (classname == StepperController::ClassName) {
                        instantiate_stepper_controller(options, config);
                        
                } else {
                        r_err("Unknown controller class: '%s'", classname.c_str());
                        throw std::runtime_error("Failed to create the controller");
                }
        }
        
        void OquamFactory::instantiate_fake_controller()
        {
                _controller = std::unique_ptr<ICNCController>(new FakeCNCController());
        }
        
        void OquamFactory::instantiate_stepper_controller(IOptions &options,
                                                          nlohmann::json &config)
        {
                std::string device = get_stepper_controller_device(options, config);
                std::shared_ptr<romiserial::ILog> log
                        = std::make_shared<romi::RomiSerialLog>();
                auto romi_serial = romiserial::RomiSerialClient::create(device,
                                                                        "oquam", log);
                _controller = std::make_unique<StepperController>(romi_serial);
        }

        std::string OquamFactory::get_stepper_controller_device(IOptions &options,
                                                                nlohmann::json &config)
        {
                std::string device_name = options.get_value(RoverOptions::cnc_device);
                if (device_name.empty()) {
                        device_name = get_stepper_controller_device_in_config(config);
                }
                return device_name;
        }

        std::string OquamFactory::get_stepper_controller_device_in_config(nlohmann::json &config)
        {
                std::string device_name;
                try {
                        device_name = config["ports"]["oquam"]["port"];
                        
                } catch (nlohmann::json::exception &je) {
                        r_warn("Failed to get the value for "
                               "ports.oquam.port: %s", je.what());
                        throw std::runtime_error("No crystal controller device defined");
                }
                return device_name;
        }
        
}
