/*
  Romi motor controller for brushed motors

  Copyright (C) 2021 Sony Computer Science Laboratories
  Author(s) Peter Hanappe

  Azhoo is free software: you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as
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
#ifndef _STEERING_INCREMENTAL_ENCODER_UNO_H
#define _STEERING_INCREMENTAL_ENCODER_UNO_H

#include <Arduino.h>
#include "IncrementalEncoder.h"

typedef void (*EncoderInterruptHandler)();

class IncrementalEncoderUno : public IncrementalEncoder
{
public:
        uint8_t pin_a_;
        uint8_t pin_b_;
        EncoderInterruptHandler callback_;
        
        IncrementalEncoderUno(uint8_t pin_a,
                              uint8_t pin_b,
                              EncoderInterruptHandler callback)
                : IncrementalEncoder(),
                  pin_a_(pin_a),
                  pin_b_(pin_b),
                  callback_(callback) {
        }
        
        ~IncrementalEncoderUno() override = default;

        void init(uint16_t pulses_per_revolution, int8_t increment);
        
        inline void update() {
                bool b = digitalRead(pin_b_);
                if (b) {
                        decrement();
                } else {
                        increment();
                }
        }
        
};

#endif // _STEERING_INCREMENTAL_ENCODER_UNO_H
