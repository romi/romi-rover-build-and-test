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

int StateMachine::getState()
{
        return _currentState;
}

int StateMachine::getError()
{
        return _error;
}

void StateMachine::add(IStateTransition *transition)
{
        // FIXME: what happens if _length >= MAX_TRANSITIONS ?!
        if (transition != 0 && _length < MAX_TRANSITIONS) {
                _transitions[_length++] = transition;
        } 
}

int StateMachine::handleEvent(int event)
{
        int r = Ignored;
        for (int i = 0; i < _length; i++) {
                IStateTransition *t = _transitions[i];
                if (t->state() == _currentState
                    && t->event() == event) {
                        int err = t->doTransition();
                        if (err == 0) {
                                _currentState = t->nextState();
                                r = OK;
                        } else {
                                _currentState = STATE_ERROR;
                                _error = err;
                                r = Error;
                        }
                        break;
                }
        }
        return r;
}
