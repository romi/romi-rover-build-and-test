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
#ifndef __ANALOG_BUTTON_H
#define __ANALOG_BUTTON_H

#include "IArduino.h"
#include "IButton.h"
#include "Events.h"

#define MIN_TIME_PRESSED 200
#define MIN_TIME_HELD    3000
        
class AnalogButton : public IButton
{
protected:
        IArduino *_arduino;
        uint8_t _pin;
        uint8_t _id;
        uint8_t _state;
        uint16_t _min;
        uint16_t _max;
        unsigned long _timestamp;

public:
                        
        AnalogButton(IArduino *arduino, uint8_t pin, uint8_t id,
                     uint16_t min, uint16_t max)
                : _arduino(arduino), _pin(pin), _id(id), _state(Released),
                  _min(min), _max(max), _timestamp(0) {}
        
        virtual ~AnalogButton() {}

        virtual uint8_t id() {
                return _id;
        }

        virtual uint16_t getmin() {
                return _min;
        }

        virtual uint16_t getmax() {
                return _max;
        }

        virtual uint8_t state() {
                return _state;
        }

        virtual unsigned long timestamp() {
                return _timestamp;
        }
        
        virtual uint8_t update(unsigned long t) {
                uint8_t r = 0;
                uint16_t pinValue = _arduino->analogRead(_pin);
                bool pressed = (pinValue >= _min) && (pinValue <= _max);
                if (_state == Released) {
                        if (pressed) {
                                _state = Down;
                                _timestamp = t;
                        }
                
                } else if (_state == Down) {
                        if (!pressed) {
                                _state = Released;
                        } else if (t - _timestamp > MIN_TIME_HELD) {
                                _state = Held;
                                r = BUTTON_HELD;
                        } else if (t - _timestamp > MIN_TIME_PRESSED) {
                                _state = Pressed;
                                r = BUTTON_PRESSED;
                        }
                
                } else if (_state == Pressed) {
                        if (!pressed) {
                                _state = Released;
                        } else if (t - _timestamp > MIN_TIME_HELD) {
                                _state = Held;
                                r = BUTTON_HELD;
                        }
                } else if (_state == Held) {
                        if (!pressed) {
                                _state = Released;
                        } 
                }
                return r;
        }
};

#endif // __ANALOG_BUTTON_H
