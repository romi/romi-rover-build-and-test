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
#include "trigger.h"

#define TRIGGER_BUFFER_SIZE 4
#define TRIGGER_BUFFER_SIZE_MASK 0x03

/**
 * \brief The 'trigger_buffer' is a circular buffer of trigger IDs.
 */
typedef struct _trigger_buffer_t {
        uint8_t readpos;
        uint8_t writepos;
        int16_t trigger[TRIGGER_BUFFER_SIZE];
} trigger_buffer_t;

static trigger_buffer_t trigger_buffer;

void init_trigger_buffer()
{
        trigger_buffer.readpos = 0;
        trigger_buffer.writepos = 0;
}

int trigger_buffer_put(int16_t id)
{
        /* This function is called in the stepper timer's ISR so it
         * has to be quick. It should not get disturbed by any other
         * code (no other ISR accesses the trigger buffer). */        
        uint8_t n = ((TRIGGER_BUFFER_SIZE
                      - trigger_buffer.writepos - 1
                      + trigger_buffer.readpos)
                     & TRIGGER_BUFFER_SIZE_MASK);
        
        if (n == 0)
                return -1;
        
        trigger_buffer.trigger[trigger_buffer.writepos] = id;
        
        trigger_buffer.writepos = ((trigger_buffer.writepos + 1)
                                   & TRIGGER_BUFFER_SIZE_MASK);
        
        return 0;
}

static inline uint8_t _available()
{
        return ((TRIGGER_BUFFER_SIZE
                 - trigger_buffer.readpos
                 + trigger_buffer.writepos)
                & TRIGGER_BUFFER_SIZE_MASK);
}

uint8_t trigger_buffer_available()
{
        return _available();
}

int16_t trigger_buffer_get()
{
        /* The execution of this function may be interrupted by the
         * stepper timer timer. Take some care. */
        
        int16_t id = trigger_buffer.trigger[trigger_buffer.readpos];
        
        trigger_buffer.readpos = ((trigger_buffer.readpos + 1)
                                  & TRIGGER_BUFFER_SIZE_MASK);
        return id;
}

void trigger_buffer_clear()
{
        trigger_buffer.readpos = trigger_buffer.writepos;
}
