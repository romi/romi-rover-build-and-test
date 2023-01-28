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

#ifndef __M0_TIMER3_H
#define __M0_TIMER3_H

#include <Arduino.h>
#include "ITimer.h"

class M0Timer3 : public ITimer
{
protected:
        static M0Timer3 instance_;
        ITimerHandler *handler_;
        uint32_t counter_;
        float interval_;

        M0Timer3();
        virtual ~M0Timer3() override = default;

        void init();

public:

        static M0Timer3& get(); 
        
        void set_handler(ITimerHandler *handler) override;
        void start() override;
        void stop() override;
        void restart() override;

        void interrupt_handler();
};

#endif // __M0_TIMER3_H
