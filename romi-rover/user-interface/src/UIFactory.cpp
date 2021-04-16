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

#include <memory>
#include <stdexcept>

#include "configuration/ConfigurationProvider.h"
#include <CrystalDisplay.h>
#include <JoystickInputDevice.h>
#include <rover/RoverOptions.h>
#include <rpc/RemoteNavigation.h>
#include <rpc/RemoteWeeder.h>
#include <fake/FakeInputDevice.h>
#include <fake/FakeDisplay.h>
#include <fake/FakeNavigation.h>
#include <fake/FakeWeeder.h>

#include "UIFactory.h"

using namespace std;
using namespace rcom;

namespace romi {
        
        UIFactory::UIFactory()
                : _linux(), _joystick_event_mapper(), _serial(), _romi_serial(),
                  _joystick(), _display(), _navigation(), _input_device(), _weeder()
        {}
        
        UIFactory::~UIFactory() = default;
        
        IDisplay& UIFactory::create_display(Options &options, JsonCpp &config)
        {
                if (_display == nullptr)
                        instantiate_display(options, config);
                return *_display;
        }

        void UIFactory::instantiate_display(Options &options, JsonCpp &config)
        {
                std::string classname = get_display_classname(config);
                instantiate_display(classname, options, config);
        }

        std::string UIFactory::get_display_classname(JsonCpp &config)
        {
                std::string display_classname;
                try {
                        display_classname = (const char *)config["user-interface"]["display-classname"];
                        
                } catch (JSONError &je) {
                        r_warn("Failed to get the value for "
                               "user-interface.display-classname: %s", je.what());
                        throw std::runtime_error("No display classname defined");
                }
                return display_classname;
        }
        
        void UIFactory::instantiate_display(const std::string& classname,
                                            Options &options,
                                            JsonCpp &config)
        {
                r_info("create_display: Creating an instance of '%s'", classname.c_str());
                
                if (classname == FakeDisplay::ClassName) {
                        instantiate_fake_display();

                } else if (classname == CrystalDisplay::ClassName) {
                        instantiate_crystal_display(options, config);
                        
                } else {
                        r_err("Unknown display class: '%s'", classname.c_str());
                        throw std::runtime_error("Failed to create the display");
                }
        }
        
        void UIFactory::instantiate_fake_display()
        {
                _display = std::unique_ptr<IDisplay>(new FakeDisplay());
        }
        
        void UIFactory::instantiate_crystal_display(Options &options, JsonCpp &config)
        {
                std::string device = get_crystal_display_device(options, config);
                _serial = make_shared<RSerial>(device, 115200, 1);
                _romi_serial = std::make_unique<RomiSerialClient>(_serial,_serial);
                _romi_serial->set_debug(true);
                _display = std::unique_ptr<IDisplay>(new CrystalDisplay(*_romi_serial));
        }

        std::string UIFactory::get_crystal_display_device(Options &options,
                                                          JsonCpp &config)
        {
                std::string device_name = options.get_value("display-device");
                if (device_name.empty()) {
                        device_name = get_crystal_display_device_in_config(config);
                }
                return device_name;
        }

        std::string UIFactory::get_crystal_display_device_in_config(JsonCpp &config)
        {
                std::string device_name;
                try {
                        device_name = (const char *)config["ports"]["crystal-display"]["port"];
                        
                } catch (JSONError &je) {
                        r_warn("Failed to get the value for "
                               "ports.crystal-display.port: %s", je.what());
                        throw std::runtime_error("No crystal display device defined");
                }
                return device_name;
        }
        
        INavigation& UIFactory::create_navigation(Options &options, JsonCpp &config)
        {
                if (_navigation == nullptr) {
                        instantiate_navigation(options, config);
                }
                return *_navigation;
        }
        
        void UIFactory::instantiate_navigation(Options &options, JsonCpp &config)
        {
                std::string classname = get_navigation_classname(config);
                instantiate_navigation(classname, options, config);
        }

        std::string UIFactory::get_navigation_classname(JsonCpp &config)
        {
                std::string classname;
                try {
                        classname = (const char *)config["user-interface"]["navigation-classname"];
                } catch (JSONError &je) {
                        r_warn("Failed to get the value for "
                               "user-interface.navigation-classname: %s", je.what());
                        throw std::runtime_error("No navigation classname defined");
                }
                return classname;
        }
        
        void UIFactory::instantiate_navigation(const std::string& classname,
                                              Options &options,
                                              JsonCpp &config)
        {
                r_info("Create an instance of navigation '%s'", classname.c_str());
                
                if (classname == FakeNavigation::ClassName) {
                        instantiate_fake_navigation();

                } else if (classname == RemoteNavigation::ClassName) {
                        instantiate_remote_navigation(options, config);
                        
                } else {
                        r_err("Unknown navigation class: '%s'", classname.c_str());
                        throw std::runtime_error("Failed to create the navigation module");
                }
        }
                
