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

#ifndef _ROMI_ROVER_NOTIFICATIONS_H
#define _ROMI_ROVER_NOTIFICATIONS_H

namespace romi {

        class RoverNotifications
        {
        public:

                static constexpr const char *startup = "startup";
                static constexpr const char *confirm_menu_mode = "confirm-menu-mode";
                static constexpr const char *leave_menu_mode = "leave-menu-mode";
                static constexpr const char *change_menu = "change-menu";
                static constexpr const char *menu_confirmed = "menu-confirmed";
                static constexpr const char *confirm_navigation_mode = "confirm-navigation-mode";
                static constexpr const char *leave_navigation_mode = "leave-navigation-mode";
                static constexpr const char *moving = "moving";

                static constexpr const char *script_finished = "script-finished";
                static constexpr const char *script_failed = "script-failed";
                static constexpr const char *rover_reset = "rover-reset";
        };
}

#endif // _ROMI_ROVER_NOTIFICATIONS_H
