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

class StateTransition
{
protected:
        int _event;
        int _from;
        int _to;

public:
        StateTransition(int from, int event, int to)
                : _from(from), _event(event), _to(to) {}
        
        virtual int doTransition() = 0;

        int event() { return _event; }
        int state() { return _from; }
        int nextState() { return _to; }
};

#define MAX_TRANSITIONS 64

class StateMachine
{
public:
        enum { ErrorState = -1, StartState = 0 };
        enum { OK = 0, Ignored = 1, Error = 2 };
        
protected:
        int _currentState;
        int _error;
        StateTransition *_transitions[MAX_TRANSITIONS];
        int _length;
        
public:
        
        StateMachine() : _currentState(0), _error(0), _length(0) {}

        int getState() {
                return _currentState;
        }

        void setError(int error) {
                _error = error;
        }

        int getError() {
                return _error;
        }

        int countTransitions() {
                return _length;
        }

        void add(StateTransition *transition) {
                if (transition != 0 && _length < MAX_TRANSITIONS) {
                        _transitions[_length++] = transition;
                }
        }

        int handleEvent(int event) {
                int r = Ignored;
                for (int i = 0; i < _length; i++) {
                        StateTransition *t = _transitions[i];
                        if (t->state() == _currentState
                            && t->event() == event) {
                                int err = t->doTransition();
                                if (err == 0) {
                                        _currentState = t->nextState();
                                        r = OK;
                                } else {
                                        _currentState = ErrorState;
                                        _error = err;
                                        r = Error;
                                }
                                break;
                        }
                }
                return r;
        }
};

class ErrorTransition : public StateTransition
{
protected:
        int _error;
public:
        ErrorTransition(int from, int event, int error)
                : StateTransition(from, event, StateMachine::ErrorState),
                _error(error) {}
        virtual int doTransition() { return _error; };
};

#endif // __STATE_MACHINE_H
