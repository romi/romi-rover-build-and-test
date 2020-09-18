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

#include "IStateMachine.h"
#include "States.h"

const char* getStateString(int state)
{
        const char* r = "?";
        switch (state) {
        case STATE_ERROR:
                r = "Error";
                break;
        case STATE_START:
                r = "Start";
                break;
        case STATE_OFF:
                r = "Off";
                break;
        case STATE_STARTING_UP:
                r = "Starting up";
                break;
        case STATE_SHUTTING_DOWN:
                r = "Shutting down";
                break;
        case STATE_ON:
                r = "On";
                break;
        case STATE_MENU:
                r = "Menu";
                break;
        case STATE_CONFIRM:
                r = "Confirm";
                break;
        case STATE_SENDING:
                r = "Action";
                break;
        }
        return r;
}
