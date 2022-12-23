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
#ifndef _MOTORCONTROLLER_SPEEDENVELOPE_H
#define _MOTORCONTROLLER_SPEEDENVELOPE_H

class SpeedEnvelope
{
public:
        
        int16_t max_;
        int16_t target_;
        int16_t current_;
        int16_t increment_;
        
        SpeedEnvelope()
                : max_(0),
                  target_(0),
                  current_(0),
                  increment_(0) {
        }
        
        void init(double max_speed, double acceleration, double dt) {
                max_ = (int16_t) (max_speed * 1000.0);
                double dv = acceleration * dt;
                increment_ = (int16_t) (dv * 1000.0);
        }

        void set_target(int16_t target) {
                if (target > max_)
                        target = max_;
                else if (target < -max_)
                        target = (int16_t) -max_;
                target_ = target;
        }

        int16_t get_target() {
                return target_;
        }

        int16_t get_current() {
                return current_;
        }

        int16_t update() {
                if (current_ < target_) {
                        current_ = (int16_t) (current_ + increment_);
                        if (current_ > target_)
                                current_ = target_;
                } else if (current_ > target_) {
                        current_ = (int16_t) (current_ - increment_);
                        if (current_ < target_)
                                current_ = target_;
                }
                return current_;
        }

        void stop() {
                target_ = 0;
                current_ = 0;
        }
};

#endif // _MOTORCONTROLLER_SPEEDENVELOPE_H
