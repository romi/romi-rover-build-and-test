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

#ifndef __CONTROL_PANEL_H
#define __CONTROL_PANEL_H

#include "IArduino.h"
#include "ISerial.h"
#include "IDisplay.h"
#include "IParser.h"
#include "IMenu.h"
#include "IStateMachine.h"
#include "IStateMachineTimer.h"
#include "IButtonPanel.h"

class ControlPanel
{
protected:
        IArduino *_arduino;
        ISerial *_serial;
        IDisplay *_display;
        IButtonPanel *_buttonPanel;
        IParser *_parser;
        IMenu *_menu;
        IStateMachine *_stateMachine;
        IStateMachineTimer *_timer;

        void handleSerialInput();
        void sendState();
        
public:
        ControlPanel(IArduino *arduino,
                     ISerial *serial,
                     IDisplay *display,
                     IButtonPanel *buttonPanel,
                     IParser *parser,
                     IMenu *menu,
                     IStateMachine *stateMachine,
                     IStateMachineTimer *timer)
                : _arduino(arduino),
                _serial(serial),
                _display(display),
                _buttonPanel(buttonPanel),
                _parser(parser),
                _menu(menu),
                _stateMachine(stateMachine),
                _timer(timer) {} 
        virtual ~ControlPanel() {}

        void init();
        void loop();
};

#endif // __ICONTROL_PANEL_H
