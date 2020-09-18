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
#ifndef __BUTTON_PANEL_H
#define __BUTTON_PANEL_H

#include "IButtonPanel.h"
#include "IButton.h"
#include "IArduino.h"
#include "IStateMachine.h"
#include "Events.h"

#define BUTTONPANEL_MAX_BUTTONS 5

class ButtonPanel : public IButtonPanel
{
protected:
        IArduino *_arduino;
        IButton *_buttons[BUTTONPANEL_MAX_BUTTONS];
        uint8_t _numButtons;
        
public:
        ButtonPanel(IArduino *arduino) : _arduino(arduino), _numButtons(0) {}
        
        virtual ~ButtonPanel() {}

        virtual void addButton(IButton *button) {
                if (_numButtons < BUTTONPANEL_MAX_BUTTONS) {
                        _buttons[_numButtons++] = button;
                } // else ?!
        }
        
        virtual void update(IStateMachine *stateMachine) {
                unsigned long t = _arduino->millis();
                for (int i = 0; i < _numButtons; i++) {
                        uint8_t e = _buttons[i]->update(t);
                        if (e != 0) {
                                int event = ButtonEvent(_buttons[i]->id(), e);
                                stateMachine->handleEvent(event);
                        }
                }
        }
};

#endif // __BUTTON_PANEL_H
