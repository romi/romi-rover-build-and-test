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

#ifndef __ROMI_ROVER_H
#define __ROMI_ROVER_H

#include "InputDevice.h"
#include "Display.h"
#include "SpeedController.h"
#include "Navigation.h"
#include "StateMachine.h"
#include "EventTimer.h"
#include "Menu.h"
#include "Notifications.h"
#include "Weeder.h"
#include "ScriptEngine.h"

namespace romi {
        
        class Rover
        {
        public:
                InputDevice& input_device;
                Display& display;
                SpeedController& speed_controller;
                Navigation& navigation;
                EventTimer& event_timer;
                Menu& menu;
                ScriptEngine<Rover>& script_engine;
                Notifications& notifications;
                Weeder& weeder;
                
                Rover(InputDevice& input_device_,
                      Display& display_,
                      SpeedController& speed_controller_,
                      Navigation& navigation_,
                      EventTimer& event_timer,
                      Menu& menu,
                      ScriptEngine<Rover>& script_engine,
                      Notifications& notifications,
                      Weeder& weeder);
                
                virtual ~Rover() = default;
        };
}

#endif // __ROMI_ROVER_H
