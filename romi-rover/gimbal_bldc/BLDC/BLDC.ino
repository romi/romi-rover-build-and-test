/*
  bldc_featherwing

  Copyright (C) 2019-2020 Sony Computer Science Laboratories
  Author(s) Peter Hanappe

  bldc_featherwing is Arduino firmware to control a brushless motor.

  bldc_featherwing is free software: you can redistribute it and/or
  modify it under the terms of the GNU General Public License as
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
#include "ArduinoImpl.h"
#include "PwmEncoder.h"
#include "BLDC.h"
#include "Parser.h"
#include "PwmOut.h"
#include "PwmGenerator.h"
#include "DigitalOut.h"
#include "pins.h"

ArduinoImpl arduino;
PwmEncoder encoder(&arduino, P_ENC, 11, 959);
Parser parser("XP", "?sC");

PwmOut pwm1(&arduino, P_IN1);
PwmOut pwm2(&arduino, P_IN2);
PwmOut pwm3(&arduino, P_IN3);
DigitalOut enable1(&arduino, P_EN1);
DigitalOut enable2(&arduino, P_EN2);
DigitalOut enable3(&arduino, P_EN3);

PwmGenerator pwmGenerator(&pwm1, &pwm2, &pwm3,
                          &enable1, &enable2, &enable3);

DigitalOut sleepPin(&arduino, P_SLEEP);
DigitalOut resetPin(&arduino, P_RESET);


BLDC motor(&encoder, &pwmGenerator, &sleepPin, &resetPin);

unsigned long prev_time = 0;

void setup()
{
        Serial.begin(115200);
        while (!Serial)
                ;
        motor.wake();
        motor.setPower(0.5f);
        Serial.println("Ready");
}

void handleSerialInput()
{
        if (Serial.available()) {
                char c = Serial.read();
                if (parser.process(c)) {
                        switch (parser.opcode()) {
                        case 'X':
                                if (parser.length() == 1) {
                                        float value = (float) parser.value() / 360.0f;
                                        motor.setTargetPosition(value);
                                        Serial.println("OK");
                                } else {
                                        Serial.println("ERR bad args");
                                }
                                break;
                        case 's':
                                if (1) {
                                        Serial.print("s[");
                                        Serial.print(360.0f * encoder.getAngle());
                                        Serial.println("]");
                                }
                                break;
                        case 'P':
                                if (parser.length() == 1) {
                                        float value = (float) parser.value() / 100.0f;
                                        motor.setPower(value);
                                        Serial.println("OK");
                                } else {
                                        Serial.println("ERR bad args");
                                }
                                break;
                        case 'C':
                                motor.calibrate();
                                break;
                        case '?':
                                Serial.println("?['BLDCController','0.1']");
                                break;
                        }
                }
        }
}

void loop()
{
        static unsigned long lastTime = 0;
        unsigned long t = arduino.micros();
        motor.update((float)(t - lastTime) / 1000000.0f);
        handleSerialInput();
        lastTime = t;
        delay(2);
}
