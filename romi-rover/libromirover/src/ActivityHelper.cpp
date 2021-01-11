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
#include <condition_variable>
#include <mutex>
#include <r.h>
#include "ActivityHelper.h" 

namespace romi {
        
        void ActivityHelper::check_pause_or_cancel_execution()
        {
                std::unique_lock<std::mutex> lock(_mutex);
                if (_state == Paused) {
                        r_debug("ActivityHelper::check: condition.wait");
                        _continue_condition.wait(lock,
                                                 [this]{ return this->_state != Paused; });
                }
                if (_state == Reset) {
                        _state = Running;
                        r_debug("ActivityHelper::check: throw ActivityResetException");
                        throw ActivityResetException();
                }
        }
        
        bool ActivityHelper::pause_activity()
        {
                r_debug("ActivityHelper::pause_activity: enter");
                _state = Paused;
                r_debug("ActivityHelper::pause_activity: leave");
                return true;
        }
        
        bool ActivityHelper::continue_activity()
        {
                r_debug("ActivityHelper::continue_activity: enter");
                _state = Running;
                r_debug("ActivityHelper::continue_activity: condition.notify");
                _continue_condition.notify_one();
                r_debug("ActivityHelper::continue_activity: leave");
                return true;
        }
        
        bool ActivityHelper::reset_activity()
        {
                r_debug("ActivityHelper::reset_activity: enter");
                _state = Reset;
                r_debug("ActivityHelper::reset_activity: condition.notify");
                _continue_condition.notify_one();
                r_debug("ActivityHelper::reset_activity: leave");
                return true;
        }
}
