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
#ifndef __ROMISERIAL_IROMISERIAL_H
#define __ROMISERIAL_IROMISERIAL_H

#include <stdint.h>

namespace romiserial {

        class IRomiSerial;
        
        typedef void (*MessageCallback)(IRomiSerial *romi_serial,
                                        int16_t *args,
                                        const char *string_arg);

        struct MessageHandler 
        {
                char opcode;
                uint8_t number_arguments;
                bool requires_string;
                MessageCallback callback;
        };

        class IRomiSerial
        {
        public:
                virtual ~IRomiSerial() = default;

                virtual void set_handlers(const MessageHandler *handlers,
                                          uint8_t num_handlers) = 0;
                virtual void handle_input() = 0;
                virtual void send_ok() = 0;
                virtual void send_error(int code, const char *message) = 0;
                virtual void send(const char *message) = 0;
                /* virtual bool read(uint8_t *data, size_t length) = 0; */
                /* virtual bool write(const uint8_t *data, size_t length) = 0; */
                virtual void log(const char *message) = 0;
        };
}

#endif // __ROMISERIAL_IROMISERIAL_H
