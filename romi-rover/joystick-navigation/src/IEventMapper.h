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
#ifndef __ROMI_I_EVENT_MAPPER_H
#define __ROMI_I_EVENT_MAPPER_H

#include <stdint.h>
#include "IJoystick.h"

namespace romi {

        class IEventMapper
        {
        public:
                virtual ~IEventMapper() = default;
                virtual int16_t map(JoystickEvent &event) = 0;
        };
}

#endif // __ROMI_I_EVENT_MAPPER_H
