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
#ifndef __ROMI_DEBUG_NAVIGATION_H
#define __ROMI_DEBUG_NAVIGATION_H

#include <r.h>
#include "api/Navigation.h"

namespace romi {

        class FakeNavigation : public Navigation
        {
        public:
                static constexpr const char *ClassName = "fake-navigation";
                
                FakeNavigation() {}
                virtual ~FakeNavigation() override = default;
                
                bool moveat(double left, double right) override {
                        r_debug("FakeNavigation: moveat(left=%0.3f, right=%0.3f)",
                                left, right);
                        return true;
                }

                bool move(double distance, double speed) override {
                        r_debug("FakeNavigation: move(distance=%0.3f, speed=%0.3f)",
                                distance, speed);
                        return true;
                }

                bool stop() override {
                        r_debug("FakeNavigation: stop");
                        return true;
                }
                
                bool pause_activity() override {
                        return true;
                }
                
                bool continue_activity() override {
                        return true;
                }
                
                bool reset_activity() override {
                        return true;
                }
        };
}

#endif // __ROMI_DEBUG_NAVIGATION_H
