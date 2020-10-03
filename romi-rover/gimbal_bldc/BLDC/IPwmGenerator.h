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

#ifndef __IPWM_GENERATOR_H
#define __IPWM_GENERATOR_H

class IPwmGenerator
{
public:
        virtual ~IPwmGenerator() {}

        virtual void set(float duty_cycle1, float duty_cycle2, float duty_cycle3) = 0;
        virtual void enable() = 0;
        virtual void disable() = 0;
};

#endif // __IPWM_GENERATOR_H
