#include <Arduino.h>
#include "ArduinoUno.h"
#include "Timer1.h"

#define SET_0(_reg, _bit) { _reg &=  ~(1 << _bit); }
#define SET_1(_reg, _bit) { _reg |=  (1 << _bit); }

static ArduinoUno instance_;

ArduinoUno& ArduinoUno::get()
{
        return instance_;
}

void ArduinoUno::block_interrupts()
{
        cli();
}

void ArduinoUno::allow_interrupts()
{
        sei();
}

uint32_t ArduinoUno::cpu_freq()
{
        return F_CPU;
}

void ArduinoUno::timer1_enable()
{
        /* Initialize counter */
        TCNT1 = 0;
        SET_1(TIMSK1, OCIE1A);
}

void ArduinoUno::timer1_disable()
{
        SET_0(TIMSK1, OCIE1A);
}

void ArduinoUno::timer1_set_prescaling(uint16_t prescaling)
{
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
        switch (prescaling) {
        case 1:
                SET_0(TCCR1B, CS12);
                SET_0(TCCR1B, CS11);
                SET_1(TCCR1B, CS10);
                break;
        case 8:
                SET_0(TCCR1B, CS12);
                SET_1(TCCR1B, CS11);
                SET_0(TCCR1B, CS10);
                break;
        case 64:
                SET_0(TCCR1B, CS12);
                SET_1(TCCR1B, CS11);
                SET_1(TCCR1B, CS10);
                break;
        case 256:
                SET_1(TCCR1B, CS12);
                SET_0(TCCR1B, CS11);
                SET_0(TCCR1B, CS10);
                break;
        case 1024:
                SET_1(TCCR1B, CS12);
                SET_0(TCCR1B, CS11);
                SET_1(TCCR1B, CS10);
                break;
        default:
                SET_0(TCCR1B, CS12);
                SET_0(TCCR1B, CS11);
                SET_0(TCCR1B, CS10);
                break;
        }
}

void ArduinoUno::timer1_set_compare_value(uint16_t value)
{
        OCR1A = value;
}

void ArduinoUno::timer1_disconnect_output_pin()
{
        // Disconnect OC1 output: Don't send the PWM to an output
        // pin.
        SET_0(TCCR1A, COM1A1);
        SET_0(TCCR1A, COM1A0);
        SET_0(TCCR1A, COM1B1);
        SET_0(TCCR1A, COM1B0);        
}

void ArduinoUno::timer1_clear_timer_on_compare()
{
        // Use the waveform generation mode, or Clear Timer on Compare
        // (CTC) mode.
        //
        // Register  WGM13 WGM12 WGM11 WGM10
        // TCCR1A                0     0
        // TCCR1B    0     1
        SET_0(TCCR1A, WGM10);
        SET_0(TCCR1A, WGM11);
        SET_1(TCCR1B, WGM12);
        SET_0(TCCR1B, WGM13);
}

// void ArduinoUno::timer1_interrupt_handler()
// {
//         instance_.timer1_.update();
// }

// ISR(TIMER1_COMPA_vect)
// {
//         ArduinoUno::timer1_interrupt_handler();
// }
