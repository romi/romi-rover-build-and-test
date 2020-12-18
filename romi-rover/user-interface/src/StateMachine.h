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
#ifndef __STATE_MACHINE_H
#define __STATE_MACHINE_H

#include <vector>
#include "IStateMachine.h"
#include "StateTransition.h"
#include "UserInterface.h"

namespace romi {

        class StateMachine : public IStateMachine
        {
        protected:
                UserInterface& _interface;
                int _currentState;
                std::vector<StateTransition> _transitions;

                int doTransition(IStateTransition &transition, unsigned long t);
                int tryTransition(int16_t event, unsigned long t);
                int tryCatchAllTransition(int16_t event, unsigned long t);
        
        public:
                
                StateMachine(UserInterface& interface)
                        : _interface(interface), _currentState(STATE_START) {}
                virtual ~StateMachine() {}

                int getState() override;
                int handleEvent(int16_t event, unsigned long t) override;
                
                void add(int from, int event, int to, StateTransitionHandler handler);
        };
}

#endif // __STATE_MACHINE_H
