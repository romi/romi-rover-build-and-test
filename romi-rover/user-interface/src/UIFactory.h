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

#include <api/Navigation.h>
#include <api/Display.h>
#include <api/InputDevice.h>
#include <api/Notifications.h>
#include <api/Weeder.h>
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
                std::unique_ptr<rcom::RPCClient> _navigation_client; 
                std::unique_ptr<rcom::RPCClient> _weeder_client; 
                std::unique_ptr<LinuxJoystick> _joystick;

                std::unique_ptr<Display> _display;
                std::unique_ptr<Navigation> _navigation;
                std::unique_ptr<InputDevice> _input_device;
            //    std::unique_ptr<Notifications> _notifications;
                std::unique_ptr<Weeder> _weeder;
                

                // Display
                void instantiate_display(Options &options, JsonCpp &config);
                void instantiate_display(const char *display_classname,
                                        Options &options,
                                        JsonCpp &config);
                const char *get_display_classname(JsonCpp &config);
                void instantiate_fake_display();
                void instantiate_crystal_display(Options &options, JsonCpp &config);
                const char *get_crystal_display_device(Options &options,
                                                       JsonCpp &config);
                const char *get_crystal_display_device_in_config(JsonCpp &config);

                
                // Navigation
                void instantiate_navigation(Options &options, JsonCpp &config);
                void instantiate_navigation(const char *classname, Options &options,
                                           JsonCpp &config);
                const char *get_navigation_classname(JsonCpp &config);
                void instantiate_fake_navigation();
                void instantiate_remote_navigation(Options &options, JsonCpp &config);

                // Input device / joystick
                const char *get_input_device_classname(JsonCpp &config);
                void instantiate_input_device(const char *classname, Options& options,
                                             JsonCpp& config);
                void instantiate_fake_input_device();
                void instantiate_joystick(Options& options, JsonCpp& config);
                const char *get_joystick_device(Options& options, JsonCpp& config);
                const char *get_joystick_device_in_config(JsonCpp& config);

                // Weeder
                void instantiate_weeder(Options &options, JsonCpp &config);
                void instantiate_weeder(const char *classname, Options &options,
                                           JsonCpp &config);
                const char *get_weeder_classname(JsonCpp &config);
                void instantiate_fake_weeder();
                void instantiate_remote_weeder(Options &options, JsonCpp &config);


                // Script
                const char *get_script_file_in_config(JsonCpp &config);
                
        public:

                UIFactory();
                virtual ~UIFactory();
                
                Display& create_display(Options& options, JsonCpp& config);
                Navigation& create_navigation(Options& options, JsonCpp& config);
                InputDevice& create_input_device(Options& options, JsonCpp& config);
                Weeder& create_weeder(Options& options, JsonCpp& config);
                // Notifications& create_notifications(Options& options, JsonCpp& config);
                const char *get_script_file(Options &options, JsonCpp &config);
        };
}

#endif // __ROMI_UI_FACTORY_H
