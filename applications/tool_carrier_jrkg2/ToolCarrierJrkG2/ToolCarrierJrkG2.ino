/*
  Romi motor controller for brushed mortors

  Copyright (C) 2018-2019 Sony Computer Science Laboratories
  Author(s) Peter Hanappe

  LettuceThink Motor Controller is a firmware for Arduino. It reads
  two RC signals for speed and direction, two encoders for two wheel,
  and generates two PWM motor signals for a motor driver.

  The LettuceThink Motor Controller is free software: you can
  redistribute it and/or modify it under the terms of the GNU Lesser
  General Public License as published by the Free Software Foundation,
  either version 3 of the License, or (at your option) any later
  version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see
  <http://www.gnu.org/licenses/>.

 */
#include <SoftwareSerial.h>
#include <JrkG2.h>
#include "Parser.h"

SoftwareSerial soft_serial(10, 11);
JrkG2Serial jrk(soft_serial);
Parser parser("T", "?");

const char *msgOK = "OK";
const char *errMissingValues = "ERR missing values";

void setup()
{
        Serial.begin(115200);
        while (!Serial)
                ;
        soft_serial.begin(9600);
        jrk.setTarget(2048);
}

void transmitVersion()
{
        Serial.println("?[\"ToolCarrierJrkG2\",\"0.1\"]"); 
}

void setTarget(int value)
{
        int target = map(value, -1000, 1000, 1448, 2648) ;
        jrk.setTarget(target);
}

void handleSerialInput()
{
        while (Serial.available()) {
                char c = Serial.read();
                if (parser.process(c)) {                        
                        switch (parser.opcode()) {
                        default: break;
                        case '?':
                                transmitVersion();
                                break;
                        case 'T':
                                if (parser.length() == 1) {
                                        setTarget(parser.value());
                                        Serial.println(msgOK);
                                } else {
                                        Serial.println(errMissingValues);
                                }
                                break;
                        }
                }
        }
}

void loop()
{
        handleSerialInput();
}

