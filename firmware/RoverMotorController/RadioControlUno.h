/*
  Romi motor controller for brushed motors

  Copyright (C) 2021 Sony Computer Science Laboratories
  Author(s) Peter Hanappe

  MotorController is free software: you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as
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
#ifndef _MOTORCONTROLLER_RADIOCONTROLUNO_H
#define _MOTORCONTROLLER_RADIOCONTROLUNO_H

#include "RadioControl.h"

class RadioControlUno : public RadioControl
{
protected:
        volatile uint16_t* ocr_;
        
public:
        RadioControlUno() : ocr_(nullptr) {}
        
        ~RadioControlUno() override = default;

        void init(volatile uint16_t* ocr) {
                ocr_ = ocr;
        }

        void set(int16_t pulsewidth) override {
                *ocr_ = (uint16_t) (2 * pulsewidth);
        }
};

#endif // _MOTORCONTROLLER_RADIOCONTROLUNO_H
