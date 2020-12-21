/*
  romi-rover

  Copyright (C) 2019-2020 Sony Computer Science Laboratories
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

#include "CrystalDisplay.h"

CrystalDisplay::CrystalDisplay(int rs, int enable, int d4, int d5, int d6, int d7,
                               int cols = 16, int rows = 2)
        : _lcd(rs, enable, d4, d5, d6, d7) {
        _lcd.begin(cols, rows);
}

void CrystalDisplay::clear(int line)
{
        for (int col = 0; col < 16; col++) {
                _lcd.setCursor(col, line);
                _lcd.write(' ');
        }
}

void CrystalDisplay::show(int line, const char* s)
{
        clear(line);
        _lcd.setCursor(0, line);
        _lcd.print(s);
}
