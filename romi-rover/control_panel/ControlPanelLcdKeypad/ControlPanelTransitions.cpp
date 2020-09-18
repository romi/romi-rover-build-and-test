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
#include "ControlPanelTransitions.h"
#include "pins.h"

int Ready::doTransition()
{
        _display->clear();
        _display->setCursor(0, 0);
        _display->print(getStateString(STATE_OFF));
        return 0;
}

int StartUp::doTransition()
{
        _display->clear();
        _display->setCursor(0, 0);
        _display->print(getStateString(STATE_STARTING_UP));
        
        _relayControlCircuit->close();
        _relayPowerCircuit->open();
        
        return 0;
}

int PowerUp::doTransition()
{
        _display->clear();
        _display->setCursor(0, 0);
        _display->print(getStateString(STATE_ON));
        
        _relayControlCircuit->close();
        _relayPowerCircuit->close();
        
        return 0;
}

int Shutdown::doTransition()
{
        _display->clear();
        _display->setCursor(0, 0);
        _display->print(getStateString(STATE_SHUTTING_DOWN));
        
        _relayControlCircuit->close();
        _relayPowerCircuit->open();
        
        _timer->setTimeout(5000, EVENT_POWERDOWN);
        return 0;
}

int SoftPowerDown::doTransition()
{
        _display->clear();
        _display->setCursor(0, 0);
        _display->print(getStateString(STATE_OFF));
        
        _relayControlCircuit->open();
        _relayPowerCircuit->open();
        
        return 0;
}

int HardPowerDown::doTransition()
{
        _display->clear();
        _display->setCursor(0, 0);
        _display->print(getStateString(STATE_OFF));
        
        _relayControlCircuit->open();
        _relayPowerCircuit->open();
        
        return 0;
}

int ShowMenu::doTransition()
{
        _display->clear();
        _display->setCursor(0, 0);
        _display->print(getStateString(STATE_MENU));
        
        _menu->firstMenuItem();
        _display->setCursor(0, 1);
        _display->print(_menu->currentMenuItemName());
        return 0;
}

int HideMenu::doTransition()
{
        _display->clear();
        _display->setCursor(0, 0);
        _display->print(getStateString(STATE_ON));
        return 0;
}

int NextMenuItem::doTransition()
{
        _display->clear();
        _display->setCursor(0, 0);
        _display->print(getStateString(STATE_MENU));
        
        _menu->nextMenuItem();
        _display->setCursor(0, 1);
        _display->print(_menu->currentMenuItemName());
        return 0;
}

int PreviousMenuItem::doTransition()
{
        _display->clear();
        _display->setCursor(0, 0);
        _display->print(getStateString(STATE_MENU));
        
        _menu->nextMenuItem();
        _display->setCursor(0, 1);
        _display->print(_menu->currentMenuItemName());
        return 0;
}

int SelectMenuItem::doTransition()
{
        _display->clear();
        _display->setCursor(0, 0);
        _display->print("Confirm?");
        return 0;
}

int SendingMenuItem::doTransition()
{
        _display->clear();
        _display->setCursor(0, 0);
        _display->print("Sending");
        _timer->setTimeout(5000, EVENT_ACTION_TIMEOUT);
        return 0;
}

int CancelMenuItem::doTransition()
{
        _display->clear();
        _display->setCursor(0, 0);
        _display->print(getStateString(STATE_MENU));
        
        _display->setCursor(0, 1);
        _display->print(_menu->currentMenuItemName());
        return 0;
}

int SentMenuItem::doTransition()
{
        _display->clear();
        _display->setCursor(0, 0);
        _display->print(getStateString(STATE_ON));
        return 0;
}

int TimeoutMenuItem::doTransition()
{
        _display->clear();
        _display->setCursor(0, 0);
        _display->print(getStateString(STATE_ON));
        _display->setCursor(0, 1);
        _display->print("Sending timeout");
        return 0;
}


