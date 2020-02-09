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

#ifndef _OQUAM_PINS_EXTCTRLR_UNO_H__
#define _OQUAM_PINS_EXTCTRLR_UNO_H_

#if USE_EXT_CONTROLLER

#define USE_EXTCTRL_PINS   1
#define USE_UNO            1
#define USE_ENCODERS       1
#define USE_LIMITS         1
#define ENABLE_PIN_HIGH    1
#define ENCODER_REVERSED   0
#define PRESCALING         1
#define FREQUENCY_STEPPER  25000
#define INTERRUPTS_PER_MILLISECOND 25

/* 
 * The pins have been changed to free up pins 2 and 3 for the
 * encoders. On the Uno, the code can attach an interrupt handler only
 * on pins 2 or 3. The interrupts are needed to read the encoder
 * signal.
 */

/** 
 * Define step pulse output pins. NOTE: All step bit pins must be on
 * the same port. 
 */
#define STEP_DDR          DDRB
#define STEP_PORT         PORTB
#define X_STEP_BIT        0  // Uno Digital Pin 8
#define Y_STEP_BIT        1  // Uno Digital Pin 9
#define Z_STEP_BIT        2  // Uno Digital Pin 10
#define STEP_MASK         ((1 << X_STEP_BIT) | (1 << Y_STEP_BIT) | (1 << Z_STEP_BIT))

/** 
 * Define step direction output pins. NOTE: All direction pins must be
 * on the same port.
 */
#define DIRECTION_DDR     DDRB
#define DIRECTION_PORT    PORTB
#define X_DIRECTION_BIT   3  // Uno Digital Pin 11
#define Y_DIRECTION_BIT   4  // Uno Digital Pin 12
#define Z_DIRECTION_BIT   5  // Uno Digital Pin 13
#define DIRECTION_MASK    ((1<<X_DIRECTION_BIT)|(1<<Y_DIRECTION_BIT)|(1<<Z_DIRECTION_BIT))

/**
 * Define stepper driver enable/disable output pin.
 */
#define STEPPERS_DISABLE_DDR    DDRD
#define STEPPERS_DISABLE_PORT   PORTD
#define STEPPERS_DISABLE_BIT    7  // Uno Digital Pin 7
#define STEPPERS_DISABLE_MASK   (1 << STEPPERS_DISABLE_BIT)

/**
 * Define the pins for the X and Y encoders.
 */
#define X_ENCODER_A         2
#define X_ENCODER_B         4
#define X_ENCODER_B_MASK    (1 << X_ENCODER_B)
#define X_ENCODER_B_PORT    PIND
#define X_ENCODER_INTERRUPT INT.0
#define Y_ENCODER_A         3  // Uno Digital Pin 3
#define Y_ENCODER_B         5  // Uno Digital Pin 5
#define Y_ENCODER_B_MASK    (1 << Y_ENCODER_B)
#define Y_ENCODER_B_PORT    PIND
#define Y_ENCODER_INTERRUPT INT.1

/**
 * Define the pins for the limit switches.
 */
#define LIMIT_DDR         DDRD
#define LIMIT_PIN         PIND
#define LIMIT_PORT        PORTD
#define X_LIMIT_BIT       6  // Uno Digital Pin 6
#define Y_LIMIT_BIT       7  // Uno Digital Pin 7
//#define Z_LIMIT_BIT	  ???
#define LIMIT_MASK        ((1 << X_LIMIT_BIT) | (1 << Y_LIMIT_BIT))

/**
 * \brief Configure the step and dir pins as output.
 */
void init_output_pins();

/**
 * \brief Raise the enable pin.
 */
#define set_enable_pin_high()                                           \
        {                                                               \
                STEPPERS_DISABLE_PORT |= (1 << STEPPERS_DISABLE_BIT);   \
        }

/**
 * \brief Reset the enable pin.
 */
#define set_enable_pin_low()                                            \
        {                                                               \
                STEPPERS_DISABLE_PORT &= ~(1 << STEPPERS_DISABLE_BIT);  \
        }

/**
 * \brief Enable the step pins according to mask.
 */
#define set_step_pins(__mask)                           \
        {                                               \
                STEP_PORT |= __mask;                    \
        }

/**
 * \brief Reset the step pins to zero.
 */
#define reset_step_pins()                               \
        {                                               \
                STEP_PORT &= ~STEP_MASK;                \
        }

/**
 * \brief Toggle a step bit in the pins mask.
 */
#define toggle_step(__mask, __axis)                     \
        {                                               \
                __mask |= (1 << __axis);                \
        }

/**
 * \brief Enable the DIR pins according to mask.
 */
#define set_dir_pins(__mask)                            \
        {                                               \
                DIRECTION_PORT |= __mask;               \
        }

/**
 * \brief Toggle a direction bit in the DIR mask.
 */
#define toggle_dir(__mask, __axis)                      \
        {                                               \
                __mask |= (1 << __axis);                \
        }

/**
 * \brief Read the X-encoder B pin.
 */
#define get_x_encoder_b()  (X_ENCODER_B_PORT & X_ENCODER_B_MASK)

/**
 * \brief Read the Y-encoder B pin.
 */
#define get_y_encoder_b()  (Y_ENCODER_B_PORT & Y_ENCODER_B_MASK)

/**
 * \brief Read the limit switches.
 */
#define get_limit_bits()  0

#endif // USE_EXT_CONTROLLER
#endif // _OQUAM_PINS_EXTCTRLR_UNO_H_
