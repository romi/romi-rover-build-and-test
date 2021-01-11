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
#ifndef __ROMI_ACTIVITY_HELPER_H
#define __ROMI_ACTIVITY_HELPER_H

#include <atomic>
#include <condition_variable>
#include <mutex>
#include "Activity.h" 

namespace romi {


        class ActivityHelper
        {
        protected:
                
                enum ActivityState { Running, Paused, Reset };
                
                std::mutex _mutex;
                std::condition_variable _continue_condition;
                std::atomic<ActivityState> _state;

        public:
                ActivityHelper() : _state(Running) {}
                virtual ~ActivityHelper() = default;

                void check_pause_or_cancel_execution();
                
                bool pause_activity();
                bool continue_activity();
                bool reset_activity();
        };
}

#endif // __ROMI_ACTIVITY_HELPER_H
