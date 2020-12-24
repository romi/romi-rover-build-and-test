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
#include "UIFactory.h"
#include "FakeNavigation.h"
#include "RemoteNavigation.h"
#include "FakeDisplay.h"
#include "CrystalDisplay.h"
#include "JoystickInputDevice.h"
#include "FakeInputDevice.h"

namespace romi {
        
        UIFactory::UIFactory()
                : _serial(0),
                  _romi_serial(0),
                  _rpc_client(0),
                  _joystick(0),
                  _display(0),
                  _navigation(0),
                  _input_device(0)
        {}
        
        UIFactory::~UIFactory()
        {
                if (_navigation)
                        delete _navigation;
                if (_rpc_client)
                        delete _rpc_client;
                if (_display)
                        delete _display;
                if (_romi_serial)
                        delete _romi_serial;
                if (_serial)
                        delete _serial;
                if (_joystick)
                        delete _joystick;
                if (_input_device)
                        delete _input_device;
        }
        
        Display& UIFactory::create_display(UIOptions &options, JsonCpp &config)
        {
                if (_display == 0)
                        instantiate_display(options, config);
                return *_display;
        }

        void UIFactory::instantiate_display(UIOptions &options, JsonCpp &config)
        {
                const char *classname = get_display_classname(options, config);
                instantiate_display(classname, options, config);
        }
        
        const char *UIFactory::get_display_classname(UIOptions &options, JsonCpp &config)
        {
                const char *display_classname = options.display_classname;
                if (display_classname == 0) {
                        display_classname = get_display_classname_in_config(config);
                } 
                return display_classname;
        }
        
        const char *UIFactory::get_display_classname_in_config(JsonCpp &config)
        {
                const char *display_classname = 0;
                try {
                        display_classname = config["user-interface"]["display-classname"];
                        
                } catch (JSONError &je) {
                        r_warn("Failed to get the value for "
                               "user-interface.display: %s", je.what());
                        throw std::runtime_error("No display classname defined");
                }
                return display_classname;
        }
        
        void UIFactory::instantiate_display(const char *classname,
                                           UIOptions &options,
                                           JsonCpp &config)
        {
                r_info("create_display: Creating an instance of '%s'", classname);
                
                if (rstreq(classname, FakeDisplay::ClassName)) {
                        instantiate_fake_display();

                } else if (rstreq(classname, CrystalDisplay::ClassName)) {
                        instantiate_crystal_display(options, config);
                        
                } else {
                        r_err("Unknown display class: '%s'", classname);
                        throw std::runtime_error("Failed to create the display");
                }
        }
        
        void UIFactory::instantiate_fake_display()
        {
                _display = new FakeDisplay();
        }
        
        void UIFactory::instantiate_crystal_display(UIOptions &options, JsonCpp &config)
        {
                const char *device = get_crystal_display_device(options, config);
                _serial = new RSerial(device, 115200, 1);
                _romi_serial = new RomiSerialClient(_serial, _serial);
                _romi_serial->set_debug(true);
                _display = new CrystalDisplay(*_romi_serial);
        }

        const char *UIFactory::get_crystal_display_device(UIOptions &options,
                                                          JsonCpp &config)
        {
                const char *device_name = options.display_device;
                if (device_name == 0) {
                        device_name = get_crystal_display_device_in_config(config);
                }
                return device_name;
        }
        
        const char *UIFactory::get_crystal_display_device_in_config(JsonCpp &config)
        {
                const char *device_name = 0;
                try {
                        device_name = config["ports"]["crystal-display"]["port"];
                        
                } catch (JSONError &je) {
                        r_warn("Failed to get the value for "
                               "crystal-display.port: %s", je.what());
                        throw std::runtime_error("No crystal display device defined");
                }
                return device_name;
        }
        
        

        
        Navigation& UIFactory::create_navigation(UIOptions &options, JsonCpp &config)
        {
                if (_navigation == 0) {
                        instantiate_navigation(options, config);
                }
                return *_navigation;
        }
        
        void UIFactory::instantiate_navigation(UIOptions &options, JsonCpp &config)
        {
                const char *classname = get_navigation_classname(options, config);
                instantiate_navigation(classname, options, config);
        }
        
        const char *UIFactory::get_navigation_classname(UIOptions &options,
                                                        JsonCpp &config)
        {
                const char *classname = options.navigation_classname;
                if (classname == 0) {
                        classname = get_navigation_classname_in_config(config);
                }
                return classname;
        }
        
