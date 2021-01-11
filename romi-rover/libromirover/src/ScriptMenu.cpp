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
#include "ScriptMenu.h"

namespace romi {
        
        void ScriptMenu::first_menu_item(std::string& name)
        {
                _current_menu = 0;
                get_current_menu(name);
        }

        bool ScriptMenu::has_menus()
        {
                return _scripts.size() > 0;
        }
        
        void ScriptMenu::next_menu_item(std::string& name)
        {
                if (has_menus()
                    && _current_menu < (int) _scripts.size() - 1)
                        _current_menu++;
                get_current_menu(name);
        }
        
        void ScriptMenu::previous_menu_item(std::string& name)
        {
                if (_current_menu > 0)
                        _current_menu--;
                get_current_menu(name);
        }

        void ScriptMenu::get_current_menu(std::string& name)
        {
                if (has_menus()) 
                        name = _scripts[_current_menu].title;
                else
                        name = Menu::Empty;
        }
        
        int ScriptMenu::get_current_index() 
        {
                return has_menus()? _current_menu : -1;
        }
}
