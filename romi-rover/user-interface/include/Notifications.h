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

#ifndef _ROMI_NOTIFICATIONS_H
#define _ROMI_NOTIFICATIONS_H

namespace romi {

        static constexpr const char *NotifyStartup = "notify-startup";
        static constexpr const char *NotifyConfirmMenuMode = "notify-confirm-menu-mode";
        static constexpr const char *NotifyLeaveMenuMode = "notify-leave-menu-mode";
        static constexpr const char *NotifyChangeMenu = "notify-change-menu";
        static constexpr const char *NotifyMenuConfirmed = "notify-menu-confirmed";
        static constexpr const char *NotifyMenuFinished = "notify-menu-finished";
        static constexpr const char *NotifyConfirmNavigationMode = "notify-confirm-navigation-mode";
        static constexpr const char *NotifyLeaveNavigationMode = "notify-leave-navigation-mode";
        static constexpr const char *NotifyMoving = "notify-moving";

        class Notifications
        {
        public:
                virtual ~Notifications() = default;

                virtual void notify(const char *name) = 0;
                virtual void stop(const char *name) = 0;
        };
}

#endif // _ROMI_NOTIFICATIONS_H
