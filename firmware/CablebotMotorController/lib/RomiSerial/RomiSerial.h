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
#ifndef __ROMISERIAL_ROMISERIAL_H
#define __ROMISERIAL_ROMISERIAL_H

#include "IRomiSerial.h"
#include "IInputStream.h"
#include "IOutputStream.h"
#include "EnvelopeParser.h"
#include "MessageParser.h"
#include "CRC8.h"

namespace romiserial {

        class RomiSerial : public IRomiSerial
        {
        protected:
                IInputStream& in_;
                IOutputStream& out_;
                const MessageHandler *handlers_;
                uint8_t num_handlers_;
                EnvelopeParser envelope_parser_;
                MessageParser message_parser_;
                bool sent_response_;
                CRC8 crc_;
                uint8_t last_id_;
        
                void process_message();
                void handle_char(char c);
                void parse_and_handle_message();
                void handle_message();
                int get_handler();
                bool assert_valid_arguments(int index);
                bool assert_valid_argument_count(int index);
                bool assert_valid_string_argument(int index);

                void start_message();
                void append_char(char c);
                void append_message(const char *s);
                void finalize_message();
                void send_message(const char*message);

        public:

                RomiSerial(IInputStream& in, IOutputStream& out);
                RomiSerial(IInputStream& in, IOutputStream& out,
                           const MessageHandler *handlers, uint8_t num_handlers);
                RomiSerial(const RomiSerial&) = delete;
                RomiSerial& operator=(const RomiSerial&) = delete;

                ~RomiSerial() override = default;

                void set_handlers(const MessageHandler *handlers,
                                  uint8_t num_handlers) override;
                void handle_input() override;
                void send_ok() override;
                void send_error(int code, const char *message) override;
                void send(const char *message) override;
                void log(const char *message) override;
        
        protected:

                void append_hex(uint8_t value);
                void append_start_metadata();
                void append_id();
                void append_crc();

        };
}

#endif
