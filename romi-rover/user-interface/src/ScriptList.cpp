/*
  romi-rover

  Copyright (C) 2019 Sony Computer Science Laboratories
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
#include <thread>
#include <stdexcept>
#include <r.h>
#include "ScriptList.h"

namespace romi {
                
        ScriptList::ScriptList(const char *path)
        {
                try {
                        
                        load_scripts(path);
                
                } catch (JSONError& je) {
                        r_err("ScriptList: JSON error while "
                              "loading scripts: %s", je.what());
                        throw std::runtime_error("Invalid script");
                        
                } catch (std::runtime_error& re) {
                        r_err("ScriptList: Error while converting "
                              "scripts: %s", re.what());
                        throw std::runtime_error("Invalid script");
                }
        }
                
        ScriptList::ScriptList(JsonCpp& json)
        {
                try {
                        
                        convert_scripts(json);
                
                } catch (JSONError& je) {
                        r_err("ScriptList: JSON error while "
                              "loading scripts: %s", je.what());
                        throw std::runtime_error("Invalid script");
                        
                } catch (std::runtime_error& re) {
                        r_err("ScriptList: Error while converting "
                              "scripts: %s", re.what());
                        throw std::runtime_error("Invalid script");
                }
        }
        
        void ScriptList::load_scripts(const char *path)
        {
                JsonCpp scripts = JsonCpp::load(path);
                convert_scripts(scripts);
        }
        
        void ScriptList::convert_scripts(JsonCpp& scripts)
        {
                for (int index = 0; index < scripts.length(); index++) {
                        JsonCpp script = scripts[index];
                        convert_script(script);
                }                        
        }
        
        void ScriptList::convert_script(JsonCpp& script)
        {
                const char *id = script["id"];
                const char *title = script["title"];
                
                push_back(Script(id, title));
                convert_script_actions(back(), script);                        
        }

        void ScriptList::convert_script_actions(Script& new_script,
                                                JsonCpp& json_script)
        {
                JsonCpp actions = json_script.array("actions");
                for (int i = 0; i < actions.length(); i++) {
                        JsonCpp action = actions[i];
                        convert_action(new_script, action);
                }
        }

        void ScriptList::convert_action(Script& new_script, JsonCpp& action)
        {
                const char *type = action["action"];
                if (rstreq(type, "move"))
                        convert_move(new_script, action);
                else if (rstreq(type, "hoe"))
                        convert_hoe(new_script);
        }
        
        void ScriptList::convert_move(Script& new_script, JsonCpp& action)
        {
                double distance = action["distance"];
                double speed = action["speed"];
                
                assure_move_params(distance, speed);
                new_script.actions.push_back(Action(Action::Move, distance, speed));
        }

        void ScriptList::assure_move_params(double distance, double speed)
        {
                if (fabs(distance) > 100.0) {
                        r_err("ScriptList: invalid distance for move: "
                              "%0.2f", distance);
                        throw std::runtime_error("Invalid distance");
                }
                if (fabs(speed) > 1.0) {
                        r_err("ScriptList: invalid speed for move: "
                              "%0.2f", speed);
                        throw std::runtime_error("Invalid speed");
                }
        }
        
        void ScriptList::convert_hoe(Script& new_script)
        {
                new_script.actions.push_back(Action(Action::Hoe));
        }
}
