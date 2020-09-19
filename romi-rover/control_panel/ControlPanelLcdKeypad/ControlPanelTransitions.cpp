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
#include "ControlPanel.h"
#include "pins.h"
#include <stdio.h>

void ControlPanelTransition::displayState()
{
        _controlPanel->display()->showState(getStateString(_to));
}
        
void ControlPanelTransition::displayMenu()
{
        const char *s = _controlPanel->menu()->currentMenuItemName();
        _controlPanel->display()->showMenu(s);
}

void ControlPanelTransition::hideMenu()
{
        _controlPanel->display()->clearMenu();
}

void ControlPanelTransition::setTimer(unsigned long when, int event)
{
        _controlPanel->timer()->setTimeout(when, event);
}

void ControlPanelTransition::doTransition(unsigned long t)
{
        displayState();
}

void Ready::doTransition(unsigned long t)
{
        displayState();
        hideMenu();
}

void StartUp::doTransition(unsigned long t)
{
        displayState();
        _controlPanel->controlRelay()->close();
        _controlPanel->powerRelay()->open();
}

void PowerUp::doTransition(unsigned long t)
{
        displayState();
        _controlPanel->controlRelay()->close();
        _controlPanel->powerRelay()->close();
}

void Shutdown::doTransition(unsigned long t)
{
        displayState();
        _controlPanel->controlRelay()->close();
        _controlPanel->powerRelay()->open();
        setTimer(t + 7000, EVENT_POWERDOWN);
}

void SoftPowerDown::doTransition(unsigned long t)
{
        displayState();
        _controlPanel->controlRelay()->open();
        _controlPanel->powerRelay()->open();
}

void HardPowerDown::doTransition(unsigned long t)
{
        displayState();
        _controlPanel->controlRelay()->open();
        _controlPanel->powerRelay()->open();
}

void ShowMenu::doTransition(unsigned long t)
{
        displayState();
        _controlPanel->menu()->firstMenuItem();
        displayMenu();
}

void NextMenuItem::doTransition(unsigned long t)
{
        displayState();        
        _controlPanel->menu()->nextMenuItem();
        displayMenu();
}

void PreviousMenuItem::doTransition(unsigned long t)
{
        displayState();
        _controlPanel->menu()->nextMenuItem();
        displayMenu();
}

void SendingMenuItem::doTransition(unsigned long t)
{
        displayState();
        setTimer(t + 5000, EVENT_ACTION_TIMEOUT);
}

void CancelMenuItem::doTransition(unsigned long t)
{
        displayState();
        hideMenu();
}

void SentMenuItem::doTransition(unsigned long t)
{
        displayState();
        hideMenu();
}

void TimeoutMenuItem::doTransition(unsigned long t)
{
        displayState();
        hideMenu();
}


