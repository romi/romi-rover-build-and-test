/*
  Romi motor controller for brushed motors

  Copyright (C) 2021 Sony Computer Science Laboratories
  Author(s) Peter Hanappe

  MotorController is free software: you can redistribute it and/or modify it
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
#include "IncrementalEncoderUno.h"
        
void IncrementalEncoderUno::init(uint16_t pulses_per_revolution,
                                 int8_t increment, 
                                 uint8_t pin_a,
                                 uint8_t pin_b,
                                 EncoderInterruptHandler callback)
{
        IncrementalEncoder::init(pulses_per_revolution, increment);
        pin_b_ = pin_b;
        pinMode(pin_a, INPUT_PULLUP);
        pinMode(pin_b, INPUT_PULLUP);
        attachInterrupt(digitalPinToInterrupt(pin_a),
                        callback,
                        RISING);
}
