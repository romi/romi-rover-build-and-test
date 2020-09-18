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
#include "StateTransition.h"
#include "StateMachine.h"
#include "IStateMachineTimer.h"
#include "IArduino.h"
#include "IDisplay.h"
#include "IRelay.h"
#include "IMenu.h"
#include "States.h"
#include "Events.h"

#ifndef __STATE_MACHINE_CONTROL_PANEL_H
#define __STATE_MACHINE_CONTROL_PANEL_H

const char* getStateString(int state);

class Ready : public StateTransition
{
protected:
        IDisplay *_display;
        
public:
        Ready(IDisplay *display) : StateTransition(STATE_START,
                                                    EVENT_READY,
                                                    STATE_OFF),
                                    _display(display) {}
        
        virtual int doTransition();
};

class StartUp : public StateTransition
{
protected:
        IDisplay *_display;
        IRelay *_relayControlCircuit;
        IRelay *_relayPowerCircuit;
        
public:
        StartUp(IDisplay *display,
                IRelay *relayControlCircuit,
                IRelay *relayPowerCircuit)
                : StateTransition(STATE_OFF,
                                  EVENT_ONOFF_HELD,
                                  STATE_STARTING_UP),
                  _display(display),
                  _relayControlCircuit(relayControlCircuit),
                  _relayPowerCircuit(relayPowerCircuit) {}
        
        virtual int doTransition();
};

class PowerUp : public StateTransition
{
protected:
        IDisplay *_display;
        IRelay *_relayControlCircuit;
        IRelay *_relayPowerCircuit;
        
public:
        // The power up event is sent by the control software when
        // the controllers are initialized and ready to go.
        PowerUp(IDisplay *display,
                IRelay *relayControlCircuit,
                IRelay *relayPowerCircuit)
                : StateTransition(STATE_STARTING_UP,
                                  EVENT_POWERUP,
                                  STATE_ON),
                  _display(display),
                  _relayControlCircuit(relayControlCircuit),
                  _relayPowerCircuit(relayPowerCircuit) {}
                
        virtual int doTransition();
};

class IgnorePowerUpWhenOn : public StateTransition
{
public:
        // Ignore the power down event if the state is already
        // on. This may happen when the control panel node reboots
        // when the rover is running.
        IgnorePowerUpWhenOn() : StateTransition(STATE_ON,
                                                EVENT_POWERUP,
                                                STATE_ON) {}
        
        virtual int doTransition() { return 0; }
};

class Shutdown : public StateTransition
{
protected:
        IDisplay *_display;
        IRelay *_relayControlCircuit;
        IRelay *_relayPowerCircuit;
        IStateMachineTimer *_timer;
        
public:
        // The shutdown event is sent by the control software when the
        // user requested to turn of the robot trough the user
        // interface. It powers off the motors and waits 5s before
        // shutting down the controllers. This gives the controller
        // the time to shut down cleanly.
        Shutdown(IDisplay *display,
                 IRelay *relayControlCircuit,
                 IRelay *relayPowerCircuit,
                 IStateMachineTimer *timer)
                : StateTransition(STATE_ON,
                                  EVENT_SHUTDOWN,
                                  STATE_SHUTTING_DOWN),
                  _display(display),
                  _relayControlCircuit(relayControlCircuit),
                  _relayPowerCircuit(relayPowerCircuit),
                  _timer(timer) {}
        
        virtual int doTransition();
};

class SoftPowerDown : public StateTransition
{
protected:
        IDisplay *_display;
        IRelay *_relayControlCircuit;
        IRelay *_relayPowerCircuit;
        
public:
        // The soft power down event is sent 5s after the shutdown
        // event.
        SoftPowerDown(IDisplay *display,
                      IRelay *relayControlCircuit,
                      IRelay *relayPowerCircuit)
                : StateTransition(STATE_SHUTTING_DOWN,
                                  EVENT_POWERDOWN,
                                  STATE_OFF),
                  _display(display),
                  _relayControlCircuit(relayControlCircuit),
                  _relayPowerCircuit(relayPowerCircuit) {}
        
        virtual int doTransition();
};

