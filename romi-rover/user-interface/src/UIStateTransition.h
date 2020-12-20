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
#ifndef __ROMI_UI_STATE_TRANSITION_H
#define __ROMI_UI_STATE_TRANSITION_H

namespace romi {

        class UserInterface;
        
        using StateTransitionHandler = void (*)(UserInterface& ui);
        
        class UIStateTransition
        {
        protected:
                int _from;
                int _event;
                int _to;
                StateTransitionHandler _handler;
                
        public:
                UIStateTransition(int from, int event, int to,
                                  StateTransitionHandler handler)
                        : _from(from), _event(event), _to(to),
                          _handler(handler) {}
                
                virtual ~UIStateTransition() {}
                
                int event() {
                        return _event;
                }
        
                int state() {
                        return _from;
                }
                
                void do_transition(UserInterface& ui) {
                        _handler(ui);
                }
        
                int next_state() {
                        return _to;
                }
        };
}

#endif // __ROMI_UI_STATE_TRANSITION_H
