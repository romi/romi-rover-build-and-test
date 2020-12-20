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
#include "UIStateMachine.h"

namespace romi {

        void UIStateMachine::do_transition(UIStateTransition &transition)
        {
                transition.do_transition(_interface);
                _currentState = transition.next_state();
        }
        
        bool UIStateMachine::try_regular_transitions(int event)
        {
                bool handled = false;
                for (size_t i = 0; i < _transitions.size(); i++) {
                        UIStateTransition &transition = _transitions[i];
                        
                        if (transition.state() == _currentState
                            && transition.event() == event) {
                                do_transition(transition);
                                handled = true;
                                break;
                        }
                }
                return handled;
        }
        
        void UIStateMachine::try_catchall_transitions(int event)
        {
                for (size_t i = 0; i < _transitions.size(); i++) {
                        UIStateTransition &transition = _transitions[i];
                        if (transition.state() == ALL_STATES
                            && transition.event() == event) {
                                do_transition(transition);
                                break;
                        }
                }
        }

        int UIStateMachine::get_state()
        {
                return _currentState;
        }

        void UIStateMachine::add(int from, int event, int to,
                                 StateTransitionHandler handler)
        {
                _transitions.push_back(UIStateTransition(from, event, to, handler));
        }

        void UIStateMachine::handle_event(int event)
        {
                bool handled = try_regular_transitions(event);
                if (!handled)
                        try_catchall_transitions(event);
        }
}
