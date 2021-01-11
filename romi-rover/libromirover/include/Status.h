/*
  romi-rover

  Copyright (C) 2019 Sony Computer Science Laboratories
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
#ifndef _ROMI_STATUS_H_
#define _ROMI_STATUS_H_

namespace romi {
        
        class Status
        {
        public:
                virtual ~Status() = default;

                virtual bool is_ready() = 0;
                virtual bool is_busy() = 0;
                virtual bool is_failing() = 0;
                
                virtual bool has_message() = 0;
                virtual const char *get_message() = 0;
                virtual bool has_progess() = 0;
                virtual double get_progress() = 0;
        };
}

#endif // _ROMI_STATUS_H_
