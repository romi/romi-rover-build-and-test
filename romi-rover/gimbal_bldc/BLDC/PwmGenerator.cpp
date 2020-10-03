/*
  bldc_featherwing

  Copyright (C) 2019-2020 Sony Computer Science Laboratories
  Author(s) Peter Hanappe

  bldc_featherwing is Arduino firmware to control a brushless motor.

  bldc_featherwing is free software: you can redistribute it and/or
  modify it under the terms of the GNU General Public License as
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
#include "PwmGenerator.h"
#include "PwmOut.h"

PwmGenerator::PwmGenerator(IOutputPin *_pwm1,
                           IOutputPin *_pwm2,
                           IOutputPin *_pwm3,
                           IOutputPin *_enable1,
                           IOutputPin *_enable2,
                           IOutputPin *_enable3)
        : pwm1(_pwm1),
          pwm2(_pwm2),
          pwm3(_pwm3),
          enable1(_enable1),
          enable2(_enable2),
          enable3(_enable3)
{
        configurePwmFrequency();
}

void PwmGenerator::set(float duty_cycle1,
                       float duty_cycle2,
                       float duty_cycle3)
{        
        pwm1->set(duty_cycle1);
        pwm2->set(duty_cycle2);
        pwm3->set(duty_cycle3);
}

void PwmGenerator::enable()
{
        enable1->set(1.0f);
        enable2->set(1.0f);
        enable3->set(1.0f);
}

void PwmGenerator::disable()
{
        enable1->set(0.0f);
        enable2->set(0.0f);
        enable3->set(0.0f);
}

void PwmGenerator::configurePwmFrequency()
{
        // Generic clock Divisor: calculated by PPWM tool.
        // Select Generic Clock (GCLK) 4.
        REG_GCLK_GENDIV = (GCLK_GENDIV_DIV(3)
                           | GCLK_GENDIV_ID(4));
        
        // Wait for synchronization.
        while (GCLK->STATUS.bit.SYNCBUSY)
                ;

        // Set the duty cycle to 50/50 HIGH/LOW.
        // Enable GCLK4.
        // 48MHz clock source selected by PPWM.
        // Select GCLK4.
        REG_GCLK_GENCTRL = (GCLK_GENCTRL_IDC
                            | GCLK_GENCTRL_GENEN
                            | GCLK_GENCTRL_SRC_DFLL48M
                            | GCLK_GENCTRL_ID(4));

        // Wait for synchronization.
        while (GCLK->STATUS.bit.SYNCBUSY)
                ;              

        // Enable the port multiplexer for one channel: timer TCC1
        // outputs.
        const uint8_t CHANNELS = 3;
        const uint8_t pwmPins[] = {6, 11, 13};
        
        for (uint8_t i = 0; i < CHANNELS; i++) {
                PORT->Group[g_APinDescription[pwmPins[i]].ulPort].PINCFG[g_APinDescription[pwmPins[i]].ulPin].bit.PMUXEN = 1;
        }
        
        PORT->Group[g_APinDescription[6].ulPort].PMUX[g_APinDescription[6].ulPin >> 1].reg
                = PORT_PMUX_PMUXO_F | PORT_PMUX_PMUXE_F;
        
        PORT->Group[g_APinDescription[11].ulPort].PMUX[g_APinDescription[11].ulPin >> 1].reg
                = PORT_PMUX_PMUXO_E | PORT_PMUX_PMUXE_E;

        // Feed GCLK4 to TCC0 and TCC1
        // Enable GCLK4 to TCC0 and TCC1
        // Select GCLK4
        // Feed GCLK4 to TCC0 and TCC1
        REG_GCLK_CLKCTRL = (GCLK_CLKCTRL_CLKEN
                            | GCLK_CLKCTRL_GEN_GCLK4
                            | GCLK_CLKCTRL_ID_TCC0_TCC1);
        
        // Wait for synchronization
        while (GCLK->STATUS.bit.SYNCBUSY)
                ;
        
        // Enable GCLK4 to TCC2
        // Select GCLK4
        // Feed GCLK4 to TCC2
        REG_GCLK_CLKCTRL = (GCLK_CLKCTRL_CLKEN
                            | GCLK_CLKCTRL_GEN_GCLK4
                            | GCLK_CLKCTRL_ID_TCC2_TC3);

        // Wait for synchronization
        while (GCLK->STATUS.bit.SYNCBUSY)
                ;
        
        // Reverse the output polarity on all TCC0 outputs
        // Setup dual slope PWM on TCC0
        REG_TCC0_WAVE |= (TCC_WAVE_POL(0xF)
                          | TCC_WAVE_WAVEGEN_DSBOTTOM);
        
        // Wait for synchronization
        while (TCC0->SYNCBUSY.bit.WAVE)
                ;
        
        // Reverse the output polarity on all TCC0 outputs
        // Setup dual slope PWM on TCC0
        REG_TCC2_WAVE |= (TCC_WAVE_POL(0xF)
                          | TCC_WAVE_WAVEGEN_DSBOTTOM);

        // Wait for synchronization
        while (TCC2->SYNCBUSY.bit.WAVE)
                ;
        
        // Resolution for TCC0 Register.
        REG_TCC0_PER = PERX;                        
        while(TCC0->SYNCBUSY.bit.PER)
                ;
        
        // Resolution for TCC0 Register. 
        REG_TCC2_PER = PERX;
        while(TCC2->SYNCBUSY.bit.PER)
                ;

        // TCC0 CCB1 - 50% PWM on D3
        REG_TCC0_CCB2 = (PERX/2);       
        while (TCC0->SYNCBUSY.bit.CCB2)
                ;

        // TCC0 CCB1 - 50% PWM on D3
        REG_TCC2_CCB0 = (PERX/2);       
        while (TCC2->SYNCBUSY.bit.CCB0)
                ;

        // TCC0 CCB1 - 50% PWM on D3
        REG_TCC2_CCB1 = (PERX/2);       
        while (TCC2->SYNCBUSY.bit.CCB1)
                ;
 
        // Divide the Divisor processed Clock signal by the Prescaler
        // factor and enable the outputs.

        // Prescaler Factor from the PPWM tool.
        // Enable the TCC0 output.
        REG_TCC0_CTRLA |= (TCC_CTRLA_PRESCALER_DIV1
                           | TCC_CTRLA_ENABLE);            

        // Wait for synchronization.
        while (TCC0->SYNCBUSY.bit.ENABLE)
                ;

        // Prescaler Factor from the PPWM tool.
        // Enable the TCC2 output.
        REG_TCC2_CTRLA |= (TCC_CTRLA_PRESCALER_DIV1
                           | TCC_CTRLA_ENABLE);
        
        // Wait for synchronization.
        while (TCC2->SYNCBUSY.bit.ENABLE)
                ;
        
        // delay(5000);        
}

