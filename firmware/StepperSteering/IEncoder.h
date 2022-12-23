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
#ifndef _STEERING_I_ENCODER_H
#define _STEERING_I_ENCODER_H

#include <stdint.h>

class IEncoder
{
public:
        virtual ~IEncoder() = default;

        virtual void init(uint16_t pulses_per_revolution, int8_t direction) = 0;
        virtual int32_t get_position() = 0;
        virtual uint16_t positions_per_revolution() = 0;
        virtual bool get_index() = 0;
        virtual void reset_index() = 0;
        virtual void set_zero() = 0;


        virtual int32_t get_increment_count() = 0;
        virtual int32_t get_decrement_count() = 0;
};

#endif // _STEERING_I_ENCODER_H
