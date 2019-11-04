#include <avr/io.h>
#include <avr/interrupt.h>
#include "action.h"

#ifndef _OQUAM_STEPPER_H_
#define _OQUAM_STEPPER_H_

/**
 *  \brief The possible states of the stepper thread.
 */
enum {
        STATE_THREAD_IDLE,
        STATE_THREAD_STARTING,
        STATE_THREAD_EXECUTING
};

/**
 * \brief: Configure Timer1 to drive the stepper's STEP pulse train.
 *
 *  Timer1 is used as the "stepper timer", i.e. this timer will pulse
 *  the STEP pins of the stepper drivers.
 */
void init_stepper_timer();

/**
 * \brief: Initialize Timer2 to reset the STEP pins back to zero 10 µs
 * after a pulse.
 *
 *  Timer2 is used as the "reset step timer", i.e. this timer will
 *  pulse the set the STEP pins of the stepper drivers to zero 10 µs
 *  after they have been raised to 1 by the stepper timer.
 */
void init_reset_step_pins_timer();


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
                thread_state = STATE_THREAD_EXECUTING;                  \
                /* Enable Timer1 */                                     \
                TIMSK1 |= (1 << OCIE1A);                                \
        }

#define disable_stepper_timer()                                         \
        {                                                               \
                /* Disable Timer1 interrupt */                          \
                TIMSK1 &= ~(1 << OCIE1A);                               \
        }

#endif // _OQUAM_STEPPER_H_
