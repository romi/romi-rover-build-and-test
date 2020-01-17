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
// CPU frequency Uno: 16000000
// Timer 1: Stepper timer
// Timer 2: Reset step pins

#include "Arduino.h"
#include "config.h"
#include "block.h"
#include "trigger.h"
#include "stepper.h"

volatile int32_t stepper_position[3];
volatile int32_t accumulation_error[3];
volatile int32_t dt;
volatile int16_t step_dir[3];
volatile uint8_t thread_state;
volatile int32_t counter_reset_timer = 0;
volatile block_t *current_block;
volatile int32_t interrupts = 0;
volatile int16_t milliseconds;
volatile int16_t block_id = -1;
volatile int16_t block_ms = -1;
volatile int8_t stepper_reset = 0;


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

/**
 * \brief: Configure Timer1 to drive the stepper's STEP pulse train.
 *
 *  Timer1 is used as the "stepper timer", i.e. this timer will pulse
 *  the STEP pins of the stepper drivers.
 */
void init_stepper_timer()
{
        current_block = 0;

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

        switch (PRESCALING) {
        case 1:
                TCCR1B |=  (1 << CS10);
                TCCR1B &= ~(1 << CS11);
                TCCR1B &= ~(1 << CS12);
                break;
        case 8:
                TCCR1B &=  ~(1 << CS10);
                TCCR1B |= (1 << CS11);
                TCCR1B &= ~(1 << CS12);
                break;
        }
        
        uint16_t compare_value = F_CPU / PRESCALING / FREQUENCY_STEPPER - 1;
        
        /* Set the compare value: 
           
           F_STEPPER = 25000 (25 kHz)
           P_STEPPER = 1/25000 = 40 µs
           F_CPU = 16 MHz
           Prescaling = 1
           F_CLOCK = 16 MHz
           P_CLOCK = 1/16 µs
           N = 40 µs / (1/16 µs) = 640
           N-1 = 639

           F_STEPPER = 10000 (10 kHz)
           P_STEPPER = 1/10000 = 100 µs
           F_CPU = 16 MHz
           Prescaling = 8
           F_CLOCK = 16 MHz / 8 = 2 MHz
           P_CLOCK = 1/2 µs
           N = 100 µs / (1/2 µs) = 200
           N-1 = 199

           int n = (1 / F_STEPPER) / (1 / F_CPU / prescaling) - 1
                 = F_CPU / (F_STEPPER * prescaling) - 1
                 = 639
         */
        OCR1A = compare_value;
        // Serial.print("stepper.c: compare_value=");
        // Serial.println(compare_value);
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
        /* If a reset is requested, set the current block and other
         * state variables to zero and return. */
        if (stepper_reset) {
                current_block = 0;
                block_id = -1;
                block_ms = 0;
                interrupts = 0;
                milliseconds = 0;
                stepper_reset = 0;
                return;
        }

        /* If there is no block active then pop the next block from
         * the buffer and do some initialization.  */
        if (current_block == 0) {
                
                current_block = block_buffer_get_next();

                if (current_block == 0) {
                        block_id = -1;
                        block_ms = 0;
                        return;
                }
                block_id = current_block->id;
                block_ms = 0;
                
                /* Do the necessary initializations for the new
                 * block. */
                
                interrupts = 0;
                milliseconds = 0;
                
                if (current_block->type == BLOCK_MOVE
                    || current_block->type == BLOCK_MOVEAT) {

                        /* Sanity check */
                        if (current_block->type == BLOCK_MOVE
                            && current_block->data[DT] <= 0) {
                                        current_block = 0;
                                        block_id = -1;
                                        return;
                        }
                        
                        block_ms = current_block->data[DT];
                                
                        /* Check the direction and set DIR pins. */
                        uint8_t dir = 0;
                        step_dir[0] = 1;
                        step_dir[1] = 1;
                        step_dir[2] = 1;
                        
                        if (current_block->data[DX] < 0) {
                                toggle_dir(dir, X_DIRECTION_BIT);
                                current_block->data[DX] = -current_block->data[DX];
                                step_dir[0] = -1;
                        }
                        if (current_block->data[DY] < 0) {
                                toggle_dir(dir, Y_DIRECTION_BIT);
                                current_block->data[DY] = -current_block->data[DY];
                                step_dir[1] = -1;
                        }
                        if (current_block->data[DZ] < 0) {
                                toggle_dir(dir, Z_DIRECTION_BIT);
                                current_block->data[DZ] = -current_block->data[DZ];
                                step_dir[2] = -1;
                        }

                        /* Set the direction output pins */
                        //Serial.println(dir);
                        set_dir_pins(dir);
                        
                        
                        /* The number of time steps during which the
                         * stepper positions are updated is equal to
                         * the length of the segment in milliseconds
                         * times the number of interrupts per
                         * millisecond. */
                        dt = INTERRUPTS_PER_MILLISECOND * (int32_t) current_block->data[DT];
                        
                        /* Initialize the accumulation error for the
                         * Bresenham algorithm. */
                        accumulation_error[0] = (int32_t) current_block->data[DX] - dt / 2;
                        accumulation_error[1] = (int32_t) current_block->data[DY] - dt / 2;
                        accumulation_error[2] = (int32_t) current_block->data[DZ] - dt / 2;
                        
                } else if (current_block->type == BLOCK_DELAY) {
                        /* Sanity check */
                        if (current_block->data[DT] <= 0) {
                                current_block = 0;
                                block_id = -1;
                                return;
                        }
                        block_ms = current_block->data[DT];
                }
                
        }

        /* Update the interrupt and millisecond counter. */
        if (++interrupts == INTERRUPTS_PER_MILLISECOND) {
                interrupts = 0;
                milliseconds++;
        }
        
        /* Move and moveat block: 99.9% of the time, we will end up
         * here. The block below handles the movement of the
         * steppers. It must be executed as quickly as possible. Just
         * like Grbl, the handler uses Bresenham's algorithm to
         * determine when a STEP pin should be raised. The
         * implementation here is based on the pseudo code found in
         * Wikipedia
         * (https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm).
         */
        if (current_block->type == BLOCK_MOVE
            || current_block->type == BLOCK_MOVEAT) {
                
                uint8_t pins = 0;

                // X-axis
                if (accumulation_error[0] > 0) {
                        toggle_step(pins, X_STEP_BIT);
                        stepper_position[0] += step_dir[0];
                        accumulation_error[0] -= dt;
                }
                accumulation_error[0] += current_block->data[DX];

                // Y-axis
                if (accumulation_error[1] > 0) {
                        toggle_step(pins, Y_STEP_BIT);
                        stepper_position[1] += step_dir[1];
                        accumulation_error[1] -= dt;
                }
                accumulation_error[1] += current_block->data[DY];

                // Z-axis
                if (accumulation_error[2] > 0) {
                        toggle_step(pins, Z_STEP_BIT);
                        stepper_position[2] += step_dir[2];
                        accumulation_error[2] -= dt;
                }
                accumulation_error[2] += current_block->data[DZ];

                // Raise the STEP pins, if needed, and schedule a
                // reset pins event.
                if (pins) {
                        set_step_pins(pins);
                        enable_reset_step_pins_timer();
                }

                // Check whether we have to move to the next block
                if ((current_block->type == BLOCK_MOVE 
                     && milliseconds >= current_block->data[DT])
                    || (current_block->type == BLOCK_MOVEAT
                        && block_buffer_available() > 0)) {
                        current_block = 0;
                }
                
                return;
        }

        /* Wait block: Stop the execution of the stepper timer but
         * don't clear the block buffer. A 'continue' command must be
         * issued to execute the remaining blocks. */
        if (current_block->type == BLOCK_WAIT) {
                disable_stepper_timer();
                current_block = 0;
                return;
        }

        /* Delay block: Just do nothing. */
        if (current_block->type == BLOCK_DELAY) {
                if (milliseconds == current_block->data[DT])
                        current_block = 0;
                return;
        }

        /* Trigger block: Send the trigger ID to the calling
         * program. */
        if (current_block->type == BLOCK_TRIGGER) {
                trigger_buffer_put(current_block->data[0]);
                current_block = 0;
                return;
        }
}
