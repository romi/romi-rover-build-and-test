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
#ifndef __STATE_MACHINE_TIMER_H
#define __STATE_MACHINE_TIMER_H

#include "IStateMachineTimer.h"
#include "IArduino.h"

class StateMachineTimer : public IStateMachineTimer
{
protected:
        IStateMachine *_stateMachine;
        IArduino *_arduino;
        int _enabled;
        int _event;
        unsigned long _timeout;

public:
        StateMachineTimer(IStateMachine *stateMachine,
                          IArduino *arduino)
                : _stateMachine(stateMachine),
                  _arduino(arduino),
                  _enabled(0), _event(0), _timeout(0) {}

        void setTimeout(int milliseconds, int event) override;
        void update(unsigned long t) override;
};

#endif // __STATE_MACHINE_TIMER_H
