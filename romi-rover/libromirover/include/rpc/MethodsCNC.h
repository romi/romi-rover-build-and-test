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
#ifndef __ROMI_METHODS_CNC_H
#define __ROMI_METHODS_CNC_H

namespace romi {
        
        class MethodsCNC
        {
        public:
                
                static constexpr const char *homing = "cnc-homing";
                static constexpr const char *moveto = "cnc-moveto";
                static constexpr const char *spindle = "cnc-spindle";
                static constexpr const char *travel = "cnc-travel";
                static constexpr const char *get_range = "cnc-get-range";

        };
}

#endif // __ROMI_METHODS_CNC_H
