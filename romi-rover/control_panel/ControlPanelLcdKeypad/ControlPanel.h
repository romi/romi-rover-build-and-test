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
#include "IInputStream.h"
#include "IOutputStream.h"
#include "IButtonPanel.h"
#include "IDisplay.h"
#include "IRelay.h"

#include "Menu.h"
#include "Parser.h"
#include "StateMachine.h"
#include "EventTimer.h"
#include "ControlPanelTransitions.h"

class ControlPanel
{
protected:
        // Inputs
        IInputStream *_in;
        IButtonPanel *_buttonPanel;
        IEventTimer *_eventTimer;
        
        // Outputs
        IOutputStream *_out;
        IDisplay *_display;
        IRelay *_relayControlCircuit;
        IRelay *_relayPowerCircuit;

        // Helpers
        Parser _parser;
        Menu _menu;
        StateMachine _stateMachine;

        // State transitions
        Ready _ready;
        StartUp _startUp;
        PowerUp _powerUp;
        IgnorePowerUpWhenOn _ignorePowerUp;
        Shutdown _shutdownFromSoftware;
        Shutdown _shutdownFromControlPanel;
        IgnoreShutdownWhenShuttingDown _ignoreShutdown;
        SoftPowerDown _softPowerDown;
        HardPowerDown _hardPowerDown;
        ShowMenu _showMenu;
        HideMenu _hideMenu;
        NextMenuItem _nextMenuItem;
        PreviousMenuItem _previousMenuItem;
        SelectMenuItem _selectMenuItem;
        SendingMenuItem _sendingMenuItem;
        CancelMenuItem _cancelMenuItem;
        SentMenuItem _sentMenuItem;
        TimeoutMenuItem _timeoutMenuItem;
        
        void initStateMachine()
        {
                _stateMachine.add(&_ready);
                _stateMachine.add(&_startUp);
                _stateMachine.add(&_powerUp);
                _stateMachine.add(&_ignorePowerUp);
                _stateMachine.add(&_shutdownFromSoftware);
                _stateMachine.add(&_shutdownFromControlPanel);
                _stateMachine.add(&_ignoreShutdown);
                _stateMachine.add(&_softPowerDown);
                _stateMachine.add(&_hardPowerDown);
                _stateMachine.add(&_showMenu);
                _stateMachine.add(&_hideMenu);
                _stateMachine.add(&_nextMenuItem);
                _stateMachine.add(&_previousMenuItem);        
                _stateMachine.add(&_selectMenuItem);
                _stateMachine.add(&_sendingMenuItem);
                _stateMachine.add(&_cancelMenuItem);
                _stateMachine.add(&_sentMenuItem);
                _stateMachine.add(&_timeoutMenuItem);
        }

        void handleSerialInput(unsigned long t);
        void sendState();
        
public:
        ControlPanel(IInputStream *in,
                     IButtonPanel *buttonPanel,
                     IEventTimer *eventTimer,
                     IOutputStream *out,
                     IDisplay *display,
                     IRelay *relayControlCircuit,
                     IRelay *relayPowerCircuit)
                : _in(in),
                _buttonPanel(buttonPanel),
                _eventTimer(eventTimer),
                _out(out),
                _display(display),
                _relayControlCircuit(relayControlCircuit),
                _relayPowerCircuit(relayPowerCircuit),
                _parser("M", "01S?"),
                _ready(this),
                _startUp(this),
                _powerUp(this),
                _ignorePowerUp(this),
                _shutdownFromSoftware(this, EVENT_SHUTDOWN),
                _shutdownFromControlPanel(this, EVENT_ONOFF_HELD),
                _ignoreShutdown(this),
                _softPowerDown(this),
                _hardPowerDown(this),
                _showMenu(this),
                _hideMenu(this),
                _nextMenuItem(this),
                _previousMenuItem(this),
                _selectMenuItem(this),
                _sendingMenuItem(this),
                _cancelMenuItem(this),
                _sentMenuItem(this),
                _timeoutMenuItem(this) {} 

        virtual ~ControlPanel() {}

        void init();
        void update(unsigned long t);

        IDisplay *display() {
                return _display;
        }

        IMenu *menu() {
                return &_menu;
        }

        IEventTimer *timer() {
                return _eventTimer;
        }
        
        IRelay *controlRelay() {
                return _relayControlCircuit;
        }
        
        IRelay *powerRelay() {
                return _relayPowerCircuit;
        }
        
        IStateMachine *stateMachine() {
                return &_stateMachine;
        }
};

#endif // __ICONTROL_PANEL_H
