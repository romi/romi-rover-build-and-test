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
#ifndef __IBUTTON_PANEL_H
#define __IBUTTON_PANEL_H

#include <stdint.h>
#include "IStateMachine.h"

#define BUTTON_PRESSED 0x01
#define BUTTON_HELD    0x02

class IButtonPanel
{
public:
        virtual ~IButtonPanel() {}

        // Update sends button events to the state machine.
        virtual void update(IStateMachine *stateMachine, unsigned long t) = 0;
};

#endif // __IBUTTON_PANEL_H
