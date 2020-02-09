/*
  romi-rover

  Copyright (C) 2019 Sony Computer Science Laboratories
  Author(s) Peter Hanappe

  romi-rover is collection of applications for the Romi Rover.

  romi-rover is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see
  <http://www.gnu.org/licenses/>.

 */
#include <avr/io.h>
#include <avr/interrupt.h>

#ifndef _OQUAM_PINS_EXTCTRLR_H_
#define _OQUAM_PINS_EXTCTRLR_H_

#if USE_EXT_CONTROLLER

#define USE_UNO            0
#define USE_MEGA2560       1

#if USE_MEGA2560
#include "extctrlr_mega2560.h"
#endif

#if USE_UNO
#include "extctrlr_uno.h"
#endif

#endif // USE_EXT_CONTROLLER
#endif // _OQUAM_PINS_EXTCTRLR_H_
