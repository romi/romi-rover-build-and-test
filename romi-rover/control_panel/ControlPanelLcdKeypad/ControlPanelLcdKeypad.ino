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

#include <LiquidCrystal.h>
#include "StateMachineControlPanel.h"
#include "Parser.h"

#define PIN_RS 8 
#define PIN_EN 9 
#define PIN_D4 4 
#define PIN_D5 5 
#define PIN_D6 6 
#define PIN_D7 7 
#define PIN_RELAY1 A1
#define PIN_RELAY2 A2

LiquidCrystal lcd(PIN_RS,  PIN_EN,  PIN_D4,  PIN_D5,  PIN_D6,  PIN_D7);

/**************************************************************************/

StateMachineControlPanel stateMachine;

const char* getStateString();

/**************************************************************************/

class Timer
{
protected:
        int _enabled;
        int _event;
        unsigned long _timeout;
public:
        Timer() : _enabled(0), _event(0), _timeout(0) {}

        void setTimeout(int milliseconds, int event) {
                _enabled = 1;
                _timeout = millis() + milliseconds;
                _event = event;
        }

        void update(unsigned long t) {
                if (_enabled && t > _timeout) {
                        stateMachine.handleEvent(_event);
                        // int r = stateMachine.handleEvent(_event);
                        // if (r == StateMachine::OK)
                        //         Serial.println("OK");
                        // else if (r == StateMachine::Ignored)
                        //         Serial.println("bad state");
                        // else 
                        //         Serial.println("failed");
                        _enabled = 0;
                }
        }
};

Timer poweroffTimer;

/**************************************************************************/

enum {
        BUTTON_RELEASED,
        BUTTON_PRESSED,
        BUTTON_HELD
};

enum {
        BUTTON_LEFT = 0,
        BUTTON_RIGHT,
        BUTTON_UP,
        BUTTON_DOWN,
        BUTTON_SELECT,
        BUTTON_LAST
};

int button_state[BUTTON_LAST];
unsigned long button_timestamp[BUTTON_LAST];

void button_pressed(int i)
{
        //Serial.print("Pressed["); Serial.print(i); Serial.println("]");
        // ignore
}

void button_released(int i)
{
        //Serial.print("Released["); Serial.print(i); Serial.println("]");
        // ignore
}

void button_held(int i)
{
        //Serial.print("Held["); Serial.print(i); Serial.println("]");
        if (i == BUTTON_SELECT) {
                //Serial.println("handleEvent(EVENT_SELECT_HELD)");
                stateMachine.handleEvent(EVENT_SELECT_HELD);
        }
}

#define time_pressed(__t,__id) ((__t) - button_timestamp[__id])

void update_button(int i, int pressed, unsigned long t)
{
        if (button_state[i] == BUTTON_RELEASED) {
                if (pressed) {
                        button_state[i] = BUTTON_PRESSED;
                        button_timestamp[i] = t;
                        button_pressed(i);
                }
                
        } else if (button_state[i] == BUTTON_PRESSED) {
                if (!pressed) {
                        button_state[i] = BUTTON_RELEASED;
                        button_released(i);
                } else if (time_pressed(t, i) > 5000) {
                        button_state[i] = BUTTON_HELD;
                        button_held(i);
                }
        } else if (button_state[i] == BUTTON_HELD) {
                if (!pressed) {
                        button_state[i] = BUTTON_RELEASED;
                        button_released(i);
                } 
        }
}

#define in(__x,__min,__max) (((__min) <= (__x)) && ((__x) < (__max)))

void updateButtons(unsigned long t)
{
        int x = analogRead(0);
        //if (x < 1000) Serial.println(x);
#if defined(ARDUINO_AVR_UNO)
        // Uno (5V)
        update_button(BUTTON_RIGHT, in(x, 0, 50), t); // 0 
        update_button(BUTTON_UP, in(x, 50, 175), t); // 99 
        update_button(BUTTON_DOWN, in(x, 175, 325), t); // 255
        update_button(BUTTON_LEFT, in(x, 325, 520), t); // 407
        update_button(BUTTON_SELECT, in(x, 520, 850), t); // 640
#else
        // Arduino Zero (3.3V...!!??)
        update_button(BUTTON_RIGHT, in(x, 0, 50), t); // 2
        update_button(BUTTON_UP, in(x, 50, 250), t); // 155
        update_button(BUTTON_DOWN, in(x, 250, 510), t); // 395
        update_button(BUTTON_LEFT, in(x, 510, 800), t); // 630  
        update_button(BUTTON_SELECT, in(x, 800, 1000), t); // 987
#endif
}

