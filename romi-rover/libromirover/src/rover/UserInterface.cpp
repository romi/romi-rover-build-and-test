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
#include "EventsAndStates.h"
#include "UserInterface.h"

namespace romi {
        
        UserInterface::UserInterface(Rover& rover,
                                     StateMachine<Rover>& state_machine)
                : _rover(rover),
                  _state_machine(state_machine)
        {
        }

        void UserInterface::handle_events()
        {
                handle_input_events();
                handle_timer_events();
                handle_script_events();
        }
        
        void UserInterface::handle_input_events()
        {
                while (true) {
                        int event = _rover.input_device.get_next_event();
                        if (event == 0)
                                break;
                        _state_machine.handle_event(event);
                }
        }
        
        void UserInterface::handle_timer_events()
        {
                int event = _rover.event_timer.get_next_event();
                if (event != 0)
                        _state_machine.handle_event(event);
        }

        void UserInterface::handle_script_events()
        {
                int event = _rover.script_engine.get_next_event();
                if (event != 0)
                        _state_machine.handle_event(event);
        }

        void UserInterface::handle_event(int event)
        {
                bool success = _state_machine.handle_event(event);
                if (!success)
                        _state_machine.handle_event(event_system_failure);
        }
}
