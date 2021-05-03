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

#ifndef __ROMI_PLANT_CARRIER_H
#define __ROMI_PLANT_CARRIER_H

#include "InputDevice.h"
#include "Display.h"
#include "rover/SpeedController.h"
#include "rover/Navigation.h"
#include "StateMachine.h"
#include "api/EventTimer.h"
#include "ui/Menu.h"
#include "Notifications.h"
#include "Weeder.h"
#include "ScriptEngine.h"

namespace romi {
        
        class PlantCarrier
        {
        public:
                InputDevice& input_device;
                Display& display;
                SpeedController& speed_controller;
                Navigation& navigation;
                EventTimer& event_timer;
                Menu& menu;
                ScriptEngine<PlantCarrier>& script_engine;
                Notifications& notifications;
                CNC& cnc;
                
                PlantCarrier(InputDevice& input_device_,
                      Display& display_,
                      SpeedController& speed_controller_,
                      Navigation& navigation_,
                      EventTimer& event_timer,
                      Menu& menu,
                      ScriptEngine<PlantCarrier>& script_engine,
                      Notifications& notifications,
                      CNC& cnc);
                
                virtual ~PlantCarrier() = default;

                bool reset();
        };
}

#endif // __ROMI_PLANT_CARRIER_H