int StartUp::doTransition()
{
        digitalWrite(PIN_RELAY1, HIGH);
        digitalWrite(PIN_RELAY2, LOW);
        return 0;
}

int PowerUp::doTransition()
{
        digitalWrite(PIN_RELAY1, HIGH);
        digitalWrite(PIN_RELAY2, HIGH);
        return 0;
}

int Shutdown::doTransition()
{
        digitalWrite(PIN_RELAY1, HIGH);
        digitalWrite(PIN_RELAY2, LOW);
        poweroffTimer.setTimeout(5000, EVENT_POWERDOWN);
        return 0;
}

int SoftPowerDown::doTransition()
{
        digitalWrite(PIN_RELAY1, LOW);
        digitalWrite(PIN_RELAY2, LOW);
        return 0;
}

int HardPowerDown::doTransition()
{
        digitalWrite(PIN_RELAY1, LOW);
        digitalWrite(PIN_RELAY2, LOW);
        return 0;
}

const char* getStateString()
{
        switch (stateMachine.getState()) {
        case StateMachine::ErrorState: return "Error"; 
        case StateMachine::StartState: return "..."; 
        case STATE_OFF: return "Off"; 
        case STATE_STARTING_UP: return "Starting up"; 
        case STATE_SHUTTING_DOWN: return "Shutting down"; 
        case STATE_ON: return "On"; 
        }
}

void displayState()
{
        lcd.setCursor(0, 0);
        lcd.print("                ");
        lcd.setCursor(0, 0);
        lcd.print(getStateString());
}

void updateDisplay(unsigned long t)
{
        static unsigned long _lastState = -1;
        if (stateMachine.getState() != _lastState) {
                displayState();
                _lastState = stateMachine.getState();
        }
}

Parser parser("", "01s?");

void handleSerialInput()
{
        while (Serial.available()) {
                char c = Serial.read();
                if (parser.process(c)) {                        
                        switch (parser.opcode()) {
                        default: break;
                        case '0':
                                if (parser.length() == 0) {
                                        int r = stateMachine.handleEvent(EVENT_SHUTDOWN);
                                        if (r == StateMachine::OK)
                                                Serial.println("OK shutdown");
                                        else if (r == StateMachine::Ignored)
                                                Serial.println("ERR bad state");
                                        else 
                                                Serial.println("ERR shutdown failed");
                                } else {
                                        Serial.println("ERR bad args");
                                }
                                break;
                        case '1':
                                if (parser.length() == 0) {
                                        int r = stateMachine.handleEvent(EVENT_POWERUP);
                                        if (r == StateMachine::OK)
                                                Serial.println("OK power up");
                                        else if (r == StateMachine::Ignored)
                                                Serial.println("ERR bad state");
                                        else 
                                                Serial.println("ERR power up failed");
                                } else {
                                        Serial.println("ERR bad args");
                                }
                                break;
                        case 's':
                                Serial.print("s'");
                                Serial.print(getStateString());
                                Serial.println("'");
                                break;
                        // case 'D':
                        //         display1(parser.text());
                        //         Serial.println("OK display");
                        //         break;
                        case '?':
                                Serial.println("?[\"RomiControlPanel\",\"0.1\"]"); 
                                break;
                        }
                }
        }
}

void setup()
{
        lcd.begin(16, 2);
        lcd.setCursor(0,0);
        displayState();
        
        Serial.begin(115200);
        while (!Serial)
                ;

        pinMode(PIN_RELAY1, OUTPUT);
        pinMode(PIN_RELAY2, OUTPUT);
        digitalWrite(PIN_RELAY1, LOW);
        digitalWrite(PIN_RELAY2, LOW);
        
        for (int i = 0; i < BUTTON_LAST; i++) {
                button_state[i] = BUTTON_RELEASED;
                button_timestamp[i] = 0;
        }
        
        stateMachine.handleEvent(EVENT_READY);        
}

void loop()
{
        unsigned long t = millis();        
        updateButtons(t);
        handleSerialInput();
        updateDisplay(t);
        poweroffTimer.update(t);
}
