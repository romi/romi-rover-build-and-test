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

#include <Arduino.h>
#include "IInputStream.h"
#include "IOutputStream.h"

#ifndef __ARDUINO_SERIAL_H
#define __ARDUINO_SERIAL_H

class ArduinoSerial : public IInputStream, public IOutputStream
{
public:
        ArduinoSerial() {}
        
        virtual ~ArduinoSerial() {}

        void init(long baudrate) {
                Serial.begin(baudrate);
                while (!Serial)
                        ;
        }
        
        virtual int available() override {
                return Serial.available();
        }
        
        virtual int read() override {
                return Serial.read();
        }
        
        virtual void print(const char *s) override {
                Serial.print(s);
        }
        
        virtual void println(const char *s) override {
                Serial.println(s);
        }
};

#endif // __ARDUINO_SERIAL_H
