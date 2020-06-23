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

#include <LiquidCrystal.h>
#include "Parser.h"

#define PIN_RELAY1 8
#define PIN_RELAY2 9
#define PIN_LED_ONBUTTON 10
#define PIN_LED_OFFBUTTON 11
#define PIN_ONBUTTON 12
#define PIN_OFFBUTTON 13

//#define EN
#define FR 1

#if EN
const char *strings[] = {
        "Error", "Starting up", "Powering up", "Ready", "Shutting down", "Off" 
};
#endif

#if FR
const char *strings[] = {
        "Erreur", "Demarrage", "Activation", "Pret", "Arret", "Eteint" 
};
#endif

enum {
        STATE_ERROR = 0,
        STATE_STARTING_UP,
        STATE_POWERING_UP,
        STATE_ON,
        STATE_SHUTTING_DOWN,
        STATE_OFF,
};

enum {
        BUTTON_UP = 0,
        BUTTON_DOWN = 1
};

enum {
        ONBUTTON = 0,
        OFFBUTTON = 1
};

Parser parser("SD", "s?");

int state = STATE_OFF;
unsigned int blink_count = 0;
unsigned long shutting_down_start;
char button_state[2];
unsigned long button_timestamp[2];

const char rs = 7, en = 6, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

void setup()
{
        Serial.begin(115200);
        while (!Serial)
                ;

        pinMode(PIN_ONBUTTON, INPUT_PULLUP);
        pinMode(PIN_OFFBUTTON, INPUT_PULLUP);
        pinMode(PIN_LED_ONBUTTON, OUTPUT);
        pinMode(PIN_LED_OFFBUTTON, OUTPUT);
        pinMode(PIN_RELAY1, OUTPUT);
        pinMode(PIN_RELAY2, OUTPUT);

        digitalWrite(PIN_RELAY1, LOW);
        digitalWrite(PIN_RELAY2, LOW);

        lcd.begin(16, 2);
        display_state();
}

int update_buttons()
{
        int pressed;
        unsigned long t = millis();
        
        pressed = (digitalRead(PIN_ONBUTTON) == LOW);
        if (pressed && button_state[ONBUTTON] == BUTTON_UP) {
                button_state[ONBUTTON] = BUTTON_DOWN;
                button_timestamp[ONBUTTON] = t;
        } 
        if (!pressed && button_state[ONBUTTON] == BUTTON_DOWN) {
                button_state[ONBUTTON] = BUTTON_UP;
                button_timestamp[ONBUTTON] = t;
        }
        
        pressed = (digitalRead(PIN_OFFBUTTON) == LOW);
        if (pressed && button_state[OFFBUTTON] == BUTTON_UP) {
                button_state[OFFBUTTON] = BUTTON_DOWN;
                button_timestamp[OFFBUTTON] = t;
        }
        if (!pressed && button_state[OFFBUTTON] == BUTTON_DOWN) {
                button_state[OFFBUTTON] = BUTTON_UP;
                button_state[OFFBUTTON] = t;
        }
}

#define button_pressed(__id)  (button_state[__id] == BUTTON_DOWN)
#define time_pressed(__id) (millis() - button_timestamp[__id])

void display_state()
{
        display0(strings[state]);
}

void update_state()
{
        
        if (button_pressed(ONBUTTON) && button_pressed(OFFBUTTON)) {
                if (time_pressed(ONBUTTON) > 5000
                    && time_pressed(OFFBUTTON) > 5000)
                        state = STATE_OFF;
                
        } else if (button_pressed(ONBUTTON)) {
                switch (state) {
                case STATE_OFF:
                        state = STATE_STARTING_UP;
                        display_state();
                        break;
                case STATE_STARTING_UP:
                case STATE_POWERING_UP:
                case STATE_ON:
                case STATE_SHUTTING_DOWN: 
                case STATE_ERROR:
                        break;
                }
                
        } else if (button_pressed(OFFBUTTON)) {
                switch (state) {
                case STATE_ON:
                        state = STATE_SHUTTING_DOWN;
                        shutting_down_start = millis();
                        display_state();
                        break;
                case STATE_OFF:
                case STATE_STARTING_UP:
                case STATE_POWERING_UP:
                case STATE_SHUTTING_DOWN: 
                case STATE_ERROR:
                        break;
                }
        }
        if (state == STATE_SHUTTING_DOWN
            && millis() - shutting_down_start > 60000) { 
                state = STATE_OFF;
        }
        
}

