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
#include "stepper.h"
#include "MovingAverage.h"

volatile int32_t left_stepper_position_ = 0;
volatile int32_t right_stepper_position_ = 0;
volatile ControlMode mode_ = kOpenLoopControl;
volatile static int16_t left_encoder_target_ = 0;
volatile static int16_t right_encoder_target_ = 0;
volatile static int32_t left_stepper_target_ = 0;
volatile static int32_t right_stepper_target_ = 0;
static IArduino *arduino_;
static MovingAverage left_filter_;
static MovingAverage right_filter_;

static uint8_t pins_ = 0;
static uint8_t dir_ = 0;
volatile int16_t left_encoder_value_ = 0;
volatile int16_t right_encoder_value_ = 0;
static uint8_t encoder_update_counter_ = 0;



void init_stepper_timer();
void init_reset_step_pins_timer();

void stepper_zero_left()
{
        left_stepper_position_ = 0;
}

void stepper_zero_right()
{
        right_stepper_position_ = 0;
}

void stepper_reset()
{
        stepper_zero_left();
        stepper_zero_right();
        set_stepper_target(0, 0);
        set_encoder_target(0, 0);
}

void stepper_reset_axis(int8_t axis)
{
        if (axis == 0) {
                stepper_zero_left();
                set_stepper_target(0, right_stepper_target_);
                set_encoder_target(0, right_encoder_target_);
        } else if (axis == 1) {
                stepper_zero_right();
                set_stepper_target(left_stepper_target_, 0);
                set_encoder_target(left_encoder_target_, 0);
        }
}

void stepper_zero_axis(int8_t axis)
{
        if (axis == 0) {
                stepper_zero_left();
        } else if (axis == 1) {
                stepper_zero_right();
        }
}

void init_stepper(IArduino *arduino)
{
        arduino_ = arduino;
        stepper_reset();
        cli();
        init_stepper_timer();
        init_reset_step_pins_timer();
        sei(); 
}

int32_t get_stepper_left()
{
        return left_stepper_position_;
}

int32_t get_stepper_right()
{
        return right_stepper_position_;
}

int32_t get_stepper_position(int8_t axis)
{
        if (axis == 0)
                return get_stepper_left();
        else
                return get_stepper_right();
}

void set_encoder_target(int16_t left, int16_t right)
{
        left_encoder_target_ = left;
        right_encoder_target_ = right;
}

void get_encoder_target(int16_t& left, int16_t& right)
{
        left = left_encoder_target_;
        right = right_encoder_target_;
}

void set_stepper_target(int32_t left, int32_t right)
{
        left_stepper_target_ = left;
        right_stepper_target_ = right;
}

void get_stepper_target(int32_t& left, int32_t& right)
{
        left = left_stepper_target_;
        right = right_stepper_target_;
}


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
        //   1     0     1     1024
        //   1     1     0     Use external clock, falling edge
        //   1     1     1     Use external clock, rising edge

        switch (PRESCALING) {
        case 1:
                TCCR1B &= ~(1 << CS12); // 0
                TCCR1B &= ~(1 << CS11); // 0
                TCCR1B |=  (1 << CS10); // 1
                break;
        case 8:
                TCCR1B &= ~(1 << CS12);  // 0
                TCCR1B |= (1 << CS11);   // 1
                TCCR1B &=  ~(1 << CS10); // 0
                break;
        case 64:
                TCCR1B &= ~(1 << CS12); // 0
                TCCR1B |= (1 << CS11);  // 1
                TCCR1B |= (1 << CS10);  // 1
                break;
        case 256:
                TCCR1B |= (1 << CS12);  // 1
                TCCR1B &= ~(1 << CS11); // 0
                TCCR1B &= ~(1 << CS10); // 0
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

           int n = (1 / F_STEPPER) / (1 / F_CPU / prescaling) - 1
                 = F_CPU / (F_STEPPER * prescaling) - 1
                 = 639

           F_STEPPER = 10000 (10 kHz)
           P_STEPPER = 1/10000 = 100 µs
           F_CPU = 16 MHz
           Prescaling = 8
           F_CLOCK = 16 MHz / 8 = 2 MHz
           P_CLOCK = 1/2 µs
           N = 100 µs / (1/2 µs) = 200
           N-1 = 199

           F_STEPPER = 2000 (2 kHz)
           P_STEPPER = 1/2000 = 500 µs
           F_CPU = 16 MHz
           Prescaling = 64
           F_CLOCK = 16 MHz / 64 = 0.25 MHz
           P_CLOCK = 4 µs
           N = 500 µs / (4 µs) = 125
           N-1 = 124

           F_STEPPER = 1000 (1 kHz)
           P_STEPPER = 1/1000 = 1000 µs
           F_CPU = 16 MHz
           Prescaling = 64
           F_CLOCK = 16 MHz / 64 = 0.25 MHz
           P_CLOCK = 4 µs
           N = 1000 µs / (4 µs) = 250
           N-1 = 249

           F_STEPPER = 500 (1/2 kHz)
           P_STEPPER = 1/500 = 2000 µs
           F_CPU = 16 MHz
           Prescaling = 256
           F_CLOCK = 16 MHz / 256 = 0.25/4 MHz
           P_CLOCK = 16 µs
           N = 2000 µs / (16 µs) = 125
           N-1 = 124
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
        //counter_reset_timer++;
}

