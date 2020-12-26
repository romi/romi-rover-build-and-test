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

#ifndef __ROMI_USER_INTERFACE_H
#define __ROMI_USER_INTERFACE_H

#include "InputDevice.h"
#include "Display.h"
#include "SpeedController.h"
#include "StateMachine.h"
#include "EventTimer.h"
#include "Menu.h"
#include "ScriptEngine.h"
#include "Notifications.h"

namespace romi {
        
        class UserInterface
        {
        protected:

                void init_state_machine();
                void init_start_transition();
                void init_stop_transitions();
                
                void init_navigation_transitions();
                void init_navigation_mode_transition();
                void init_forward_driving_transitions();
                void init_accurate_forward_driving_transitions();
                void init_backward_driving_transitions();
                void init_accurate_backward_driving_transitions();
                void init_spinning_transitions();

                void init_menu_transitions();
                void init_menu_mode_transition();
                void init_menu_selection_transitions();

                void handle_input_events();
                void handle_timer_events();
                void handle_script_events();

        public:
                InputDevice& input_device;
                Display& display;
                SpeedController& speed_controller;
                EventTimer& event_timer;
                Menu& menu;
                ScriptEngine& script_engine;
                Notifications& notifications;
                
                StateMachine<UserInterface> state_machine;

                int saved_state;

                UserInterface(InputDevice& input_device_,
                              Display& display_,
                              SpeedController& speed_controller_,
                              EventTimer& event_timer,
                              Menu& menu,
                              ScriptEngine& script_engine,
                              Notifications& notifications);
                
                virtual ~UserInterface() = default;

                void handle_events();

                int get_state() {
                        return state_machine.get_state();
                }

        };
}
#endif // __ROMI_USER_INTERFACE_H
