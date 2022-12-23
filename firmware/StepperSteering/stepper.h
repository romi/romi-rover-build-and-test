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
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include "config.h"
#include "IArduino.h"

#ifndef _STEERING_STEPPER_H_
#define _STEERING_STEPPER_H_

enum ControlMode {
        kOpenLoopControl,
        kClosedLoopControl
};

/**
 * \brief: Configure the stepper. Must be called with interrupts
 * disabled (see cli()).
 *
 */
void init_stepper(IArduino *arduino);

void set_encoder_target(int16_t left, int16_t right);
void get_encoder_target(int16_t& left, int16_t& right);
void set_stepper_target(int32_t left, int32_t right);
void get_stepper_target(int32_t& left, int32_t& right);

int32_t get_stepper_position(int8_t axis);
int32_t get_stepper_left();
int32_t get_stepper_right();

// stepper_zero_axis sets the stepper position to zero.
void stepper_zero_axis(int8_t axis);

// stepper_reset*() sets the stepper position to zero AND sets the
// target angles to zero.
void stepper_reset();
void stepper_reset_axis(int8_t axis);

/**
 * \brief Disable/enable the stepper driver.
 */
#if ENABLE_PIN_HIGH
#define enable_driver()  set_enable_pin_high()
#define disable_driver() set_enable_pin_low()
#else
#define enable_driver()  set_enable_pin_low()
#define disable_driver() set_enable_pin_high()
#endif

//extern volatile uint8_t thread_state;

#define enable_reset_step_pins_timer()                                  \
        {                                                               \
                /* Initialize counter */                                \
                TCNT2 = 0;                                              \
                /* Enable Timer2 */                                     \
                TIMSK2 |= (1 << OCIE2A);                                \
        }

#define disable_reset_step_pins_timer()                                 \
        {                                                               \
                /* Disable Timer2 interrupt */                          \
                TIMSK2 &= ~(1 << OCIE2A);                               \
        }

#define enable_stepper_timer()                                          \
        {                                                               \
                /* Initialize counter */                                \
                TCNT1 = 0;                                              \
                /* Set the status of the stepper thread */              \
                /* thread_state = STATE_THREAD_EXECUTING; */            \
                /* Enable Timer1 */                                     \
                TIMSK1 |= (1 << OCIE1A);                                \
        }

#define disable_stepper_timer()                                         \
        {                                                               \
                /* Set the status of the stepper thread */              \
                /*thread_state = STATE_THREAD_IDLE;*/                   \
                /* Disable Timer1 interrupt */                          \
                TIMSK1 &= ~(1 << OCIE1A);                               \
        }

#endif // _STEERING_STEPPER_H_
