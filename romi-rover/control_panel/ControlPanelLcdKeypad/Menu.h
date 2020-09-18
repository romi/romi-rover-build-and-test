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
#include "IMenu.h"

#ifndef __MENU_H
#define __MENU_H

class MenuItem
{
protected:
        char _name[17];

public:
        MenuItem() {
                clear();
        }

        virtual void clear() {
                memset(_name, 0, sizeof(_name));
        }

        virtual void setName(const char* name) {
                clear();
                for (int i = 0; name[i] != 0 && i < 16; i++)
                        _name[i] = name[i];
        }
        
        virtual const char* getName() {
                return _name;
        }

        virtual bool hasName() {
                return _name[0] != 0;
        }
};

#define MAX_MENU_ITEMS 16

class Menu : public IMenu
{
protected:
        MenuItem _items[MAX_MENU_ITEMS];
        int _current;

public:

        Menu() : _current(0) {}
        
        virtual ~Menu() {}

        virtual int setMenuItem(const char *name, int i) {
                int r = -1;
                if (i >= 0 && i < MAX_MENU_ITEMS) {
                        _items[i].setName(name);
                        r = 0;
                }
                return r;
        }

        virtual void firstMenuItem() {
                _current  = 0;
                while (!_items[_current].hasName()) {
                        _current++;
                        if (_current == MAX_MENU_ITEMS) {
                                _current = 0;
                                break;
                        }
                }
        }

        virtual void nextMenuItem() {
                int cur = _current + 1;
                if (cur == MAX_MENU_ITEMS)
                        cur = 0;
                while (!_items[cur].hasName()) {
                        cur++;
                        if (cur == MAX_MENU_ITEMS)
                                cur = 0;
                        if (cur == _current)
                                break;
                }
                _current = cur;
        }
        
        virtual void previousMenuItem() {
                int cur = _current - 1;
                if (cur <= 0)
                        cur = MAX_MENU_ITEMS - 1;
                while (!_items[cur].hasName()) {
                        cur--;
                        if (cur <= 0)
                                cur = MAX_MENU_ITEMS - 1;
                        if (cur == _current)
                                break;
                }
                _current = cur;
        }
        
        virtual const char *currentMenuItemName() {
                return _items[_current].getName();
        }
        
        virtual int currentMenuItemID() {
                return _current;
        }
};

#endif // __MENU_H
