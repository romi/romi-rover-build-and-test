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
#ifndef __PWM_ENCODER_H
#define __PWM_ENCODER_H

#include "IArduino.h"
#include "IEncoder.h"
#include <stdint.h>

class PwmEncoder;

void setPwmEncoder(PwmEncoder *encoder);
void PwmEncoderRise();
void PwmEncoderFall();

class PwmEncoder : public IEncoder
{
public:
        uint16_t count_;
        uint16_t value0_;
        
private:
        IArduino *arduino_;
        int pin_;
        unsigned long previous_time_;
        uint16_t value_;
        uint16_t min_value_;
        uint16_t max_value_;
        bool inverted_;
        float unit_to_angle;


        friend void PwmEncoderRise();
        friend void PwmEncoderFall();
        
        void rise() {
                previous_time_ = arduino_->micros();
                attachInterrupt(pin_, PwmEncoderFall, FALLING);
        }
        
        void fall() {
                uint16_t tmp = arduino_->micros() - previous_time_;
                
                value0_ = tmp;
                count_++;
                
                /** Filter the value using [min,max] and |delta| < 100
                 * because the reading is not always reliable.  */
                if (tmp >= min_value_ && tmp <= max_value_) {
                        // uint16_t delta = value - tmp;
                        // if (delta < 0)
                        //         delta = -delta;
                        // if (delta < 100)
                                value_ = tmp;
                }
                attachInterrupt(pin_, PwmEncoderRise, RISING);
        }

public:
        PwmEncoder(IArduino *arduino, int pin,
                   uint16_t min_value, uint16_t max_value)
                : arduino_(arduino),
                  pin_(pin), 
                  previous_time_(0),
                  value_(0),
                  min_value_(min_value),
                  max_value_(max_value),
                  inverted_(false) {
                count_ = 0;
                value0_ = 0;
                // Sets the global encoder, used by the interrupt
                // service routines.
                setPwmEncoder(this); // Hmmm...
                unit_to_angle = 1.0f / (float) (max_value_ - min_value_);
                arduino_->attachInterrupt(pin_, PwmEncoderFall, FALLING);
        }
        
        
        virtual ~PwmEncoder() {}

        void set_inverted(bool value) override;
        bool get_inverted() override;
        
        uint16_t get_value() {
                return value_;
        }

        float get_angle() override {
                float angle = unit_to_angle * (float) (value_ - min_value_);
                if (inverted_) {
                        angle = 1.0f - angle;
                }
                return angle;
        }
};

#endif // __PWM_ENCODER_H
