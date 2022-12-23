/*
  Romi motor controller for brushed motors

  Copyright (C) 2021 Sony Computer Science Laboratories
  Author(s) Peter Hanappe

  Azhoo is free software: you can redistribute it and/or modify it
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
#ifndef _STEERING_ANALOG_ABSOLUTE_ENCODER_H
#define _STEERING_ANALOG_ABSOLUTE_ENCODER_H

#include "IEncoder.h"

class AnalogAbsoluteEncoder : public IEncoder
{
public:
        int pin_;
        int direction_;
        int revolutions_;
        int prev_value_;
        int position_;
        int max_;
        int zero_;
        bool index_;
        
        AnalogAbsoluteEncoder(int pin)
                : pin_(pin),
                  direction_(1),
                  revolutions_(0),
                  prev_value_(0),
                  position_(0),
                  max_(1024),
                  zero_(512),
                  index_(false) {
        }
        
        ~AnalogAbsoluteEncoder() override = default;

        void init(uint16_t pulses_per_revolution, int8_t direction) {
                prev_value_ = analogRead(pin_);
                direction_ = direction;
                max_ = pulses_per_revolution;
        }

        void update_position() {
                int value = analogRead(pin_);
                if (prev_value_ > max_ - 100 && value < 100) {
                        revolutions_++;
                        Serial.print("Revolutions++ ");
                        Serial.print(revolutions_);
                        Serial.print(" value ");
                        Serial.print(value);
                        Serial.print(" position ");
                        Serial.println(direction_ * (revolutions_ * max_ + value));
                } else if (prev_value_ < 100 && value > max_ - 100) {
                        revolutions_--;
                        Serial.print("Revolutions-- ");
                        Serial.print(revolutions_);
                        Serial.print(" value ");
                        Serial.print(value);
                        Serial.print(" position ");
                        Serial.println(direction_ * (revolutions_ * max_ + value));
                }
                position_ = direction_ * (revolutions_ * max_ + value) - zero_;
                prev_value_ = value;
                /* if (pin_ == A2) { */
                /*         static unsigned long last_time = 0; */
                /*         unsigned long now = millis(); */
                /*         if (now - last_time > 1000) { */
                /*                 Serial.print(value); */
                /*                 Serial.print(" "); */
                /*                 Serial.println(position_); */
                /*                 last_time = now; */
                /*         } */
                /* } */
        }

        int32_t get_position() override {
                update_position();
                return position_;
        }

        int32_t get_increment_count() override {
                return 0;
        }

        int32_t get_decrement_count() override {
                return 0;
        }

        uint16_t positions_per_revolution() override {
                return max_;
        }

        // For testing only
        void set_position(int32_t t) {
                // nop
        }

        bool get_index() override {
                if (!index_) {
                        update_position();
                        index_ = (abs(position_) <= 2);
                }
                return index_;
        }
        
        void reset_index() override {
                index_ = false;
        }
        
        void set_index() {
                index_ = true;
        }

        void set_zero() override {
        }
};

#endif // _STEERING_ANALOG_ABSOLUTE_ENCODER_H
