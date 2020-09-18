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
        _serial->init(115200);
}

void ControlPanel::loop()
{
        _buttonPanel->update(_stateMachine);
        handleSerialInput();
        _timer->update(_arduino->millis());
}

void ControlPanel::sendState()
{
        static char buffer[64];
        snprintf(buffer, sizeof(buffer),
                 "S[\"%s\",%d]",
                 getStateString(_stateMachine->getState()),
                 _menu->currentMenuItemID());
        _serial->println(buffer);
        
        if (_stateMachine->getState() == STATE_SENDING) {
                _stateMachine->handleEvent(EVENT_ACTION_SENT);
        }
}

void ControlPanel::handleSerialInput()
{
        while (_serial->available()) {
                int c = _serial->read();
                if (_parser->process(c)) {                        
                        switch (_parser->opcode()) {
                        default: break;
                        case '0':
                                if (_parser->length() == 0) {
                                        int r = _stateMachine->handleEvent(EVENT_SHUTDOWN);
                                        if (r == IStateMachine::OK)
                                                _serial->println(ok);
                                        else if (r == IStateMachine::Ignored)
                                                _serial->println(errorBadState);
                                        else 
                                                _serial->println(errorFail);
                                } else {
                                        _serial->println(errorBadArgs);
                                }
                                break;
                        case '1':
                                if (_parser->length() == 0) {
                                        int r = _stateMachine->handleEvent(EVENT_POWERUP);
                                        if (r == IStateMachine::OK)
                                                _serial->println(ok);
                                        else if (r == IStateMachine::Ignored)
                                                _serial->println(errorBadState);
                                        else 
                                                _serial->println(errorFail);
                                } else {
                                        _serial->println(errorBadArgs);
                                }
                                break;
                        case 'S':
                                sendState();
                                break;
                        case 'M':
                                if (_parser->has_string() && _parser->length() == 1) {
                                        _menu->setMenuItem(_parser->string(),
                                                           _parser->value());
                                        _serial->println(ok);
                                } else {
                                        _serial->println(errorBadArgs);
                                }
                                break;
                        case '?':
                                _serial->println("?[\"RomiControlPanel\",\"0.1\"]"); 
                                break;
                        }
                }
        }
}

