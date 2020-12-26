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

#include "Navigation.h"
#include "Display.h"
#include "InputDevice.h"

#include "UIOptions.h"
#include "JsonCpp.h"
#include "LinuxJoystick.h"
#include "UIEventMapper.h"
#include "RomiSerialClient.h"
#include "RSerial.h"
#include "RPCClient.h"
#include "Linux.h"

#ifndef __ROMI_UI_FACTORY_H
#define __ROMI_UI_FACTORY_H



namespace romi {

        class UIFactory
        {
        protected:

                rpp::Linux _linux;
                RSerial *_serial;
                RomiSerialClient *_romi_serial;
                rcom::RPCClient *_rpc_client; 
                LinuxJoystick *_joystick;
                UIEventMapper _joystick_event_mapper;

                Display *_display;
                Navigation *_navigation;
                InputDevice *_input_device;
                

                // Display
                void instantiate_display(UIOptions &options, JsonCpp &config);
                void instantiate_display(const char *display_classname,
                                        UIOptions &options,
                                        JsonCpp &config);
                const char *get_display_classname(UIOptions& options, JsonCpp& config);
                const char *get_display_classname_in_config(JsonCpp &config);
                void instantiate_fake_display();
                void instantiate_crystal_display(UIOptions &options, JsonCpp &config);
                const char *get_crystal_display_device(UIOptions &options,
                                                       JsonCpp &config);
                const char *get_crystal_display_device_in_config(JsonCpp &config);

                
                // Navigation
                void instantiate_navigation(UIOptions &options, JsonCpp &config);
                void instantiate_navigation(const char *classname, UIOptions &options,
                                           JsonCpp &config);
                const char *get_navigation_classname(UIOptions& options, JsonCpp& config);
                const char *get_navigation_classname_in_config(JsonCpp &config);
                void instantiate_fake_navigation();
                void instantiate_remote_navigation(UIOptions &options, JsonCpp &config);
                const char *get_remote_navigation_server(UIOptions &options,
                                                         JsonCpp &config);
                const char *get_remote_navigation_server_in_config(JsonCpp &config);
                        

                // Input device / joystick
                const char *get_input_device_classname(UIOptions &options,
                                                       JsonCpp &config);
                const char *get_input_device_classname_in_config(JsonCpp &config);
                void instantiate_input_device(const char *classname, UIOptions& options,
                                             JsonCpp& config);
                void instantiate_fake_input_device();
                void instantiate_joystick(UIOptions& options, JsonCpp& config);
                const char *get_joystick_device(UIOptions& options, JsonCpp& config);
                const char *get_joystick_device_in_config(JsonCpp& config);
                
                
        public:

                UIFactory();
                virtual ~UIFactory();
                
                Display& create_display(UIOptions& options, JsonCpp& config);
                Navigation& create_navigation(UIOptions& options, JsonCpp& config);
                InputDevice& create_input_device(UIOptions& options, JsonCpp& config);
        };
}

#endif // __ROMI_UI_FACTORY_H
