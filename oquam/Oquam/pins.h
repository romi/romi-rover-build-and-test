#include <avr/io.h>
#include <avr/interrupt.h>
#include "config.h"

#ifndef _OQUAM_PINS_H_
#define _OQUAM_PINS_H_

/* 
 * The STEP_ and DIRECTION_ defines below are taken from Grbl but the
 * pins have been changed to free up pins 2 and 3 for the encoders.
 * On the Uno, the code can attach an interrupt handler only on pins 2
 * or 3.
 */

/** 
 * Define step pulse output pins. NOTE: All step bit pins must be on
 * the same port. 
 */
#define STEP_DDR        DDRB
#define STEP_PORT       PORTB
#define X_STEP_BIT      0  // Uno Digital Pin 8
#define Y_STEP_BIT      1  // Uno Digital Pin 9
#define Z_STEP_BIT      2  // Uno Digital Pin 10
#define STEP_MASK       ((1 << X_STEP_BIT) | (1 << Y_STEP_BIT) | (1 << Z_STEP_BIT))

/** 
 * Define step direction output pins. NOTE: All direction pins must be
 * on the same port.
 */
#define DIRECTION_DDR   DDRB
#define DIRECTION_PORT  PORTB
#define X_DIRECTION_BIT 3  // Uno Digital Pin 11
#define Y_DIRECTION_BIT 4  // Uno Digital Pin 12
#define Z_DIRECTION_BIT 5  // Uno Digital Pin 13
#define DIRECTION_MASK  ((1<<X_DIRECTION_BIT)|(1<<Y_DIRECTION_BIT)|(1<<Z_DIRECTION_BIT))

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
#define Y_ENCODER_A         3
#define Y_ENCODER_B         5
#define Y_ENCODER_B_MASK    (1 << Y_ENCODER_B)
#define Y_ENCODER_B_PORT    PIND
#define Y_ENCODER_INTERRUPT INT.1

/**
 * \brief Configure the step and dir pins as output.
 */
void init_output_pins();

/**
 * \brief Raise the enable pin.
 */
#define enable_pin_high()                                               \
        {                                                               \
                STEPPERS_DISABLE_PORT |= (1 << STEPPERS_DISABLE_BIT);   \
        }

/**
 * \brief Reset the enable pin.
 */
#define enable_pin_low()                                                \
        {                                                               \
                STEPPERS_DISABLE_PORT &= ~(1 << STEPPERS_DISABLE_BIT);  \
        }

/**
 * \brief Disable/enable the stepper driver.
 */
#if ENABLE_PIN_HIGH
#define enable_driver()  enable_pin_high()
#define disable_driver() enable_pin_low()
#else
#define enable_driver()  enable_pin_low()
#define disable_driver() enable_pin_high()
#endif


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
#define set_dir_pins(__mask)                           \
        {                                               \
                DIRECTION_PORT |= __mask;                    \
        }

/**
 * \brief Toggle a direction bit in the DIR mask.
 */
#define toggle_dir(__mask, __axis)                     \
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

#endif // _OQUAM_PINS_H_
