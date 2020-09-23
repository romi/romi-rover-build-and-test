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

#ifndef __IMENU_H
#define __IMENU_H

class IMenu
{
public:
        virtual ~IMenu() {}
        
        virtual int setMenuItem(const char *name, int i) = 0;

        virtual void firstMenuItem() = 0;
        virtual void nextMenuItem() = 0;
        virtual void previousMenuItem() = 0;
        virtual int currentMenuItemID() = 0;
        virtual const char *currentMenuItemName() = 0;
};

#endif // __IMENU_H
