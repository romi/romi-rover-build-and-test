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
#include "StateMachine.h"

namespace romi {

        int StateMachine::doTransition(IStateTransition &transition, unsigned long t)
        {
                transition.doTransition(_interface, t);
                _currentState = transition.next_state();
                //r_debug("new-state=%d", _currentState);
                return OK;
        }
        
        int StateMachine::tryTransition(int16_t event, unsigned long t)
        {
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
        
        int StateMachine::tryCatchAllTransition(int16_t event, unsigned long t)
        {
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

        int StateMachine::getState()
        {
                return _currentState;
        }

        void StateMachine::add(int from, int event, int to,
                               StateTransitionHandler handler)
        {
                _transitions.push_back(StateTransition(from, event, to, handler));
        }

        int StateMachine::handleEvent(int16_t event, unsigned long t)
        {
                int r = tryTransition(event, t);
                if (r == Ignored)
                        r = tryCatchAllTransition(event, t);
                return r;
        }
}
