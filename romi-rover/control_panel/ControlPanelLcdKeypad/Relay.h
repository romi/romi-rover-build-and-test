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

#ifndef __RELAY_H
#define __RELAY_H

#include "IArduino.h" 
#include "IRelay.h" 
#include "ArduinoConstants.h" 

class Relay : public IRelay
{
protected:
        IArduino *_arduino;
        uint8_t _pin;
        
public:
        Relay(IArduino *arduino, uint8_t pin) : _arduino(arduino), _pin(pin) {
                _arduino->pinMode(_pin, OUTPUT);
                open();
        }
        
        virtual ~Relay() {
                open();
        }

        // "open" means that the "Normally Open" pin (NO) on the relay
        // will be connected after this operation.
        virtual void open() override {
                _arduino->digitalWrite(_pin, LOW);
        }
        
        // "close" means that the "Normally Closed" pin (NC) on the
        // relay will be connected after this operation.
        virtual void close() override {
                _arduino->digitalWrite(_pin, HIGH);
        }
};

#endif // __RELAY_H
