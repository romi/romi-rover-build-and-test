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
#ifndef __ROMI_STATE_TRANSITION_H
#define __ROMI_STATE_TRANSITION_H

#include "IStateTransition.h"
#include "UserInterface.h"

namespace romi {

        using StateTransitionHandler = void (*)(UserInterface& ui, unsigned long t);
        
        class StateTransition : public IStateTransition
        {
        protected:
                int8_t _from;
                int16_t _event;
                int8_t _to;
                StateTransitionHandler _handler;
                
        public:
                StateTransition(int8_t from, int16_t event, int8_t to,
                        StateTransitionHandler handler)
                        : _from(from), _event(event), _to(to),
                          _handler(handler) {}
                
                virtual ~StateTransition() {}
                
                int16_t event() override {
                        return _event;
                }
        
                int8_t state() override {
                        return _from;
                }
                
                void doTransition(UserInterface& ui, unsigned long t) override {
                        _handler(ui, t);
                }
        
                int8_t next_state() override {
                        return _to;
                }
        };
}

#endif // __ROMI_STATE_TRANSITION_H