static inline void left_step_up()
{
        toggle_left_step(pins_);
        left_stepper_position_++;
}

static inline void left_step_down()
{
        toggle_left_step(pins_);
        toggle_dir(dir_, LEFT_DIRECTION_BIT);
        left_stepper_position_--;
}

static inline void right_step_up()
{
        toggle_right_step(pins_);
        right_stepper_position_++;
}

static inline void right_step_down()
{
        toggle_right_step(pins_);
        toggle_dir(dir_, RIGHT_DIRECTION_BIT);
        right_stepper_position_--;
}

static inline void move_to_stepper_target()
{
        if (left_stepper_position_ < left_stepper_target_) {
                left_step_up();
        } else if (left_stepper_position_ > left_stepper_target_) {
                left_step_down();
        }

        if (right_stepper_position_ < right_stepper_target_) {
                right_step_up();
        } else if (right_stepper_position_ > right_stepper_target_) {
                right_step_down();
        }
}

static inline int16_t update_encoder_value(IEncoder& encoder, IFilter& filter)
{
        int16_t value = (int16_t) encoder.get_position();
        return filter.process(value);
}

static inline void do_update_encoder_values()
{
        left_encoder_value_ = update_encoder_value(arduino_->left_encoder(),
                                                   left_filter_);
        right_encoder_value_ = update_encoder_value(arduino_->right_encoder(),
                                                    right_filter_);
}

static inline void update_encoder_values()
{
        if (encoder_update_counter_ == 0) {
                do_update_encoder_values();
        }
        encoder_update_counter_++;
        if (encoder_update_counter_ * PERIOD_STEPPER_MILLIS == ENCODER_PERIOD_MILLIS) {
                encoder_update_counter_ = 0;
        }
}

static inline void step_to_encoder_target()
{
        if (left_encoder_value_ < left_encoder_target_ - 1) {
                left_step_up();
        } else if (left_encoder_value_ > left_encoder_target_ + 1) {
                left_step_down();
        }

        if (right_encoder_value_ < right_encoder_target_ - 1) {
                right_step_up();
        } else if (right_encoder_value_ > right_encoder_target_ + 1) {
                right_step_down();
        }
}

static inline void move_to_encoder_target()
{
        update_encoder_values();
        step_to_encoder_target();
}

/**
 * \brief The interrupt service routine for the stepper timer.
 *
 */
ISR(TIMER1_COMPA_vect)
{
        pins_ = 0;
        dir_ = 0;
        
        if (mode_ == kOpenLoopControl) {
                move_to_stepper_target();
        } else if (mode_ == kClosedLoopControl) {
                move_to_encoder_target();
        }
        
        if (pins_) {
                set_dir_pins(dir_);
                set_step_pins(pins_);
                enable_reset_step_pins_timer();
        }
}
