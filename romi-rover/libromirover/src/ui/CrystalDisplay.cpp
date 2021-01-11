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

#include <string.h>
#include <r.h>
#include "CrystalDisplay.h"

namespace romi {

        bool CrystalDisplay::show(int line, const char* s)
        {
                bool success = false;
                if (line >= 0 && line < count_lines() && s != 0 && strlen(s) <= 32) {
                        char buffer[64];
                        JsonCpp response;
                        
                        rprintf(buffer, 64, "S[%d,\"%s\"]", line, s);
                        
                        _serial.send(buffer, response);

                        return response.num(0) == 0;
                        
                } else {
                        r_warn("CrystalDisplay::show: Invalid line number or string "
                                "(line=%d, s='%s')", line, s);
                }
                return success;
        }
        
        bool CrystalDisplay::clear(int line)
        {
                bool success = false;
                if (line >= 0 && line < count_lines()) {
                        char buffer[64];
                        JsonCpp response;
                        
                        rprintf(buffer, 64, "C[%d]", line);
                        
                        _serial.send(buffer, response);

                        return response.num(0) == 0;
                        
                } else {
                        r_warn("CrystalDisplay::show: Invalid line number "
                                "(line=%d)", line);
                }
                return success;
        }
}
