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
#ifndef __EVENT_TIMER_H
#define __EVENT_TIMER_H

#include "IEventTimer.h"

class EventTimer : public IEventTimer
{
protected:
        int _enabled;
        int _event;
        unsigned long _timeout;

public:
        EventTimer() : _enabled(0), _event(0), _timeout(0) {}

        void setTimeout(unsigned long milliseconds, int event) override {
                _enabled = 1;
                _timeout = milliseconds;
                _event = event;
        }
        
        int16_t update(unsigned long t) override {
                int r = -1;
                if (_enabled && t > _timeout) {
                        r = _event;
                        _enabled = 0;
                }
                return r;
        }
};

#endif // __EVENT_TIMER_H
