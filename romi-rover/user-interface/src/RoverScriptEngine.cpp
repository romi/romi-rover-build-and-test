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
#include "RoverScriptEngine.h"
#include "UserInterface.h"

namespace romi {
                
        RoverScriptEngine::RoverScriptEngine(ScriptList& scripts,
                                             int finished_event,
                                             int error_event)
                : _scripts(scripts),
                  _finished_event(finished_event),
                  _error_event(error_event),
                  _send_event(false),
                  _event(0),
                  _script_index(0),
                  _rover(0)
        {}
        
        void RoverScriptEngine::_run_script(RoverScriptEngine *engine)
        {
                engine->run_script();
        }
        
        void RoverScriptEngine::run_script()
        {
                bool success = false;
                Script& script = _scripts[_script_index];
                
                r_debug("RoverScriptEngine::run_script: '%s'", script.title.c_str());

                for (size_t i = 0; i < script.actions.size(); i++) {

                        success = execute_action(script.actions[i]);
                        if (!success)
                                break;
                }
                
                _event = (success)? _finished_event : _error_event;
                _send_event = true;
                
                r_debug("RoverScriptEngine::run_script: finished '%s'", script.title.c_str());
        }
                
        bool RoverScriptEngine::execute_action(Action& action)
        {
                bool success = false;
                if (action.type == Action::Move)
                        success = execute_move(action.params[0], action.params[1]);
                else if (action.type == Action::Hoe)
                        success = execute_hoe();
                return success;
        }
                
        bool RoverScriptEngine::execute_move(double distance, double speed)
        {
                return _rover->navigation.move(distance, speed);
        }
                
        bool RoverScriptEngine::execute_hoe()
        {
                return _rover->weeder.hoe();
        }

        void RoverScriptEngine::execute_script(Rover& rover, int id)
        {
                r_debug("RoverScriptEngine::execute_script: '%d'", id);

                _rover = &rover;
                _script_index = id;
                _send_event = false;
                
                std::thread t(_run_script, this); 
                t.detach();
        }

        int RoverScriptEngine::get_next_event()
        {
                int event = 0;
                if (_send_event) {
                        event = _event;
                        _send_event = false;
                }
                return event;
        }
}
