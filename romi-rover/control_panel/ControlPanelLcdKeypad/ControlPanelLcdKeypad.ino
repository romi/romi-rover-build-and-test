/*
  romi-rover

  Copyright (C) 2019 Sony Computer Science Laboratories
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

#include "EventTimer.h"
#include "ArduinoImpl.h"
#include "ArduinoSerial.h"
#include "CrystalDisplay.h"
#include "ControlPanel.h"
#include "ButtonPanel.h"
#include "AnalogButton.h"
#include "Relay.h"
#include "pins.h"
#include "Buttons.h"

ArduinoImpl arduino;
ArduinoSerial serial;
ButtonPanel buttonPanel;
AnalogButton selectButton(&arduino, 0, BUTTON_SELECT, 0, 50);
AnalogButton upButton(&arduino, 0, BUTTON_UP, 50, 175);
AnalogButton downButton(&arduino, 0, BUTTON_DOWN, 175, 325);
AnalogButton menuButton(&arduino, 0, BUTTON_MENU, 325, 520);
AnalogButton onoffButton(&arduino, 0, BUTTON_ONOFF, 520, 850);
EventTimer eventTimer;
CrystalDisplay display(PIN_RS,  PIN_EN,  PIN_D4,  PIN_D5,  PIN_D6,  PIN_D7);
Relay relayControlCircuit(&arduino, PIN_RELAY1);
Relay relayPowerCircuit(&arduino, PIN_RELAY2);

// One control panel to unity them all
ControlPanel controlPanel(&serial, &buttonPanel, &eventTimer, &display,
                          &relayControlCircuit, &relayPowerCircuit);

void initButtonPanel()
{
        buttonPanel.addButton(&selectButton);
        buttonPanel.addButton(&upButton);
        buttonPanel.addButton(&downButton);
        buttonPanel.addButton(&menuButton);
        buttonPanel.addButton(&onoffButton);
}

void setup()
{
        initButtonPanel();
        serial.init(115200);
        controlPanel.init();        
}

void loop()
{
        unsigned long t = ::millis();
        controlPanel.update(t);
}
