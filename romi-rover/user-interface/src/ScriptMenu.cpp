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
                current_menu_item(name);
        }

        bool ScriptMenu::has_menus()
        {
                return _script_engine.count_scripts() > 0;
        }
        
        void ScriptMenu::next_menu_item(std::string& name)
        {
                if (has_menus()
                    && _current_menu < _script_engine.count_scripts() - 1)
                        _current_menu++;
                current_menu_item(name);
        }
        
        void ScriptMenu::previous_menu_item(std::string& name)
        {
                if (_current_menu > 0)
                        _current_menu--;
                current_menu_item(name);
        }

        void ScriptMenu::current_menu_item(std::string& name)
        {
                std::string id;
                if (has_menus())
                        _script_engine.get_script(_current_menu, id, name);
                else
                        name = "No menus!";
        }
        
        void ScriptMenu::current_menu_item_id(std::string& id)
        {
                std::string name;
                if (has_menus())
                        _script_engine.get_script(_current_menu, id, name);
                else
                        id = "No menus!";
        }
}
