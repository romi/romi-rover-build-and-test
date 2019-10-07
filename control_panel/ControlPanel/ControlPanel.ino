/*
  ControlPanel

  Copyright (C) 2018-2019 Sony Computer Science Laboratories
  Author(s) Peter Hanappe

  ControlPanel is a firmware for Arduino. It manages the on/off
  buttons and the status display for the Romi rover.

  ControlPanel is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation, either version 3 of the
  License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see
  <http://www.gnu.org/licenses/>.

 */

#include "Parser.h"

#define pinRelay1 8
#define pinRelay2 9
#define pinOnButton 10
#define pinLedOnButton 11
#define pinOffButton 12
#define pinLedOffButton 13

enum {
        STATE_ERROR = -1,
        STATE_OFF = 0,
        STATE_ON = 1,
        STATE_STARTING_UP = 2,
        STATE_SHUTTING_DOWN = 3
};

int state = STATE_OFF;
int blink_count = 0;
unsigned long off_timestamp;

Parser parser("SD", "s?");

void setup()
{
        Serial.begin(115200);
        while (!Serial)
                ;

        pinMode(pinOnButton, INPUT_PULLUP);
        pinMode(pinOffButton, INPUT_PULLUP);
        pinMode(pinLedOnButton, OUTPUT);
        pinMode(pinLedOffButton, OUTPUT);
        pinMode(pinRelay1, OUTPUT);
        pinMode(pinRelay2, OUTPUT);

        digitalWrite(pinRelay1, LOW);
        digitalWrite(pinRelay2, LOW);
}

int onButtonPressed()
{
        return (digitalRead(pinOnButton) == LOW);
}

int offButtonPressed()
{
        return (digitalRead(pinOffButton) == LOW);
}

void updateState()
{
        if (onButtonPressed()) {
                Serial.println("on");
                switch (state) {
                case STATE_OFF:
                        Serial.println("starting up");
                        state = STATE_STARTING_UP;
                        blink_count = 0;
                        break;
                case STATE_STARTING_UP:
                case STATE_ON:
                case STATE_SHUTTING_DOWN: 
                case STATE_ERROR:
                        break;
                }
        }
        if (offButtonPressed()) {
                Serial.println("off");
                switch (state) {
                case STATE_ON:
                        Serial.println("shutting down");
                        state = STATE_SHUTTING_DOWN;
                        blink_count = 0;
                        break;
                case STATE_OFF:
                case STATE_STARTING_UP:
                case STATE_SHUTTING_DOWN: 
                case STATE_ERROR:
                        break;
                }
        }
        if (
}

void setState(int value)
{
        switch (state) {
        case STATE_STARTING_UP:
        case STATE_ON:
        case STATE_SHUTTING_DOWN: 
        case STATE_ERROR:
                state = value;
                break;
        case STATE_OFF:
                state = value;
                off_timestamp = millis();
                break;
        default:
                break;
        }
}

void updateLEDs()
{
        switch (state) {
        case STATE_OFF:
                if (millis() - off_timestamp < 20000) {
                        digitalWrite(pinLedOnButton, LOW);
                        digitalWrite(pinLedOffButton, blink_count < 5? HIGH : LOW);
                } else {
                        digitalWrite(pinLedOnButton, LOW);
                        digitalWrite(pinLedOffButton, HIGH);
                }
                break;
        case STATE_STARTING_UP:
                digitalWrite(pinLedOnButton, blink_count < 5? HIGH : LOW);
                digitalWrite(pinLedOffButton, LOW);
                break;
        case STATE_ON:
                digitalWrite(pinLedOnButton, HIGH);
                digitalWrite(pinLedOffButton, LOW);
                break;
        case STATE_SHUTTING_DOWN: 
                digitalWrite(pinLedOnButton, LOW);
                digitalWrite(pinLedOffButton, blink_count < 5? HIGH : LOW);
                break;
        case STATE_ERROR:
                digitalWrite(pinLedOnButton, blink_count < 5? HIGH : LOW);
                digitalWrite(pinLedOffButton, blink_count < 5? HIGH : LOW);
                break;
        }
        
        blink_count++;
        if (blink_count == 10)
                blink_count = 0;
        
}

void updateRelay()
{
        switch (state) {
        case STATE_OFF:
                digitalWrite(pinRelay2, LOW);
                if (millis() - off_timestamp < 20000)
                        digitalWrite(pinRelay1, HIGH);
                else 
                        digitalWrite(pinRelay1, LOW);
                break;
        case STATE_ERROR:
                digitalWrite(pinRelay1, LOW);
                digitalWrite(pinRelay2, LOW);
                break;
        case STATE_STARTING_UP:
        case STATE_SHUTTING_DOWN: 
                digitalWrite(pinRelay1, HIGH);
                digitalWrite(pinRelay2, LOW);
                break;
        case STATE_ON:
                digitalWrite(pinRelay1, HIGH);
                digitalWrite(pinRelay2, HIGH);
                break;
        }
}

void handleSerialInput()
{
        while (Serial.available()) {
                char c = Serial.read();
                if (parser.process(c)) {                        
                        switch (parser.opcode()) {
                        default: break;
                        case 'S':
                                if (parser.length() == 1) {
                                        Serial.print("OK set state");
                                        setState(parser.value());
                                } else {
                                        Serial.print("ERR bad args");
                                }
                                break;
                        case 's':
                                Serial.print("s");
                                Serial.println(state);
                                break;
                        case 'D':
                                Serial.println(parser.text());
                                Serial.print("OK display");
                                break;
                        case '?':
                                Serial.print("?[\"RomiControlPanel\",\"0.1\"]"); 
                                break;
                        }
                }
        }
}

void loop()
{
        handleSerialInput();
        updateState();
        updateRelay();
        updateLEDs();
        delay(100);
}