void set_state(int value)
{
        switch (value) {
        case STATE_OFF:
        case STATE_STARTING_UP:
        case STATE_SHUTTING_DOWN: 
                break;
        case STATE_POWERING_UP:
        case STATE_ON:
        case STATE_ERROR:
                state = value;
                display_state();
                break;
        default:
                break;
        }
}

void update_leds()
{
        unsigned int n8 = blink_count & 0x07;
        unsigned int n4 = blink_count & 0x03;
        
        switch (state) {
        case STATE_OFF:
                digitalWrite(PIN_LED_ONBUTTON, LOW);
                digitalWrite(PIN_LED_OFFBUTTON, HIGH);
                break;
        case STATE_STARTING_UP:
                digitalWrite(PIN_LED_ONBUTTON, n8 < 4? HIGH : LOW);
                digitalWrite(PIN_LED_OFFBUTTON, LOW);
                break;
        case STATE_POWERING_UP:
                digitalWrite(PIN_LED_ONBUTTON, n4 < 2? HIGH : LOW);
                digitalWrite(PIN_LED_OFFBUTTON, LOW);
                break;
        case STATE_ON:
                digitalWrite(PIN_LED_ONBUTTON, HIGH);
                digitalWrite(PIN_LED_OFFBUTTON, LOW);
                break;
        case STATE_SHUTTING_DOWN: 
                digitalWrite(PIN_LED_ONBUTTON, LOW);
                digitalWrite(PIN_LED_OFFBUTTON, n8 < 4? HIGH : LOW);
                break;
        case STATE_ERROR:
                digitalWrite(PIN_LED_ONBUTTON, n8 < 4? HIGH : LOW);
                digitalWrite(PIN_LED_OFFBUTTON, n8 < 4? HIGH : LOW);
                break;
        }
        
        blink_count++;
}

void update_relays()
{
        switch (state) {
        case STATE_OFF:
                digitalWrite(PIN_RELAY2, LOW);
                digitalWrite(PIN_RELAY1, LOW);
                break;
        case STATE_ERROR:
                digitalWrite(PIN_RELAY1, LOW);
                digitalWrite(PIN_RELAY2, LOW);
                break;
        case STATE_STARTING_UP:
        case STATE_SHUTTING_DOWN: 
                digitalWrite(PIN_RELAY1, HIGH);
                digitalWrite(PIN_RELAY2, LOW);
                break;
        case STATE_POWERING_UP:
        case STATE_ON:
                digitalWrite(PIN_RELAY1, HIGH);
                digitalWrite(PIN_RELAY2, HIGH);
                break;
        }
}

void display0(const char *text)
{
        lcd.setCursor(0, 0);
        lcd.print("                ");
        lcd.setCursor(0, 0);
        lcd.print(text);
}

void display1(const char *text)
{
        lcd.setCursor(0, 1);
        lcd.print("                ");
        lcd.setCursor(0, 1);
        lcd.print(text);
}

void handle_serial_input()
{
        while (Serial.available()) {
                char c = Serial.read();
                if (parser.process(c)) {                        
                        switch (parser.opcode()) {
                        default: break;
                        case 'S':
                                if (parser.length() == 1) {
                                        Serial.println("OK set state");
                                        set_state(parser.value());
                                } else {
                                        Serial.println("ERR bad args");
                                }
                                break;
                        case 's':
                                Serial.print("s");
                                Serial.println(state);
                                break;
                        case 'D':
                                display1(parser.text());
                                Serial.println("OK display");
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
        handle_serial_input();
        update_buttons();
        update_state();
        update_relays();
        update_leds();
        delay(100);
}

