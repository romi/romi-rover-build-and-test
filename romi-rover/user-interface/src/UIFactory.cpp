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
#include "FakeWeeder.h"
#include "RemoteWeeder.h"

using namespace std;
using namespace rcom;

namespace romi {
        
        UIFactory::UIFactory()
                : _serial(0)
        {}
        
        UIFactory::~UIFactory()
        {
                if (_serial)
                        delete _serial;
        }
        
        Display& UIFactory::create_display(Options &options, JsonCpp &config)
        {
                if (_display == 0)
                        instantiate_display(options, config);
                return *_display;
        }

        void UIFactory::instantiate_display(Options &options, JsonCpp &config)
        {
                const char *classname = get_display_classname(options, config);
                instantiate_display(classname, options, config);
        }
        
        const char *UIFactory::get_display_classname(Options &options, JsonCpp &config)
        {
                const char *display_classname = options.get_value("display-classname");
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
                               "user-interface.display-classname: %s", je.what());
                        throw std::runtime_error("No display classname defined");
                }
                return display_classname;
        }
        
        void UIFactory::instantiate_display(const char *classname,
                                            Options &options,
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
                _display = std::unique_ptr<Display>(new FakeDisplay());
        }
        
        void UIFactory::instantiate_crystal_display(Options &options, JsonCpp &config)
        {
                const char *device = get_crystal_display_device(options, config);
                _serial = new RSerial(device, 115200, 1);
                _romi_serial = unique_ptr<RomiSerialClient>(new RomiSerialClient(_serial,
                                                                                 _serial));
                _romi_serial->set_debug(true);
                _display = std::unique_ptr<Display>(new CrystalDisplay(*_romi_serial));
        }

        const char *UIFactory::get_crystal_display_device(Options &options,
                                                          JsonCpp &config)
        {
                const char *device_name = options.get_value("display-device");
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
                               "ports.crystal-display.port: %s", je.what());
                        throw std::runtime_error("No crystal display device defined");
                }
                return device_name;
        }
        
        Navigation& UIFactory::create_navigation(Options &options, JsonCpp &config)
        {
                if (_navigation == 0) {
                        instantiate_navigation(options, config);
                }
                return *_navigation;
        }
        
        void UIFactory::instantiate_navigation(Options &options, JsonCpp &config)
        {
                const char *classname = get_navigation_classname(options, config);
                instantiate_navigation(classname, options, config);
        }
        
        const char *UIFactory::get_navigation_classname(Options &options,
                                                        JsonCpp &config)
        {
                const char *classname = options.get_value("navigation-classname");
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
                                              Options &options,
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
                _navigation = unique_ptr<Navigation>(new FakeNavigation());
        }

        void UIFactory::instantiate_remote_navigation(Options &options, JsonCpp &config)
        {
                _navigation_client = unique_ptr<RPCClient>(new RPCClient("navigation",
                                                                         "navigation",
                                                                         10.0));
                _navigation = unique_ptr<Navigation>(new RemoteNavigation(*_navigation_client));
        }
        
        InputDevice& UIFactory::create_input_device(Options& options, JsonCpp& config)
        {
                const char *classname = get_input_device_classname(options, config);
                instantiate_input_device(classname, options, config);
                return *_input_device;
        }

        const char *UIFactory::get_input_device_classname(Options &options,
                                                        JsonCpp &config)
        {
                const char *classname = options.get_value("input-device-classname");
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
                                                Options& options,
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
                _input_device = std::unique_ptr<InputDevice>(new FakeInputDevice());
        }
        
        void UIFactory::instantiate_joystick(Options& options, JsonCpp& config)
        {
                const char *joystick_device = get_joystick_device(options, config);
                _joystick = std::unique_ptr<LinuxJoystick>(new LinuxJoystick(_linux, joystick_device));
                _joystick->set_debug(true);
                _input_device = std::unique_ptr<InputDevice>(new JoystickInputDevice(*_joystick,
                                                                                     _joystick_event_mapper));
        }

        const char *UIFactory::get_joystick_device(Options& options, JsonCpp& config)
        {
                const char *joystick_device = options.get_value("joystick-device");
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

        const char *UIFactory::get_script_file(Options &options, JsonCpp &config)
        {
                const char *path = options.get_value("script-file");
                if (path == 0) {
                        path = get_script_file_in_config(config);
                }
                return path;
        }

        const char *UIFactory::get_script_file_in_config(JsonCpp &config)
        {
                const char *path = 0;
                try {
                        path = config["user-interface"]["rover-script-engine"]["script-file"];
                        
                } catch (JSONError &je) {
                        r_warn("Failed to get the value for "
                               "user-interface.rover-script-engine.script-file: %s",
                               je.what());
                        throw std::runtime_error("No script file");
                }
                return path;
        }

        // Notifications& create_notifications(Options& options, JsonCpp& config)
        // {
        // }


        Weeder& UIFactory::create_weeder(Options &options, JsonCpp &config)
        {
                if (_weeder == 0) {
                        instantiate_weeder(options, config);
                }
                return *_weeder;
        }
        
        void UIFactory::instantiate_weeder(Options &options, JsonCpp &config)
        {
                const char *classname = get_weeder_classname(options, config);
                instantiate_weeder(classname, options, config);
        }
        
        const char *UIFactory::get_weeder_classname(Options &options,
                                                        JsonCpp &config)
        {
                const char *classname = options.get_value("weeder-classname");
                if (classname == 0) {
                        classname = get_weeder_classname_in_config(config);
                }
                return classname;
        }
        
        const char *UIFactory::get_weeder_classname_in_config(JsonCpp &config)
        {
                const char *classname = 0;
                try {
                        classname = config["user-interface"]["weeder-classname"];
                } catch (JSONError &je) {
                        r_warn("Failed to get the value for "
                               "user-interface.weeder-classname: %s", je.what());
                        throw std::runtime_error("No weeder classname defined");
                }
                return classname;
        }
        
        void UIFactory::instantiate_weeder(const char *classname,
                                              Options &options,
                                              JsonCpp &config)
        {
                r_info("Create an instance of weeder '%s'", classname);
                
                if (rstreq(classname, FakeWeeder::ClassName)) {
                        instantiate_fake_weeder();

                } else if (rstreq(classname, RemoteWeeder::ClassName)) {
                        instantiate_remote_weeder(options, config);
                        
                } else {
                        r_err("Unknown weeder class: '%s'", classname);
                        throw std::runtime_error("Failed to create the weeder module");
                }
        }
                
        void UIFactory::instantiate_fake_weeder()
        {
                _weeder = unique_ptr<Weeder>(new FakeWeeder());
        }

        void UIFactory::instantiate_remote_weeder(Options &options, JsonCpp &config)
        {
                _weeder_client = unique_ptr<RPCClient>(new RPCClient("weeder",
                                                                     "weeder", 60.0));
                _weeder = unique_ptr<Weeder>(new RemoteWeeder(*_weeder_client));
        }
        
}
