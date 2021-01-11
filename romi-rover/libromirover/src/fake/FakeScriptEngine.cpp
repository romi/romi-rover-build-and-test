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
#include <r.h>
#include "FakeScriptEngine.h"

namespace romi {
        
        int FakeScriptEngine::count_scripts()
        {
                return (int) _scripts.size();
        }
        
        void FakeScriptEngine::get_script(int index,
                                          std::string& id,
                                          std::string& name)
        {
                if (index >= 0 && index < (int) _scripts.size()) {
                        id = _scripts[index].id;
                        name = _scripts[index].name;
                } else {
                        id = "";
                        name = "";
                }
        }
        
        void FakeScriptEngine::_run_script(FakeScriptEngine *engine,
                                           FakeScript *script)
        {
                engine->run_script(script);
        }
        
        void FakeScriptEngine::run_script(FakeScript *script)
        {
                r_debug("FakeScriptEngine::run_script: %s", script->id.c_str());
                clock_sleep(10.0);
                _send_finished_event = true;                
                r_debug("FakeScriptEngine::run_script: finished %s", script->id.c_str());
        }
        
        int FakeScriptEngine::find_script(std::string& id)
        {
                int index = -1;
                for (size_t i = 0; i < _scripts.size(); i++) {
                        if (id.compare(_scripts[i].id) == 0) {
                                index = (int) i;
                                break;
                        }
                }
                return index;
        }

        void FakeScriptEngine::execute_script(std::string& id)
        {
                r_debug("FakeScriptEngine::execute_script: %s", id.c_str());

                int index = find_script(id);
                _send_finished_event = false;
                
                if (index >= 0) {
                        std::thread t(_run_script, this, &_scripts[index]); 
                        t.detach();
                } else {
                        _send_finished_event = true;
                }
        }

        void FakeScriptEngine::add_script(const char *id, const char *name)
        {
                _scripts.push_back(FakeScript(id, name));
        }

        int FakeScriptEngine::get_next_event()
        {
                int event = 0;
                if (_send_finished_event) {
                        event = _finished_event;
                        _send_finished_event = false;
                }
                return event;
        }
}
