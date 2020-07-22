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
#include "StateMachineTimer.h"

void StateMachineTimer::setTimeout(int milliseconds, int event)
{
        _enabled = 1;
        _timeout = _arduino->millis() + milliseconds;
        _event = event;
}

void StateMachineTimer::update(unsigned long t)
{
        if (_enabled && t > _timeout) {
                _stateMachine->handleEvent(_event);
                // int r = stateMachine.handleEvent(_event);
                // if (r == StateMachine::OK)
                //         Serial.println("OK");
                // else if (r == StateMachine::Ignored)
                //         Serial.println("bad state");
                // else 
                //         Serial.println("failed");
                _enabled = 0;
        }
}
