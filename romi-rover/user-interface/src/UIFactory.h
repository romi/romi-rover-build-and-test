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

#include <JsonCpp.h>
#include <RomiSerialClient.h>
#include <RSerial.h>
#include <RPCClient.h>

#include <api/INavigation.h>
#include <api/IDisplay.h>
#include <api/IInputDevice.h>
#include <api/INotifications.h>
#include <api/IWeeder.h>
#include <Options.h>
#include <LinuxJoystick.h>
#include <UIEventMapper.h>
#include <Linux.h>

#ifndef __ROMI_UI_FACTORY_H
#define __ROMI_UI_FACTORY_H

namespace romi {

        class UIFactory
        {
        protected:

                rpp::Linux _linux;
                UIEventMapper _joystick_event_mapper;

                std::shared_ptr<RSerial> _serial;
                std::unique_ptr<RomiSerialClient> _romi_serial;
                std::unique_ptr<LinuxJoystick> _joystick;

                std::unique_ptr<IDisplay> _display;
                std::unique_ptr<INavigation> _navigation;
                std::unique_ptr<IInputDevice> _input_device;
                std::unique_ptr<IWeeder> _weeder;
                

                // Display
                void instantiate_display(Options &options, JsonCpp &config);
                void instantiate_display(const std::string& display_classname,
                                        Options &options,
                                        JsonCpp &config);
                std::string get_display_classname(JsonCpp &config);
                void instantiate_fake_display();
                void instantiate_crystal_display(Options &options, JsonCpp &config);
                std::string get_crystal_display_device(Options &options,
                                                       JsonCpp &config);
                std::string get_crystal_display_device_in_config(JsonCpp &config);

                
                // Navigation
                void instantiate_navigation(Options &options, JsonCpp &config);
                void instantiate_navigation(const std::string& classname, Options &options,
                                           JsonCpp &config);
                std::string get_navigation_classname(JsonCpp &config);
                void instantiate_fake_navigation();
                void instantiate_remote_navigation(Options &options, JsonCpp &config);

                // Input device / joystick
                static std::string get_input_device_classname(JsonCpp &config);
                void instantiate_input_device(const std::string& classname, Options& options,
                                             JsonCpp& config);
                void instantiate_fake_input_device();
                void instantiate_joystick(Options& options, JsonCpp& config);
                std::string get_joystick_device(Options& options, JsonCpp& config);
                std::string get_joystick_device_in_config(JsonCpp& config);

                // Weeder
                void instantiate_weeder(Options &options, JsonCpp &config);
                void instantiate_weeder(const std::string& classname, Options &options,
                                           JsonCpp &config);
                static std::string get_weeder_classname(JsonCpp &config);
                void instantiate_fake_weeder();
                void instantiate_remote_weeder(Options &options, JsonCpp &config);
                
        public:

                UIFactory();
                virtual ~UIFactory();
                
                IDisplay& create_display(Options& options, JsonCpp& config);
                INavigation& create_navigation(Options& options, JsonCpp& config);
                IInputDevice& create_input_device(Options& options, JsonCpp& config);
                IWeeder& create_weeder(Options& options, JsonCpp& config);
                // INotifications& create_notifications(Options& options, JsonCpp& config);
             //   std::string get_script_file(Options &options, JsonCpp &config);
        };
}

#endif // __ROMI_UI_FACTORY_H
