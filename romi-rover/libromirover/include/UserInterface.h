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

#include "Rover.h"
#include "StateMachine.h"

namespace romi {
        
        class UserInterface
        {
        protected:

                void handle_input_events();
                void handle_timer_events();
                void handle_script_events();
                void handle_event(int event);
                
        public:
                Rover& _rover;
                StateMachine<Rover>& _state_machine;

                UserInterface(Rover& rover,
                              StateMachine<Rover>& state_machine);
                
                virtual ~UserInterface() = default;

                void handle_events();

                int get_state() {
                        return _state_machine.get_state();
                }

        };
}
#endif // __ROMI_USER_INTERFACE_H
