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
#ifndef __I_ROMI_SERIAL_CLIENT_H
#define __I_ROMI_SERIAL_CLIENT_H

#include "JSON.h"

class IRomiSerialClient
{
public:
        virtual ~IRomiSerialClient() = default;

        /**
         *  Sends a request to the firmware and retrieves its
         *  response.
         *
         *  command: the string representation of the request.
         *
         *  Returns: An JSON array. The first element of the array is
         *  a number that indicates whether the request was
         *  successfully handled or not. A value of zero means
         *  success. If the first value is zero then the remaining
         *  values of the array are the values returned by the
         *  firmware to the client. If the firmware does not return
         *  any values, the length of the array will be one. If the
         *  first value is not zero then the second element of the
         *  array is a string with a human-readable error message.
         */
        virtual void send(const char *request, JSON &response) = 0;
};

#endif
