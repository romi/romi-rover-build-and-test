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

#include "ControlPanelTransitions.h"
#include "StateMachineTimer.h"
#include "Parser.h"
#include "ArduinoImpl.h"
#include "ArduinoSerial.h"
#include "Menu.h"
#include "CrystalDisplay.h"
#include "ControlPanel.h"
#include "ButtonPanel.h"
#include "AnalogButton.h"
#include "Relay.h"
#include "pins.h"
#include "Buttons.h"

// The main components
ArduinoImpl arduino;
ArduinoSerial serial;
CrystalDisplay display(PIN_RS,  PIN_EN,  PIN_D4,  PIN_D5,  PIN_D6,  PIN_D7);
ButtonPanel buttonPanel(&arduino);
Parser parser("M", "01S?");
Menu menu;
StateMachine stateMachine;
StateMachineTimer stateMachineTimer(&stateMachine, &arduino);

// One control panel to unity them all
ControlPanel controlPanel(&arduino, &serial, &display, &buttonPanel, &parser,
                          &menu, &stateMachine, &stateMachineTimer);

// The relays
Relay relayControlCircuit(&arduino, PIN_RELAY1);
Relay relayPowerCircuit(&arduino, PIN_RELAY2);

// The state transitions
Ready ready(&display);
StartUp startUp(&display, &relayControlCircuit, &relayPowerCircuit);
PowerUp powerUp(&display, &relayControlCircuit, &relayPowerCircuit);
IgnorePowerUpWhenOn ignorePowerUp;
Shutdown shutdown(&display, &relayControlCircuit, &relayPowerCircuit, &stateMachineTimer);
SoftPowerDown softPowerDown(&display, &relayControlCircuit, &relayPowerCircuit);
HardPowerDown hardPowerDownWhenOn(&display, &relayControlCircuit,
                                  &relayPowerCircuit, STATE_ON);
HardPowerDown hardPowerDownWhenStarting(&display, &relayControlCircuit,
                                        &relayPowerCircuit, STATE_STARTING_UP);
HardPowerDown hardPowerDownWhenShuttingDown(&display, &relayControlCircuit,
                                            &relayPowerCircuit, STATE_SHUTTING_DOWN);
ShowMenu showMenu(&display, &menu);
HideMenu hideMenu(&display);
NextMenuItem nextMenuItem(&display, &menu);
PreviousMenuItem previousMenuItem(&display, &menu);
SelectMenuItem selectMenuItem(&display, &menu);
SendingMenuItem sendingMenuItem(&display, &stateMachineTimer);
CancelMenuItem cancelMenuItem(&display, &menu);
SentMenuItem sentMenuItem(&display);
TimeoutMenuItem timeoutMenuItem(&display);

// The buttons
AnalogButton selectButton(&arduino, 0, BUTTON_SELECT, 0, 50);
AnalogButton upButton(&arduino, 0, BUTTON_UP, 50, 175);
AnalogButton downButton(&arduino, 0, BUTTON_DOWN, 175, 325);
AnalogButton menuButton(&arduino, 0, BUTTON_MENU, 325, 520);
AnalogButton onoffButton(&arduino, 0, BUTTON_ONOFF, 520, 850);

void init_state_machine()
{
        stateMachine.add(&ready);
        stateMachine.add(&startUp);
        stateMachine.add(&powerUp);
        stateMachine.add(&ignorePowerUp);
        stateMachine.add(&shutdown);
        stateMachine.add(&softPowerDown);
        stateMachine.add(&hardPowerDownWhenOn);
        stateMachine.add(&hardPowerDownWhenStarting);
        stateMachine.add(&hardPowerDownWhenShuttingDown);
        stateMachine.add(&showMenu);
        stateMachine.add(&hideMenu);
        stateMachine.add(&nextMenuItem);
        stateMachine.add(&previousMenuItem);        
        stateMachine.add(&selectMenuItem);
        stateMachine.add(&sendingMenuItem);
        stateMachine.add(&cancelMenuItem);
        stateMachine.add(&sentMenuItem);
        stateMachine.add(&timeoutMenuItem);
}

void init_button_panel()
{
        buttonPanel.addButton(&selectButton);
        buttonPanel.addButton(&upButton);
        buttonPanel.addButton(&downButton);
        buttonPanel.addButton(&menuButton);
        buttonPanel.addButton(&onoffButton);
}

void setup()
{
        controlPanel.init();        
        init_state_machine();
        init_button_panel();
        stateMachine.handleEvent(EVENT_READY);        
}

void loop()
{
        controlPanel.loop();
}
