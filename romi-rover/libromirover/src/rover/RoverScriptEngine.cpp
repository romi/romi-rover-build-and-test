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
#include "Activity.h"

namespace romi {
                
        RoverScriptEngine::RoverScriptEngine(ScriptList& scripts,
                                             int finished_event,
                                             int error_event)
                : _scripts(scripts),
                  _finished_event(finished_event),
                  _error_event(error_event),
                  _result(0)
        {}
        
        void RoverScriptEngine::_run_script(RoverScriptEngine *engine,
                                            RoverAndScriptIndex args)
        {
                engine->run_script(args.rover, args.script_index);
        }
        
        void RoverScriptEngine::run_script(Rover* rover, int script_index)
        {
                int result_event = _finished_event;
                Script& script = _scripts[script_index];
                
                r_debug("RoverScriptEngine::run_script: '%s'", script.title.c_str());

                try {
                        result_event = try_run_script(rover, script);
                        
                } catch (ActivityResetException& reset_exception) {
                        r_warn("RoverScriptEngine::run_script: reset exception");
                        rover->reset();
                }
                
                set_result(result_event);
                
                r_debug("RoverScriptEngine::run_script: finished '%s'",
                        script.title.c_str());
        }
                
        int RoverScriptEngine::try_run_script(Rover* rover, Script& script)
        {
                int result_event = _finished_event;
                for (size_t i = 0; i < script.actions.size(); i++) {
                        
                        bool success = execute_action(rover, script.actions[i]);
                        if (!success) {
                                result_event = _error_event;
                                break;
                        }
                }
                return result_event;
        }
                
        bool RoverScriptEngine::execute_action(Rover* rover, Action& action)
        {
                bool success = false;
                if (action.type == Action::Move)
                        success = execute_move(rover, action.params[0], action.params[1]);
                else if (action.type == Action::Hoe)
                        success = execute_hoe(rover);
                return success;
        }
                
        bool RoverScriptEngine::execute_move(Rover* rover, double distance, double speed)
        {
                bool success = rover->navigation.move(distance, speed);
                if (!success)
                        r_err("RoverScriptEngine: 'move' failed");
                return success;
        }
                
        bool RoverScriptEngine::execute_hoe(Rover* rover)
        {
                bool success = rover->weeder.hoe();
                if (!success)
                        r_err("RoverScriptEngine: 'hoe' failed");
                return success;
        }

        void RoverScriptEngine::execute_script(Rover& rover, int id)
        {
                r_debug("RoverScriptEngine::execute_script: '%d'", id);
                
                RoverAndScriptIndex args = { &rover, id };
                clear_result();
                std::thread t(_run_script, this, args); 
                t.detach();
        }

        void RoverScriptEngine::clear_result()
        {
                SynchronizedCodeBlock sync(_mutex);
                _result = 0;
        }

        void RoverScriptEngine::set_result(int event)
        {
                SynchronizedCodeBlock sync(_mutex);
                _result = event;
        }

        int RoverScriptEngine::get_result()
        {
                SynchronizedCodeBlock sync(_mutex);
                int result = _result;
                _result = 0;
                return result;
        }

        int RoverScriptEngine::get_next_event()
        {
                return get_result();
        }
}
