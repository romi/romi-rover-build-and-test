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
#ifndef __ROMI_STATE_MACHINE_H
#define __ROMI_STATE_MACHINE_H

#include <r.h>
#include <vector>
#include "EventsAndStates.h"

namespace romi {

#define STATE_ERROR 0
#define STATE_START 1
#define ALL_STATES -1

        template<class T> using StateTransitionHandler = void (*)(T& target);

        template <class T>
        class StateTransition
        {
        protected:
                int _from;
                int _event;
                StateTransitionHandler<T> _handler;
                int _to;
                
        public:
                StateTransition(int from,
                                int event,
                                StateTransitionHandler<T> handler,
                                int to)
                        : _from(from), _event(event), _handler(handler), _to(to) {}
                
                virtual ~StateTransition() {}
                
                int event() {
                        return _event;
                }
        
                int state() {
                        return _from;
                }
                
                void do_transition(T& ui) {
                        _handler(ui);
                }
        
                int next_state() {
                        return _to;
                }
        };
        
        template <class T>
        class StateMachine
        {
        protected:
                T& _target;
                int _current_state;
                std::vector<StateTransition<T>> _transitions;

                void do_transition(StateTransition<T>& transition) {
                        transition.do_transition(_target);
                        _current_state = transition.next_state();
                }
                
                bool try_regular_transitions(int event) {
                        bool handled = false;
                        for (size_t i = 0; i < _transitions.size(); i++) {
                                StateTransition<T>& transition = _transitions[i];
                                if (transition.state() == _current_state
                                    && transition.event() == event) {
                                        do_transition(transition);
                                        handled = true;
                                        break;
                                }
                        }
                        return handled;
                }
                
                void try_catchall_transitions(int event) {
                        for (size_t i = 0; i < _transitions.size(); i++) {
                                StateTransition<T>& transition = _transitions[i];
                                if (transition.state() == ALL_STATES
                                    && transition.event() == event) {
                                        do_transition(transition);
                                        break;
                                }
                        }
                }

        public:
                StateMachine(T& target)
                        : _target(target),
                          _current_state(STATE_START) {}
                
                virtual ~StateMachine() = default;
                
                int get_state() {
                        return _current_state;
                }
                        
                void handle_event(int event) {
                        r_debug("handle_event %d", event);
                        bool handled = try_regular_transitions(event);
                        if (!handled)
                                try_catchall_transitions(event);
                }

                void add(int from,
                         int event,
                         StateTransitionHandler<T> handler,
                         int to) {
                        _transitions.push_back(StateTransition(from, event, handler, to));
                }
        };
}

#endif // __ROMI_STATE_MACHINE_H
