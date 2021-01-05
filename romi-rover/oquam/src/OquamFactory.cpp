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
#include "OquamFactory.h"
#include "FakeCNCController.h"
#include "StepperController.h"

using namespace std;

namespace romi {
        
        OquamFactory::OquamFactory()
                : _serial(0)
        {}
        
        OquamFactory::~OquamFactory()
        {
                if (_serial)
                        delete _serial;
        }
        
        CNCController& OquamFactory::create_controller(Options &options,
                                                       JsonCpp &config)
        {
                if (_controller == 0)
                        instantiate_controller(options, config);
                return *_controller;
        }

        void OquamFactory::instantiate_controller(Options &options, JsonCpp &config)
        {
                const char *classname = get_controller_classname(options, config);
                instantiate_controller(classname, options, config);
        }
        
        const char *OquamFactory::get_controller_classname(Options &options,
                                                           JsonCpp &config)
        {
                const char *controller_classname = options.get_value("oquam-controller-classname");
                if (controller_classname == 0) {
                        controller_classname = get_controller_classname_in_config(config);
                } 
                return controller_classname;
        }
        
        const char *OquamFactory::get_controller_classname_in_config(JsonCpp &config)
        {
                const char *controller_classname = 0;
                try {
                        controller_classname = config["oquam"]["controller-classname"];
                        
                } catch (JSONError &je) {
                        r_warn("Failed to get the value for "
                               "oquam.controller-classname: %s", je.what());
                        throw std::runtime_error("No controller classname defined");
                }
                return controller_classname;
        }
        
        void OquamFactory::instantiate_controller(const char *classname,
                                                  Options &options,
                                                  JsonCpp &config)
        {
                r_info("create_controller: Creating an instance of '%s'", classname);
                
                if (rstreq(classname, FakeCNCController::ClassName)) {
                        instantiate_fake_controller();

                } else if (rstreq(classname, StepperController::ClassName)) {
                        instantiate_stepper_controller(options, config);
                        
                } else {
                        r_err("Unknown controller class: '%s'", classname);
                        throw std::runtime_error("Failed to create the controller");
                }
        }
        
        void OquamFactory::instantiate_fake_controller()
        {
                _controller = std::unique_ptr<CNCController>(new FakeCNCController());
        }
        
        void OquamFactory::instantiate_stepper_controller(Options &options,
                                                          JsonCpp &config)
        {
                const char *device = get_stepper_controller_device(options, config);
                _serial = new RSerial(device, 115200, 1);
                _romi_serial = unique_ptr<RomiSerialClient>(new RomiSerialClient(_serial, _serial));
                _romi_serial->set_debug(true);
                _controller = std::unique_ptr<CNCController>(new StepperController(*_romi_serial));
        }

        const char *OquamFactory::get_stepper_controller_device(Options &options,
                                                                JsonCpp &config)
        {
                const char *device_name = options.get_value("oquam-controller-device");
                if (device_name == 0) {
                        device_name = get_stepper_controller_device_in_config(config);
                }
                return device_name;
        }
        
        const char *OquamFactory::get_stepper_controller_device_in_config(JsonCpp &config)
        {
                const char *device_name = 0;
                try {
                        device_name = config["ports"]["oquam"]["port"];
                        
                } catch (JSONError &je) {
                        r_warn("Failed to get the value for "
                               "ports.oquam.port: %s", je.what());
                        throw std::runtime_error("No crystal controller device defined");
                }
                return device_name;
        }
        
}