        void UIFactory::instantiate_fake_navigation()
        {
                _navigation = unique_ptr<INavigation>(new FakeNavigation());
        }

        void UIFactory::instantiate_remote_navigation(__attribute__((unused))Options &options, __attribute__((unused))JsonCpp &config)
        {
                std::shared_ptr<rcom::IRPCHandler> navigationclient = std::make_shared<RPCClient>("navigation",
                                                                         "navigation",
                                                                         10.0);
                _navigation = std::make_unique<RemoteNavigation>(navigationclient);
        }
        
        IInputDevice& UIFactory::create_input_device(Options& options, JsonCpp& config)
        {
                std::string classname = get_input_device_classname(config);
                instantiate_input_device(classname, options, config);
                return *_input_device;
        }

        std::string UIFactory::get_input_device_classname(JsonCpp &config)
        {
                std::string classname;
                try {
                        classname = (const char *) config["user-interface"]["input-device-classname"];
                        
                } catch (JSONError &je) {
                        r_warn("Failed to get the value for "
                               "user-interface.input-device-classname: %s", je.what());
                        throw std::runtime_error("No input device classname");
                }
                return classname;
        }
        
        void UIFactory::instantiate_input_device(const std::string& classname,
                                                Options& options,
                                                JsonCpp& config)
        {
                r_info("Instantiating input device: '%s'", classname.c_str());
                if (classname == FakeInputDevice::ClassName) {
                        instantiate_fake_input_device();
                        
                } else if (classname == JoystickInputDevice::ClassName) {
                        instantiate_joystick(options, config);
                        
                } else {
                        r_err("Unknown input device class: '%s'", classname.c_str());
                        throw std::runtime_error("Failed to create the input device");
                }
        }
        
        void UIFactory::instantiate_fake_input_device()
        {
                _input_device = std::unique_ptr<IInputDevice>(new FakeInputDevice());
        }
        
        void UIFactory::instantiate_joystick(Options& options, JsonCpp& config)
        {
                std::string joystick_device = get_joystick_device(options, config);
                _joystick = std::make_unique<LinuxJoystick>(_linux, joystick_device);
                _joystick->set_debug(true);
                _input_device = std::make_unique<JoystickInputDevice>(*_joystick,
                                                                                      _joystick_event_mapper);
        }

        std::string UIFactory::get_joystick_device(Options& options, JsonCpp& config)
        {
                std::string joystick_device = options.get_value("joystick-device");
                if (joystick_device.empty()) {
                        joystick_device = get_joystick_device_in_config(config);
                }
                return joystick_device;
        }

        std::string UIFactory::get_joystick_device_in_config(JsonCpp& config)
        {
                std::string joystick_device;
                try {
                        joystick_device = (const char *) config["ports"]["joystick"]["port"];
                        
                } catch (JSONError &je) {
                        r_warn("Failed to get the value for "
                               "ports.joystick.port: %s", je.what());
                        throw std::runtime_error("No joystick device specified");
                }
                return joystick_device;
        }

        IWeeder& UIFactory::create_weeder(Options &options, JsonCpp &config)
        {
                if (_weeder == nullptr) {
                        instantiate_weeder(options, config);
                }
                return *_weeder;
        }
        
        void UIFactory::instantiate_weeder(Options &options, JsonCpp &config)
        {
                std::string classname = get_weeder_classname(config);
                instantiate_weeder(classname, options, config);
        }

        std::string UIFactory::get_weeder_classname(JsonCpp &config)
        {
                std::string classname;
                try {
                        classname = (const char *) config["user-interface"]["weeder-classname"];
                } catch (JSONError &je) {
                        r_warn("Failed to get the value for "
                               "user-interface.weeder-classname: %s", je.what());
                        throw std::runtime_error("No weeder classname defined");
                }
                return classname;
        }
        
        void UIFactory::instantiate_weeder(const std::string& classname,
                                              Options &options,
                                              JsonCpp &config)
        {
                r_info("Create an instance of weeder '%s'", classname.c_str());
                
                if (classname == FakeWeeder::ClassName) {
                        instantiate_fake_weeder();

                } else if (classname == RemoteWeeder::ClassName) {
                        instantiate_remote_weeder(options, config);
                        
                } else {
                        r_err("Unknown weeder class: '%s'", classname.c_str());
                        throw std::runtime_error("Failed to create the weeder module");
                }
        }
                
        void UIFactory::instantiate_fake_weeder()
        {
                _weeder = unique_ptr<IWeeder>(new FakeWeeder());
        }

        void UIFactory::instantiate_remote_weeder(__attribute__((unused))Options &options, __attribute__((unused))JsonCpp &config)
        {
                std::shared_ptr<rcom::IRPCHandler> weederclient = std::make_shared<RPCClient>("weeder",
                                                                     "weeder", 60.0);
                _weeder = std::make_unique<RemoteWeeder>(weederclient);
        }
        
}
