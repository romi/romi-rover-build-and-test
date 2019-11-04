// CPU frequency Uno: 16000000
// Timer 1: Stepper timer
// Timer 2: Reset step pins

#include "trigger.h"
#include "stepper.h"
#include "pins.h"

volatile int16_t stepper_positions[3];
volatile uint8_t thread_state;
volatile int32_t counter_stepper_timer = 0;
volatile int32_t counter_reset_timer = 0;
volatile uint16_t current_action_id = -1;
action_t *current_action;
int16_t accumulation_error[3];
int16_t step_dir[3];

/*
 
  https://fr.wikiversity.org/wiki/Micro_contr%C3%B4leurs_AVR/Le_Timer_1
  http://maxembedded.com/2011/07/avr-timers-ctc-mode/
  

 */
/**
 * \brief: Initialize Timer2 to reset the STEP pins back to zero 10 µs
 * after a pulse.
 *
 *  Timer2 is used as the "reset step timer", i.e. this timer will
 *  pulse the set the STEP pins of the stepper drivers to zero 10 µs
 *  after they have been raised to 1 by the stepper timer.
 */
void init_reset_step_pins_timer()
{
        /* Don't enable the timer, yet. */
        TIMSK2 &= ~(1 << OCIE2A);

        // Use the waveform generation mode, or Clear Timer on Compare
        // (CTC) mode:
        //
        // Register  WGM22 WGM21 WGM20
        // TCCR2A          1     0
        // TCCR2B    0                 
        TCCR2A &= ~(1 << WGM20);
        TCCR2A |=  (1 << WGM21);
        TCCR2B &= ~(1 << WGM22); 

        // Disconnect OC1 output: Don't send the PWM to an output
        // pin.
        TCCR2A &= ~((1 << COM2A1) | (1 << COM2A0)
                    | (1 << COM2B1) | (1 << COM2B0));

        // Set the prescaling
        //   CS22  CS21  CS20
        //   0     0     0     Disabled
        //   0     0     1     1
        //   0     1     0     8
        //   0     1     1     32
        //   1     0     0     64
        //   1     0     1     128
        //   1     1     0     256
        //   1     1     1     1024

        // Prescaling: 8
        TCCR2B &= ~(1 << CS20);
        TCCR2B |=  (1 << CS21);
        TCCR2B &= ~(1 << CS22);
        // TCCR2B &= ~(1 << CS20);
        // TCCR2B &= ~(1 << CS21);
        // TCCR2B &= ~(1 << CS22);

        /* Set the compare value:

           Timer delay = T = 10 µs
           F_CPU = 16 MHz
           Prescaling = 8
           F_CLOCK = 2 MHz
           P_CLOCK = 1/2 µs
           N = 10 µs / (1/2 µs) = 20
           N-1 = 19
           
           int n = T / (1 / F_CPU / prescaling) - 1
                 = (T * F_CPU / prescaling) - 1
                 = 19
        */
        OCR2A = 19;
        
        /* Initialize counter */
        TCNT2 = 0;
}


/*

1. Grbl uses second timer to turn off pins
2. Set CS21 bit for 8 prescaler...??? 32? http://ww1.microchip.com/downloads/en/AppNotes/Atmel-2505-Setup-and-Use-of-AVR-Timers_ApplicationNote_AVR130.pdf, page 8

void Tim()
{
    //set timer2 interrupt at 8kHz for acceleration
    TCCR2A = 0;// set entire TCCR2A register to 0
    // turn on CTC mode
    TCCR2A |= (1 << WGM21);
    
    TCCR2B = 0;// same for TCCR2B
    // Set CS21 bit for 8 prescaler
    TCCR2B |= (1 << CS21);   
    TCCR2B |= (1 << CS20);   

    TCNT2  = 0;//initialize counter value to 0
    // set compare match register for 8khz increments
    OCR2A = 249;// = (16*10^6) / (8000*8) - 1 (must be <256)
    // enable timer compare interrupt
    TIMSK2 |= (1 << OCIE2A);
}

*/


/**
 * \brief: Configure Timer1 to drive the stepper's STEP pulse train.
 *
 *  Timer1 is used as the "stepper timer", i.e. this timer will pulse
 *  the STEP pins of the stepper drivers.
 */
void init_stepper_timer()
{
        current_action = 0;

        /* Don't enable the timer, yet */
        TIMSK1 &= ~(1 << OCIE1A);
        
        // Use the waveform generation mode, or Clear Timer on Compare
        // (CTC) mode.
        //
        // Register  WGM13 WGM12 WGM11 WGM10  
        // TCCR1A                0     0    
        // TCCR1B    0     1                
        TCCR1A &= ~(1 << WGM10);
        TCCR1A &= ~(1 << WGM11);
        TCCR1B |=  (1 << WGM12);
        TCCR1B &= ~(1 << WGM13); 

        // Disconnect OC1 output: Don't send the PWM to an output
        // pin.
        TCCR1A &= ~((1 << COM1A1) | (1 << COM1A0)
                    | (1 << COM1B1) | (1 << COM1B0));
        
        // Set the prescaling
        //   CS12  CS11  CS10
        //   0     0     0     Disabled
        //   0     0     1     1
        //   0     1     0     8
        //   0     1     1     64
        //   1     0     0     256
        //   1     0     1     1025
        //   1     1     0     Use external clock, falling edge
        //   1     1     1     Use external clock, rising edge
        // prescaling: 1
        TCCR1B |=  (1 << CS10);
        TCCR1B &= ~(1 << CS11);
        TCCR1B &= ~(1 << CS12);

        /* Set the compare value: 
           
           F_STEPPER = 25000 (25 kHz)
           P_STEPPER = 1/25000 = 40 µs
           F_CPU = 16 MHz
           Prescaling = 1
           F_CLOCK = 16 MHz
           P_CLOCK = 1/16 µs
           N = 40 µs / (1/16 µs) = 640
           N-1 = 639

           int n = (1 / F_STEPPER) / (1 / F_CPU / prescaling) - 1
                 = F_CPU / (F_STEPPER * prescaling) - 1
                 = 639
         */
        OCR1A = 639;
}

