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
#include <avr/interrupt.h>
#include "Arduino.h"
#include "config.h"
#include "encoder.h"

volatile int32_t encoder_position[3];

void init_encoders()
{
        for (int i = 0; i < 3; i++)
                encoder_position[i] = 0;

        if (X_ENCODER_A > 0) {
                pinMode(X_ENCODER_A, INPUT_PULLUP);
                pinMode(X_ENCODER_B, INPUT_PULLUP);
                attachInterrupt(digitalPinToInterrupt(X_ENCODER_A),
                                handle_x_encoder_interrupt,
                                RISING);
        }
        
        if (Y_ENCODER_A > 0) {
                pinMode(Y_ENCODER_A, INPUT_PULLUP);
                pinMode(Y_ENCODER_B, INPUT_PULLUP);
                attachInterrupt(digitalPinToInterrupt(Y_ENCODER_A),
                                handle_y_encoder_interrupt,
                                RISING);
        }
}

/**
 * Interrupt service routines for the quadrature encoder on the
 * X-axis.
 */
void handle_x_encoder_interrupt()
{
        uint8_t b = get_x_encoder_b();
#if ENCODER_REVERSED
        encoder_position[0] -= b ? -1 : +1;
#else
        encoder_position[0] += b ? -1 : +1;
#endif
}
 
/**
 * Interrupt service routines for the quadrature encoder on the
 * Y-axis.
 */
void handle_y_encoder_interrupt()
{
        uint8_t b = get_y_encoder_b(); 
#if ENCODER_REVERSED
        encoder_position[1] -= b ? -1 : +1;
#else
        encoder_position[1] += b ? -1 : +1;
#endif
}
