/*
  Romi motor controller for brushed motors

  Copyright (C) 2021 Sony Computer Science Laboratories
  Author(s) Peter Hanappe

  MotorControlle is free software: you can redistribute it and/or modify it
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
#include <Arduino.h>
#include "ArduinoUno.h"

static IncrementalEncoderUno left_encoder_;
static IncrementalEncoderUno right_encoder_;

ArduinoUno::ArduinoUno()
        : left_pwm_(),
          right_pwm_()
{
}

// Interrupt callback for the left and right encoders
static void update_left_encoder()
{
        left_encoder_.update();
}

static void update_right_encoder()
{
        right_encoder_.update();
}

void ArduinoUno::setup()
{
        init_pwm();
}

void ArduinoUno::init_encoders(uint16_t encoder_steps,
                               int8_t left_increment,
                               int8_t right_increment)
{
        left_encoder_.init(encoder_steps,
                           left_increment, 
                           kLeftEncoderPinA,
                           kLeftEncoderPinB,
                           update_left_encoder);

        right_encoder_.init(encoder_steps,
                            right_increment, 
                            kRightEncoderPinA,
                            kRightEncoderPinB,
                            update_right_encoder);
        
}

void ArduinoUno::init_pwm()
{
        /*
          It can't be repeated enough:
          TCCR = Timer/Counter Control Register
          OCR = Output Compare Register 
          ICR = Input Capture Register 
          DDRB = Data Direction Register for port B 
         */
        
        // From https://wolles-elektronikkiste.de/en/timer-and-pwm-part-2-16-bit-timer1
        // Clear OC1A and OC1B on Compare Match / Set OC1A and OC1B at Bottom; 
        // Wave Form Generator: Fast PWM 14, Top = ICR1

        TCCR1A = (1 << COM1A1) + (1 << COM1B1) + (1 << WGM11); 
        TCCR1B = (1 << WGM13) + (1 << WGM12) + (1 << CS11); // prescaler = 8

        // F_CPU = 16000000
        // frequency = 50 Hz -> period = 20 ms
        // prescaler = 8
        // top = F_CPU / (prescaler * frequency) - 1 = 39999

        // 1 tick = 1 period / 40000 = 20 ms / 40000 s = 1/2 µs

        ICR1 = 39999;
        OCR1A = 3000; // 3000 ticks = 3000 * 1/2 µs = 1500 µs = 1.5 ms
        OCR1B = 3000;

        // Output directly to pins 9 and 10
        DDRB |= (1 << PB1) | (1 << PB2);

        left_pwm_.init(&OCR1A);
        right_pwm_.init(&OCR1B);
}

IEncoder& ArduinoUno::left_encoder()
{
        return left_encoder_;
}

IEncoder& ArduinoUno::right_encoder()
{
        return right_encoder_;
}

