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
        // STATE_ERROR(-1) and STATE_START(0) are defined by default
        // in IStateMachine.h

        // Both relays off.
        STATE_OFF = 1,
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
        STATE_SENDING 
};

const char* getStateString(int state);

#endif // __STATES_H
