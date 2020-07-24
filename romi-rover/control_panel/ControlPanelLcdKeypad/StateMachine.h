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

#include "IStateMachine.h"

#define MAX_TRANSITIONS 64

class StateMachine : public IStateMachine
{
protected:
        int _currentState;
        int _error;
        IStateTransition *_transitions[MAX_TRANSITIONS];
        int _length;
        
public:
        
        StateMachine() : _currentState(0), _error(0), _length(0) {}
        virtual ~StateMachine() {}

        int getState() override;
        void setError(int error) override;
        int getError() override;
        int countTransitions() override;
        void add(IStateTransition *transition) override;
        int handleEvent(int event) override;
};

#endif // __STATE_MACHINE_H
