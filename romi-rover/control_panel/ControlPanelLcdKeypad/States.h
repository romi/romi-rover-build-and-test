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

#ifndef __STATES_H
#define __STATES_H

enum {
        // STATE_ERROR(0) and STATE_START(1) are defined by default
        // in IStateMachine.h

        // Both relays off.
        STATE_OFF = 2,
        // Power relay off and controller relay on. Waiting for
        // controller circuit to send powerup event.
        STATE_STARTING_UP,
        // Power relay off and controller relay remains on for
        // 5s. This gives the controller the time to shut down
        // cleanly.
        STATE_SHUTTING_DOWN,
        // Both relays on
        STATE_ON,
        // Showing menu
        STATE_MENU,
        // Confirm menu selection
        STATE_CONFIRM,
        // Sending selection to rover
        STATE_SENDING,
        // Insert new state before this one :)
        STATE_LAST
};

#define STATE_ERROR_STR         "Error"
#define STATE_START_STR         "Start"
#define STATE_OFF_STR           "Off"
#define STATE_STARTING_UP_STR   "Starting up"
#define STATE_SHUTTING_DOWN_STR "Shutting down"
#define STATE_ON_STR            "On"
#define STATE_MENU_STR          "Menu"
#define STATE_CONFIRM_STR       "Confirm"
#define STATE_SENDING_STR       "Action"

const char* getStateString(int state);

#endif // __STATES_H
