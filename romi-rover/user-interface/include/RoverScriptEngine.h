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

#include "ScriptEngine.h"
#include "ScriptList.h"
#include "Rover.h"

namespace romi {

        class RoverScriptEngine : public ScriptEngine<Rover>
        {
        protected:

                ScriptList& _scripts;
                int _finished_event;
                int _error_event;
                bool _send_event;
                int _event;
                
                int _script_index;
                Rover *_rover;
                        
                static void _run_script(RoverScriptEngine *engine);
                
                void run_script();
                bool execute_action(Action& action);
                bool execute_move(double distance, double speed);
                bool execute_hoe();
                
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