        const char *UIFactory::get_navigation_classname_in_config(JsonCpp &config)
        {
                const char *classname = 0;
                try {
                        classname = config["user-interface"]["navigation-classname"];
                } catch (JSONError &je) {
                        r_warn("Failed to get the value for "
                               "user-interface.navigation-classname: %s", je.what());
                        throw std::runtime_error("No navigation classname defined");
                }
                return classname;
        }
        
        void UIFactory::instantiate_navigation(const char *classname,
                                              UIOptions &options,
                                              JsonCpp &config)
        {
                r_info("Create an instance of navigation '%s'", classname);
                
                if (rstreq(classname, FakeNavigation::ClassName)) {
                        instantiate_fake_navigation();

                } else if (rstreq(classname, RemoteNavigation::ClassName)) {
                        instantiate_remote_navigation(options, config);
                        
                } else {
                        r_err("Unknown navigation class: '%s'", classname);
                        throw std::runtime_error("Failed to create the navigation module");
                }
        }
                
        void UIFactory::instantiate_fake_navigation()
        {
                _navigation = new FakeNavigation();
        }

        void UIFactory::instantiate_remote_navigation(UIOptions &options, JsonCpp &config)
        {
                const char *server_name = get_remote_navigation_server(options, config);
                _rpc_client = new rcom::RPCClient(server_name, "navigation");
                _navigation = new RemoteNavigation(*_rpc_client);
        }
        
        const char *UIFactory::get_remote_navigation_server(UIOptions &options,
                                                            JsonCpp &config)
        {
                const char *name = options.navigation_server_name;
                if (name == 0) {
                        name = get_remote_navigation_server_in_config(config);
                }
                return name;
        }
        
        const char *UIFactory::get_remote_navigation_server_in_config(JsonCpp &config)
        {
                const char *name = 0;
                try {
                        name = config["user-interface"][RemoteNavigation::ClassName]["server-name"];
                        
                } catch (JSONError &je) {
                        r_warn("Failed to get the value for "
                               "user-interface.remote-navigation.server-name: %s",
                               je.what());
                        throw std::runtime_error("Remote navigation: No server name");
                }
                return name;
        }
        

        
        InputDevice& UIFactory::create_input_device(UIOptions& options, JsonCpp& config)
        {
                const char *classname = get_input_device_classname(options, config);
                instantiate_input_device(classname, options, config);
                return *_input_device;
        }

        const char *UIFactory::get_input_device_classname(UIOptions &options,
                                                        JsonCpp &config)
        {
                const char *classname = options.input_device_classname;
                if (classname == 0) {
                        classname = get_input_device_classname_in_config(config);
                }
                return classname;
        }
        
        const char *UIFactory::get_input_device_classname_in_config(JsonCpp &config)
        {
                const char *classname = 0;
                try {
                        classname = config["user-interface"]["input-device-classname"];
                        
                } catch (JSONError &je) {
                        r_warn("Failed to get the value for "
                               "user-interface.input-device-classname: %s", je.what());
                        throw std::runtime_error("No input device classname");
                }
                return classname;
        }
        
        void UIFactory::instantiate_input_device(const char *classname,
                                                UIOptions& options,
                                                JsonCpp& config)
        {
                r_info("Instantiating input device: '%s'", classname);
                if (rstreq(classname, FakeInputDevice::ClassName)) {
                        instantiate_fake_input_device();
                        
                } else if (rstreq(classname, JoystickInputDevice::ClassName)) {
                        instantiate_joystick(options, config);
                        
                } else {
                        r_err("Unknown input device class: '%s'", classname);
                        throw std::runtime_error("Failed to create the input device");
                }
        }
        
        void UIFactory::instantiate_fake_input_device()
        {
                _input_device = new FakeInputDevice();
        }
        
        void UIFactory::instantiate_joystick(UIOptions& options, JsonCpp& config)
        {
                const char *joystick_device = get_joystick_device(options, config);
                _joystick = new LinuxJoystick(_linux, joystick_device);
                _joystick->set_debug(true);
                _input_device = new JoystickInputDevice(*_joystick, _joystick_event_mapper);
        }

        const char *UIFactory::get_joystick_device(UIOptions& options, JsonCpp& config)
        {
                const char *joystick_device = options.joystick_device;
                if (joystick_device == 0) {
                        joystick_device = get_joystick_device_in_config(config);
                }
                return joystick_device;
        }

        const char *UIFactory::get_joystick_device_in_config(JsonCpp& config)
        {
                const char *joystick_device = 0;
                try {
                        joystick_device = config["ports"]["joystick"]["port"];
                        
                } catch (JSONError &je) {
                        r_warn("Failed to get the value for "
                               "ports.joystick.port: %s", je.what());
                        throw std::runtime_error("No joystick device specified");
                }
                return joystick_device;
        }
}
