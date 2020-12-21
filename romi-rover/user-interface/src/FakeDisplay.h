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

#ifndef __ROMI_FAKE_DISPLAY_H
#define __ROMI_FAKE_DISPLAY_H

#include "Display.h"

namespace romi {
        
        class FakeDisplay : public Display
        {
        public:
                static constexpr const char *ClassName = "fake-display";
                
                std::string _line[2];
                
        public:
                FakeDisplay() {}
                virtual ~FakeDisplay() = default;

                bool show(int line, const char* s) override {
                        bool success = false;
                        if (line >= 0 && line < count_lines()) {
                                _line[line] = s;
                                success = true;
                        }
                        return success;
                }
                
                bool clear(int line) override {
                        bool success = false;
                        if (line >= 0 && line < count_lines()) {
                                _line[line].clear();
                                success = true;
                        }
                        return success;
                }
                
                int count_lines() override {
                        return 2;
                }
        };
}
#endif // __ROMI_FAKE_DISPLAY_H
