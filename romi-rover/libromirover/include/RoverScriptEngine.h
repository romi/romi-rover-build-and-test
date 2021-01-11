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
#ifndef _ROMI_ROVER_SCRIPT_ENGINE_H
#define _ROMI_ROVER_SCRIPT_ENGINE_H

#include <mutex>
#include "ScriptEngine.h"
#include "ScriptList.h"
#include "Rover.h"

namespace romi {
        
        using SynchronizedCodeBlock = std::lock_guard<std::mutex>;

        struct RoverAndScriptIndex {
                Rover *rover;
                int script_index;
        };
        
        class RoverScriptEngine : public ScriptEngine<Rover>
        {
        protected:

                ScriptList& _scripts;
                int _finished_event;
                int _error_event;
                int _result;
                std::mutex _mutex;
                        
                static void _run_script(RoverScriptEngine *engine,
                                        RoverAndScriptIndex args);
                
                void run_script(Rover* rover, int script_index);
                int try_run_script(Rover* rover, Script& script);
                bool execute_action(Rover* rover, Action& action);
                bool execute_move(Rover* rover, double distance, double speed);
                bool execute_hoe(Rover* rover);

                void clear_result();
                void set_result(int event);
                int get_result();
                
        public:
                
                RoverScriptEngine(ScriptList& scripts,
                                  int finished_event,
                                  int error_event);
                virtual ~RoverScriptEngine() override = default;

                void execute_script(Rover& target, int id) override;
                int get_next_event() override;                
        };
}

#endif // _ROMI_ROVER_SCRIPT_ENGINE_H
