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

#include <stdio.h>
#include "pins.h"
#include "ControlPanel.h"
#include "States.h"
#include "Events.h"

static const char *ok = "OK"; 
static const char *errorBadArgs = "ERR args"; 
static const char *errorBadState = "ERR state"; 
static const char *errorFail = "ERR fail"; 

void ControlPanel::init()
{
        initStateMachine();
        _stateMachine.handleEvent(EVENT_READY, 0);        
}

void ControlPanel::update(unsigned long t)
{
        _buttonPanel->update(&_stateMachine, t);
        handleSerialInput(t);
        int16_t event =_eventTimer->update(t);
        if (event >= 0)
                _stateMachine.handleEvent(event, t);
}

void ControlPanel::sendState()
{
        static char buffer[64];
        snprintf(buffer, sizeof(buffer),
                 "S[\"%s\",%d]",
                 getStateString(_stateMachine.getState()),
                 _menu.currentMenuItemID());
        _out->println(buffer);
}

void ControlPanel::handleSerialInput(unsigned long t)
{
        while (_in->available()) {
                int c = _in->read();
                if (_parser.process(c)) {                        
                        switch (_parser.opcode()) {
                        default: break;
                        case '0':
                                if (_parser.length() == 0) {
                                        int r = _stateMachine.handleEvent(EVENT_SHUTDOWN, t);
                                        if (r == IStateMachine::OK)
                                                _out->println(ok);
                                        else if (r == IStateMachine::Ignored)
                                                _out->println(errorBadState);
                                        else 
                                                _out->println(errorFail);
                                } else {
                                        _out->println(errorBadArgs);
                                }
                                break;
                        case '1':
                                if (_parser.length() == 0) {
                                        int r = _stateMachine.handleEvent(EVENT_POWERUP, t);
                                        if (r == IStateMachine::OK)
                                                _out->println(ok);
                                        else if (r == IStateMachine::Ignored)
                                                _out->println(errorBadState);
                                        else 
                                                _out->println(errorFail);
                                } else {
                                        _out->println(errorBadArgs);
                                }
                                break;
                        case 'S':
                                sendState();
                                if (_stateMachine.getState() == STATE_SENDING) {
                                        _stateMachine.handleEvent(EVENT_ACTION_SENT, t);
                                }
                                break;
                        case 'M':
                                if (_parser.has_string() && _parser.length() == 1) {
                                        _menu.setMenuItem(_parser.string(), _parser.value());
                                        _out->println(ok);
                                } else {
                                        _out->println(errorBadArgs);
                                }
                                break;
                        case '?':
                                _out->println("?[\"RomiControlPanel\",\"0.2\"]"); 
                                break;
                        }
                }
        }
}

