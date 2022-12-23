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
#ifndef _STEERING_MOVINGAVERAGE_H
 #define _STEERING_MOVINGAVERAGE_H

#include "IFilter.h"

class MovingAverage : public IFilter
{
public:
        const static uint8_t kSize = 10;
        
        uint8_t index_;
        int16_t buffer_[kSize];
        
        MovingAverage();
        ~MovingAverage() override = default;
        int16_t process(int16_t) override;
};

#endif // _STEERING_MOVINGAVERAGE_H
