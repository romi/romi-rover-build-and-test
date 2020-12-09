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
#ifndef _OQUAM_CONFIG_H_
#define _OQUAM_CONFIG_H_

/*

Required pins:

  What                          Num   I/O   Interupt 
----------------------------------------------------
* step:                          3     O
* direction:                     3     O
* encoders (A):                  3     I    yes  (Ext. interrupt)
  encoders (B):                  3     I    
* limit switch (xyx, min/max):   1(6)  I    yes  (PCINT)
* enable:                        1     O
* spindle:                       1     O    
* serial:                        2    I/O   yes
----------------------------------------------------
* Total                        17(22)       6(10)


Configuration     Board        Encoders   Limit switch   
------------------------------------------------------------
gShield           Uno          no         yes (min, not max)
RAMPS             Mega 2560              
Ext. controller   Uno                      


*/

#define USE_GSHIELD 1
#define USE_RAMPS 0
#define USE_EXT_CONTROLLER 0

#if USE_GSHIELD
#include "gshield.h"

#elif USE_RAMPS
#include "ramps.h"

#elif USE_EXT_CONTROLLER
#include "extctrlr.h"
#endif

#endif // _OQUAM_CONFIG_H_