class HardPowerDown : public StateTransition
{
protected:
        IDisplay *_display;
        IRelay *_relayControlCircuit;
        IRelay *_relayPowerCircuit;
        
public:
        // The hard power down event is sent when the user presses and
        // holds the select button. It powers off the motors and the
        // controllers.
        HardPowerDown(IDisplay *display,
                      IRelay *relayControlCircuit,
                      IRelay *relayPowerCircuit,
                      int from)
                : StateTransition(from,
                                  EVENT_ONOFF_HELD,
                                  STATE_OFF),
                  _display(display),
                  _relayControlCircuit(relayControlCircuit),
                  _relayPowerCircuit(relayPowerCircuit) {}
        
        virtual int doTransition();
};

class ShowMenu : public StateTransition
{
protected:
        IDisplay *_display;
        IMenu* _menu;
        
public:
        ShowMenu(IDisplay *display, IMenu* menu)
                : StateTransition(STATE_ON,
                                  EVENT_MENU_HELD,
                                  STATE_MENU),
                  _display(display),_menu(menu) {}
        
        virtual int doTransition();
};

class HideMenu : public StateTransition
{
protected:
        IDisplay *_display;
        
public:
        HideMenu(IDisplay *display)
                : StateTransition(STATE_MENU,
                                  EVENT_MENU_PRESSED,
                                  STATE_ON),
                  _display(display) {}
        
        virtual int doTransition();
};

class NextMenuItem : public StateTransition
{
protected:
        IDisplay *_display;
        IMenu *_menu;
        
public:
        NextMenuItem(IDisplay *display, IMenu* menu)
                : StateTransition(STATE_MENU,
                                  EVENT_DOWN_PRESSED,
                                  STATE_MENU),
                  _display(display), _menu(menu) {}
        
        virtual int doTransition();
};

class PreviousMenuItem : public StateTransition
{
protected:
        IDisplay *_display;
        IMenu *_menu;
        
public:
        PreviousMenuItem(IDisplay *display, IMenu* menu)
                : StateTransition(STATE_MENU,
                                  EVENT_UP_PRESSED,
                                  STATE_MENU),
                  _display(display), _menu(menu) {}
        
        virtual int doTransition();
};

class SelectMenuItem : public StateTransition
{
protected:
        IDisplay *_display;
        IMenu *_menu;
        
public:
        SelectMenuItem(IDisplay *display, IMenu* menu)
                : StateTransition(STATE_MENU,
                                  EVENT_SELECT_PRESSED,
                                  STATE_CONFIRM),
                  _display(display), _menu(menu) {}
        
        virtual int doTransition();
};

class SendingMenuItem : public StateTransition
{
protected:
        IDisplay *_display;
        IStateMachineTimer *_timer;
        
public:
        SendingMenuItem(IDisplay *display, IStateMachineTimer *timer)
                : StateTransition(STATE_CONFIRM,
                                  EVENT_SELECT_PRESSED,
                                  STATE_SENDING),
                  _display(display), _timer(timer) {}
        
        virtual int doTransition();
};

class CancelMenuItem : public StateTransition
{
protected:
        IDisplay *_display;
        IMenu *_menu;
        
public:
        CancelMenuItem(IDisplay *display, IMenu* menu)
                : StateTransition(STATE_CONFIRM,
                                  EVENT_MENU_PRESSED,
                                  STATE_MENU),
                  _display(display), _menu(menu) {}
        
        virtual int doTransition();
};

class SentMenuItem : public StateTransition
{
protected:
        IDisplay *_display;
        
public:
        SentMenuItem(IDisplay *display)
                : StateTransition(STATE_SENDING,
                                  EVENT_ACTION_SENT,
                                  STATE_ON),
                  _display(display) {}
        
        virtual int doTransition();
};

class TimeoutMenuItem : public StateTransition
{
protected:
        IDisplay *_display;
        
public:
        TimeoutMenuItem(IDisplay *display)
                : StateTransition(STATE_SENDING,
                                  EVENT_ACTION_TIMEOUT,
                                  STATE_ON),
                  _display(display) {}
        
        virtual int doTransition();
};

#endif // __STATE_MACHINE_CONTROL_PANEL_H
