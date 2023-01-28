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

#ifndef __ISPEEDCONTROLLER_H
#define __ISPEEDCONTROLLER_H

class ISpeedController
{
public:
        virtual ~ISpeedController() = default;
        virtual void update_speed(float time) = 0;
        virtual void set_target_speed(float v) = 0;
        virtual void init_start_speed() = 0;
};

#endif // __ISPEEDCONTROLLER_H
