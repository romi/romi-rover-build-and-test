/*
  bldc_featherwing

  Copyright (C) 2019-2020 Sony Computer Science Laboratories
  Author(s) Timothée Wintz, Peter Hanappe

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

#include "M0Timer3.h"

#define DEFAULT_CPUFREQ 48000000.0f
#define DEFAULT_PRESCALER 1.0f
#define DEFAULT_INTERVAL ((float) (65536.0f * DEFAULT_PRESCALER / DEFAULT_CPUFREQ))

M0Timer3 M0Timer3::instance_;

M0Timer3& M0Timer3::get()
{
        return instance_;
}

M0Timer3::M0Timer3()
        : handler_(nullptr),
          counter_(0),
          interval_(DEFAULT_INTERVAL)
{
}

void M0Timer3::set_handler(ITimerHandler *handler)
{
        if (handler_ == nullptr
            && handler != nullptr) {
                handler_ = handler;
                init();
        }
}

void M0Timer3::start()
{
        if (handler_) {
                TcCount16* TC = (TcCount16*) TC3;
                TC->CTRLBSET.reg |= TC_CTRLBSET_CMD_RETRIGGER;
        }
}

void M0Timer3::stop()
{
        TcCount16* TC = (TcCount16*) TC3;
        TC->CTRLBSET.reg |= TC_CTRLBSET_CMD_STOP;
}

void M0Timer3::restart()
{
        counter_ = 0;
        start();
}

void M0Timer3::interrupt_handler()
{
        counter_++;
        handler_->update(counter_ * interval_);
}

static inline void synchronize(TcCount16* timer)
{
        // Wait for sync
        while (timer->STATUS.bit.SYNCBUSY == 1)
                ; 
}

void M0Timer3::init()
{
        GCLK->CLKCTRL.reg = (uint16_t) (GCLK_CLKCTRL_CLKEN
                                        | GCLK_CLKCTRL_GEN_GCLK0
                                        | GCLK_CLKCTRL_ID(GCM_TCC2_TC3));

        // Wait for synchronization
        while (GCLK->STATUS.bit.SYNCBUSY)
                ;

        // Get timer struct
        TcCount16* TC = (TcCount16*) TC3;

        // Disable TC
        TC->CTRLA.reg &= ~TC_CTRLA_ENABLE;
        synchronize(TC);

        // Set Timer counter Mode to 16 bits
        TC->CTRLA.reg |= TC_CTRLA_MODE_COUNT16;
        synchronize(TC);
        
        // Set TC as normal Normal Frq
        TC->CTRLA.reg |= TC_CTRLA_WAVEGEN_NFRQ;
        synchronize(TC);

        // Set prescaler
        TC->CTRLA.reg |= TC_CTRLA_PRESCALER_DIV1;   
        synchronize(TC);
        
        // Interrupts
        // Disable all interrupts
        TC->INTENSET.reg = 0;
        // Enable overfollow interrup
        TC->INTENSET.bit.OVF = 1;          
        // Enable InterruptVector
        NVIC_EnableIRQ(TC3_IRQn);
        // Enable TC
        TC->CTRLA.reg |= TC_CTRLA_ENABLE;

        // Wait for sync
        synchronize(TC);
}

void TC3_Handler()  
{
        TcCount16* TC = (TcCount16*) TC3; // get timer struct
        M0Timer3::get().interrupt_handler();
        TC->INTFLAG.bit.OVF = 1;    // writing a one clears the ovf flag
}