/**
 * \brief The interrupt service handler for the reset-step-pins timer
 */
ISR(TIMER2_COMPA_vect)
{
        reset_step_pins();
        disable_reset_step_pins_timer();
        counter_reset_timer++;
}

/**
 * \brief The interrupt service routine for the stepper timer.
 *
 */
ISR(TIMER1_COMPA_vect)
{
        /* If there is no action active then pop the next action from
         * the buffer.  */
        if (current_action == 0) {
                
                current_action = action_buffer_get_next();

                /* If there are no more actions, go into idle mode. */
                if (current_action == 0) {
                        current_action_id = -1;
                        disable_stepper_timer();
                        thread_state = STATE_THREAD_IDLE;
                        return;
                }

                /* Do the necessary initializations for the new
                 * action. */
                
                counter_stepper_timer = 0;
                current_action_id = current_action->id;
                
                if (current_action->type == ACTION_MOVE) {
                        /* Check the direction and set DIR pins. */
                        uint8_t dir = 0;
                        step_dir[0] = 1;
                        step_dir[1] = 1;
                        step_dir[2] = 1;
                        if (current_action->data[DX] < 0) {
                                toggle_dir(dir, X_DIRECTION_BIT);
                                current_action->data[DX] = -current_action->data[DX];
                                step_dir[0] = -1;
                        }
                        if (current_action->data[DY] < 0) {
                                toggle_dir(dir, Y_DIRECTION_BIT);
                                current_action->data[DY] = -current_action->data[DY];
                                step_dir[1] = -1;
                        }
                        if (current_action->data[DZ] < 0) {
                                toggle_dir(dir, Z_DIRECTION_BIT);
                                current_action->data[DZ] = -current_action->data[DZ];
                                step_dir[2] = -1;
                        }
                        
                        set_dir_pins(dir);
                        
                        /* Initialize the accumulation error for the
                         * Bresenham algorithm. */
                        accumulation_error[0] = (current_action->data[DX]
                                                 - current_action->data[DT] / 2);
                        accumulation_error[1] = (current_action->data[DY]
                                                 - current_action->data[DT] / 2);
                        accumulation_error[2] = (current_action->data[DZ]
                                                 - current_action->data[DT] / 2);
                }
        }

        /* 99.9% of the time, we will end up here. The block below
         * handles the movement of the steppers. It must be executed
         * as quickly as possible. Just like Grbl, the handler uses
         * Bresenham's algorithm to determine when a STEP pin should
         * be raised. The implementation here is based on the pseudo
         * code in Wikipedia
         * (https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm).
         */
        if (current_action->type == ACTION_MOVE) {
                
                uint8_t pins = 0;

                // X-axis
                if (accumulation_error[0] > 0) {
                        toggle_step(pins, X_STEP_BIT);
                        stepper_positions[0] += step_dir[0];
                        accumulation_error[0] -= current_action->data[DT];
                }
                accumulation_error[0] += current_action->data[DX];

                // Y-axis
                if (accumulation_error[1] > 0) {
                        toggle_step(pins, Y_STEP_BIT);
                        stepper_positions[1] += step_dir[1];
                        accumulation_error[1] -= current_action->data[DT];
                }
                accumulation_error[1] += current_action->data[DY];

                // Z-axis
                if (accumulation_error[2] > 0) {
                        toggle_step(pins, Z_STEP_BIT);
                        stepper_positions[2] += step_dir[2];
                        accumulation_error[2] -= current_action->data[DT];
                }
                accumulation_error[2] += current_action->data[DZ];

                // Raise the STEP pins, if needed, and schedule a
                // reset pins event.
                if (pins) {
                        set_step_pins(pins);
                        enable_reset_step_pins_timer();
                }

                // Update the stepper counter for this action.
                if (++counter_stepper_timer == current_action->data[DT]) {
                        current_action = 0;
                        return;
                }
                
                return;
        }

        /* Stop the execution of the stepper timer but don't
         * clear the action buffer. Wait for a 'continue'
         * command to execute the remaining actions. */
        if (current_action->type == ACTION_WAIT) {
                disable_stepper_timer();
                thread_state = STATE_THREAD_IDLE;
                current_action = 0;
                return;
        }

        /* Just do nothing. */
        if (current_action->type == ACTION_DELAY) {
                if (++counter_stepper_timer == current_action->data[DT])
                        current_action = 0;
                return;
        }

        /* Send the trigger ID to the calling program. */
        if (current_action->type == ACTION_TRIGGER) {
                trigger_buffer_put(current_action->data[0]);
                current_action = 0;
                return;
        }
}
