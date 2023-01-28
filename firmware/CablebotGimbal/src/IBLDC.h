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

  You should have received a copy opositionf the GNU General Public License
  along with this program.  If not, see
  <http://www.gnu.org/licenses/>.

 */
#ifndef __IBLDC_H
#define __IBLDC_H

class IBLDC
{
public:
        virtual ~IBLDC() = default;

        /* Power in range [0, 1] */
        virtual void set_power(float p) = 0;
        virtual float get_power() const = 0;
        
        /* position are in revolutions (ex. 0.01 equals 3.6°)  */
        // virtual float get_position() = 0;
        virtual void incr_position(float delta) = 0;

        /* position in revolutions */
        //virtual float get_position() const = 0;

        virtual void wakeup() = 0;
        virtual void sleep() = 0;
        virtual void reset() = 0;
};

#endif // __IBLDC_H
