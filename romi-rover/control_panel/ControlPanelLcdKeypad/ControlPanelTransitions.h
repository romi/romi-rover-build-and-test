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
#include "States.h"
#include "Events.h"

#ifndef __CONTROL_PANEL_TRANSITIONS_H
#define __CONTROL_PANEL_TRANSITIONS_H

class ControlPanel;

const char* getStateString(int state);

class ControlPanelTransition : public StateTransition
{
protected:
        ControlPanel *_controlPanel;

        void displayState();
        void displayMenu();
        void hideMenu();
        void setTimer(unsigned long when, int event);

public:
        ControlPanelTransition(ControlPanel *controlPanel,
                               int8_t from, int16_t event, int8_t to)
                : StateTransition(from, event, to),
                  _controlPanel(controlPanel) {}
        
        void doTransition(unsigned long t) override;
};

class Ready : public ControlPanelTransition
{
public:
        Ready(ControlPanel *controlPanel)
                : ControlPanelTransition(controlPanel,
                                         STATE_START,
                                         EVENT_READY,
                                         STATE_OFF) {}
        
        void doTransition(unsigned long t) override;
};

class StartUp : public ControlPanelTransition
{
public:
        StartUp(ControlPanel *controlPanel)
                : ControlPanelTransition(controlPanel,
                                         STATE_OFF,
                                         EVENT_ONOFF_HELD,
                                         STATE_STARTING_UP) {}
        
        void doTransition(unsigned long t) override;
};

class PowerUp : public ControlPanelTransition
{
public:
        // The power up event is sent by the control software when
        // the controllers are initialized and ready to go.
        PowerUp(ControlPanel *controlPanel, int8_t from)
                : ControlPanelTransition(controlPanel,
                                         from,
                                         EVENT_POWERUP,
                                         STATE_ON) {}
                
        void doTransition(unsigned long t) override;
};

class IgnorePowerUpWhenOn : public ControlPanelTransition
{
public:
        // Ignore the power down event if the state is already
        // on. This may happen when the control panel node reboots
        // when the rover is running.
        IgnorePowerUpWhenOn(ControlPanel *controlPanel)
                : ControlPanelTransition(controlPanel,
                                         STATE_ON,
                                         EVENT_POWERUP,
                                         STATE_ON) {}
};

class Shutdown : public ControlPanelTransition
{
public:
        // The shutdown event is sent by holding the on/off button or
        // by the control software or when the user requested to turn
        // of the robot trough the user interface. It powers off the
        // motors and waits 5-10s before shutting down the
        // controllers. This gives the controller the time to shut
        // down cleanly.
        Shutdown(ControlPanel *controlPanel, int event)
                : ControlPanelTransition(controlPanel,
                                         STATE_ON,
                                         event,
                                         STATE_SHUTTING_DOWN) {}
        
        void doTransition(unsigned long t) override;
};

class IgnoreShutdownWhenShuttingDown : public ControlPanelTransition
{
public:
        // Ignore the shutdown event if the state is already shutting
        // down. This may happen when the user requtes a shutdown both
        // from the control panel and through the web interface.
        IgnoreShutdownWhenShuttingDown(ControlPanel *controlPanel)
                : ControlPanelTransition(controlPanel,
                                         STATE_SHUTTING_DOWN,
                                         EVENT_SHUTDOWN,
                                         STATE_SHUTTING_DOWN) {}
};

class SoftPowerDown : public ControlPanelTransition
{
public:
        // The soft power down event is sent 5s after the shutdown
        // event.
        SoftPowerDown(ControlPanel *controlPanel)
                : ControlPanelTransition(controlPanel,
                                         STATE_SHUTTING_DOWN,
                                         EVENT_POWERDOWN,
                                         STATE_OFF) {}
        
        void doTransition(unsigned long t) override;
};

class HardPowerDown : public ControlPanelTransition
{
public:
        // The hard power down event is sent when the user presses and
        // holds the select button. It powers off the motors and the
        // controllers.
        HardPowerDown(ControlPanel *controlPanel)
                : ControlPanelTransition(controlPanel,
                                         ALL_STATES,
                                         EVENT_ONOFF_HELD,
                                         STATE_OFF) {}
        
        void doTransition(unsigned long t) override;
};

class ShowMenu : public ControlPanelTransition
{
public:
        ShowMenu(ControlPanel *controlPanel)
                : ControlPanelTransition(controlPanel,
                                         STATE_ON,
                                         EVENT_MENU_HELD,
                                         STATE_MENU) {}
        
        void doTransition(unsigned long t) override;
};

class HideMenu : public ControlPanelTransition
{
public:
        HideMenu(ControlPanel *controlPanel)
                : ControlPanelTransition(controlPanel,
                                         STATE_MENU,
                                         EVENT_MENU_PRESSED,
                                         STATE_ON) {}
        
        void doTransition(unsigned long t) override;
};

class NextMenuItem : public ControlPanelTransition
{
public:
        NextMenuItem(ControlPanel *controlPanel)
                : ControlPanelTransition(controlPanel,
                                         STATE_MENU,
                                         EVENT_DOWN_PRESSED,
                                         STATE_MENU) {}
        
        void doTransition(unsigned long t) override;
};

class PreviousMenuItem : public ControlPanelTransition
{
public:
        PreviousMenuItem(ControlPanel *controlPanel)
                : ControlPanelTransition(controlPanel,
                                         STATE_MENU,
                                         EVENT_UP_PRESSED,
                                         STATE_MENU) {}
        
        void doTransition(unsigned long t) override;
};

class SelectMenuItem : public ControlPanelTransition
{
public:
        SelectMenuItem(ControlPanel *controlPanel)
                : ControlPanelTransition(controlPanel,
                                         STATE_MENU,
                                         EVENT_SELECT_PRESSED,
                                         STATE_CONFIRM) {}
};

class SendingMenuItem : public ControlPanelTransition
{
public:
        SendingMenuItem(ControlPanel *controlPanel)
                : ControlPanelTransition(controlPanel,
                                         STATE_CONFIRM,
                                         EVENT_SELECT_PRESSED,
                                         STATE_SENDING) {}
        
        void doTransition(unsigned long t) override;
};

class CancelMenuItem : public ControlPanelTransition
{
public:
        CancelMenuItem(ControlPanel *controlPanel)
                : ControlPanelTransition(controlPanel,
                                         STATE_CONFIRM,
                                         EVENT_MENU_PRESSED,
                                         STATE_MENU) {}
};

class SentMenuItem : public ControlPanelTransition
{
public:
        SentMenuItem(ControlPanel *controlPanel)
                : ControlPanelTransition(controlPanel,
                                         STATE_SENDING,
                                         EVENT_ACTION_SENT,
                                         STATE_ON) {}

        void doTransition(unsigned long t) override;
};

class TimeoutMenuItem : public ControlPanelTransition
{
public:
        TimeoutMenuItem(ControlPanel *controlPanel)
                : ControlPanelTransition(controlPanel,
                                         STATE_SENDING,
                                         EVENT_ACTION_TIMEOUT,
                                         STATE_ON) {}
        
        void doTransition(unsigned long t) override;
};

#endif // __CONTROL_PANEL_TRANSITIONS_H
