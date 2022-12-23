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

  You should have received a copy opositionf the GNU General Public License
  along with this program.  If not, see
  <http://www.gnu.org/licenses/>.

 */
#ifndef __PINS_H
#define __PINS_H

//#define TIMS_BOARD

// Logic input pins
#define P_IN1 13
#define P_EN1 12
#define P_IN2 11
#define P_EN2 10
#define P_IN3 6
#define P_EN3 5

#define P_SLEEP 19
#if defined(TIMS_BOARD)
#define P_RESET 18
#else
#define P_RESET A0
#endif

// Encoder pin
#if defined(TIMS_BOARD)
#define P_ENC SDA
#else
#define P_ENC A1
#endif

#endif // __PINS_H

