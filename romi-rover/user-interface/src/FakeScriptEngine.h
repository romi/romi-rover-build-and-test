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
#ifndef _ROMI_FAKE_SCRIPT_ENGINE_H
#define _ROMI_FAKE_SCRIPT_ENGINE_H

#include <string>
#include <vector>
#include "ScriptEngine.h"

namespace romi {

        struct FakeScript
        {
                std::string id;
                std::string name;

                FakeScript(const char *id_, const char *name_) {
                        id = id_;
                        name = name_;
                }
        };
                        
        class FakeScriptEngine : public ScriptEngine
        {
        protected:
                std::vector<FakeScript> _scripts;
                int _finished_event;
                bool _send_finished_event;

                static void _run_script(FakeScriptEngine *engine, FakeScript *script);
                
                void run_script(FakeScript *script);
                int find_script(std::string& id);

        public:
                FakeScriptEngine(int event)
                        : _finished_event(event),
                          _send_finished_event(false) {}
                
                virtual ~FakeScriptEngine() override = default;

                int count_scripts() override;
                void get_script(int index,
                                std::string& id,
                                std::string& name) override;
                void execute_script(std::string& id) override;

                int get_next_event() override;

                void add_script(const char *id, const char *name);
        };
}

#endif // _ROMI_FAKE_SCRIPT_ENGINE_H
