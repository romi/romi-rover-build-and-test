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
#ifndef __STATE_TRANSITION_H
#define __STATE_TRANSITION_H

#include "IStateTransition.h"

class StateTransition : public IStateTransition
{
protected:
        int8_t _from;
        int16_t _event;
        int8_t _to;

public:
        StateTransition(int8_t from, int16_t event, int8_t to)
                : _from(from), _event(event), _to(to) {}
        virtual ~StateTransition() {}
                
        int16_t event() override {
                return _event;
        }
        
        int8_t state() override {
                return _from;
        }
        
        int8_t nextState() override {
                return _to;
        }
};

#endif // __STATE_TRANSITION_H
