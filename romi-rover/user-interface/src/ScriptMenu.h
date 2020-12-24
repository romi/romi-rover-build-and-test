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
#ifndef _ROMI_DEFAULT_MENU_H
#define _ROMI_DEFAULT_MENU_H

#include <vector>
#include "Menu.h"
#include "ScriptEngine.h"

namespace romi {
        
        class ScriptMenu : public Menu
        {
        protected:
                ScriptEngine& _script_engine;
                int _current_menu;

                bool has_menus();
                
        public:
                ScriptMenu(ScriptEngine& script_engine)
                        : _script_engine(script_engine), _current_menu(0) {}
                
                virtual ~ScriptMenu() override = default;
                
                void first_menu_item(std::string& name) override;
                void next_menu_item(std::string& name) override;
                void previous_menu_item(std::string& name) override;
                void current_menu_item(std::string& name) override;
                void current_menu_item_id(std::string& id) override;
        };
}

#endif // _ROMI_DEFAULT_MENU_H
