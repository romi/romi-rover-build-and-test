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
#include <stdint.h>

#ifndef _OQUAM_TRIGGER_H_
#define _OQUAM_TRIGGER_H_

/**
 * \brief Initializes the trigger buffer.
 */
void init_trigger_buffer();

/**
 * \brief Adds a trigger to the send list.
 *
 * If the buffer is full, the trigger will be discarded and -1 will be
 * returned. Otherwise, the id is inserted at the end of the buffer
 * and the function returns zero. 
 */
int trigger_buffer_put(int16_t id);

/**
 * \brief Returns the number of triggers waiting to be send.
 */
uint8_t trigger_buffer_available();

/**
 * \brief Returns the next id of the trigger to be send.
 *
 * You MUST verify that there is data in the buffer by calling
 * trigger_buffer_available() first.
 */
int16_t trigger_buffer_get();


/**
 * \brief Removes all the triggers.
 */
void trigger_buffer_clear();

#endif // _OQUAM_TRIGGER_H_
