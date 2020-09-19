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

#ifndef __EVENTS_H
#define __EVENTS_H

#include "Buttons.h"
#include "IButtonPanel.h"

enum {
        EVENT_READY = 0,
        EVENT_POWERUP,
        EVENT_SHUTDOWN,
        EVENT_POWERDOWN,
        EVENT_ACTION_SENT,
        EVENT_ACTION_TIMEOUT
};

#define EVENT_BUTTON 0x1000

#define ButtonEvent(_button, _state) ((int16_t) (EVENT_BUTTON | (_button << 4) | _state))

#define EVENT_ONOFF_HELD ButtonEvent(BUTTON_ONOFF, BUTTON_HELD)
#define EVENT_MENU_PRESSED ButtonEvent(BUTTON_MENU, BUTTON_PRESSED)
#define EVENT_MENU_HELD ButtonEvent(BUTTON_MENU, BUTTON_HELD)
#define EVENT_UP_PRESSED ButtonEvent(BUTTON_UP, BUTTON_PRESSED)
#define EVENT_DOWN_PRESSED ButtonEvent(BUTTON_DOWN, BUTTON_PRESSED)
#define EVENT_SELECT_PRESSED ButtonEvent(BUTTON_SELECT, BUTTON_PRESSED)

#endif // __EVENTS_H
