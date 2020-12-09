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

namespace romi {

#define MAX_TRANSITIONS 32

        class StateMachine : public IStateMachine
        {
        protected:
                int _currentState;
                std::vector<StateTransition> _transitions;

                int doTransition(IStateTransition &transition, unsigned long t) {
                        _currentState = transition.doTransition(t);
                        //r_debug("new-state=%d", _currentState);
                        return OK;
                }
        
                int tryTransition(int16_t event, unsigned long t) {
                        int r = Ignored;
                        for (size_t i = 0; i < _transitions.size(); i++) {
                                StateTransition &transition = _transitions[i];
                                
                                // r_debug("state(cur=%d,trans=%d), "
                                //         "event=(cur=%d,trans=%d)",
                                //         _currentState, transition.state(), 
                                //         event, transition.event());
                                
                                if (transition.state() == _currentState
                                    && transition.event() == event) {
                                        r = doTransition(transition, t);
                                        break;
                                }
                        }
                        return r;
                }
        
                int tryCatchAllTransition(int16_t event, unsigned long t) {
                        int r = Ignored;
                        for (size_t i = 0; i < _transitions.size(); i++) {
                                StateTransition &transition = _transitions[i];
                                if (transition.state() == ALL_STATES
                                    && transition.event() == event) {
                                        r = doTransition(transition, t);
                                        break;
                                }
                        }
                        return r;
                }
        
        public:
                
                StateMachine() : _currentState(STATE_START) {}
                virtual ~StateMachine() {}

                int getState() override {
                        return _currentState;
                }

                void add(int from, int event, int to, StateTransitionHandler &handler) {
                        _transitions.push_back(StateTransition(from, event, to, handler));
                }

                int handleEvent(int16_t event, unsigned long t) override {
                        int r = tryTransition(event, t);
                        if (r == Ignored)
                                r = tryCatchAllTransition(event, t);
                        return r;
                }

        };
}

#endif // __STATE_MACHINE_H
