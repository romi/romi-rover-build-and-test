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
#include "UIOptions.h"
#include "JsonCpp.h"

#include "RomiSerialClient.h"
#include "RSerial.h"
#include "RPCClient.h"

#ifndef __ROMI_UI_FACTORY_H
#define __ROMI_UI_FACTORY_H



namespace romi {

        class UIFactory
        {
        protected:

                RSerial *_serial = 0;
                RomiSerialClient *_romi_serial = 0;
                rcom::RPCClient *_rpc_client = 0; 

                Display *_display = 0;
                Navigation *_navigation = 0;

                void do_create_display(UIOptions& options, JsonCpp& config);
                void do_create_navigation(UIOptions& options, JsonCpp& config);
                const char *get_display_classname(UIOptions& options, JsonCpp& config);
                const char *get_navigation_classname(UIOptions& options, JsonCpp& config);
                
        public:
                UIFactory()
                        : _serial(0),
                          _romi_serial(0),
                          _rpc_client(0),
                          _display(0),
                          _navigation(0) {}
                
                virtual ~UIFactory() {
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
                }
                
                Display& create_display(UIOptions& options, JsonCpp& config);
                Navigation& create_navigation(UIOptions& options, JsonCpp& config);
        };
}

#endif // __ROMI_UI_FACTORY_H
