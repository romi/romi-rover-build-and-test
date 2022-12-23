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

#include "Log.h"
#include "RomiSerial.h"
#include "RomiSerialErrors.h"
#include "Printer.h"
#include "RomiSerialUtil.h"
#include <stdio.h>

namespace romiserial {

        RomiSerial::RomiSerial(IInputStream& in, IOutputStream& out)
                : in_(in), out_(out),
                  handlers_(nullptr),
                  num_handlers_(0),
                  envelope_parser_(),
                  message_parser_(),
                  sent_response_(false),
                  crc_(),
                  last_id_(255)
        {
        }

        RomiSerial::RomiSerial(IInputStream& in, IOutputStream& out,
                               const MessageHandler *handlers, uint8_t num_handlers)
                : RomiSerial(in, out) {
                set_handlers(handlers, num_handlers);
        }

        void RomiSerial::set_handlers(const MessageHandler *handlers, uint8_t num_handlers)
        {
                handlers_ = handlers;
                num_handlers_ = num_handlers;
        }

        void RomiSerial::handle_input()
        {
                char c;
                while (in_.available()) {
                        if (in_.read(c)) {
                                handle_char(c);
                        }
                }
        }

        void RomiSerial::handle_char(char c)
        {
                bool has_message = envelope_parser_.process(c);
                if (has_message) {
                        process_message();
                
                } else if (envelope_parser_.error() != 0) {
                        send_error(envelope_parser_.error(), nullptr);
                }
        }

        void RomiSerial::process_message()
        {
                if (envelope_parser_.has_id()) {
                        if (envelope_parser_.id() == last_id_) {
                                // This message was already handled.
                                send_error(kDuplicate, nullptr);
                        } else {
                                parse_and_handle_message();
                        }
                } else {
                        // The incoming message has no ID. This happens for
                        // messages sent on the command line that terminate
                        // with :xxxx
                        parse_and_handle_message();
                }
        }

        void RomiSerial::parse_and_handle_message()
        {
                bool ok = message_parser_.parse(envelope_parser_.message(),
                                                envelope_parser_.length());
                if (ok) {
                        handle_message();

                } else {
                        send_error(message_parser_.error(), nullptr);
                }
        }

        void RomiSerial::handle_message()
        {
                int index = get_handler();

                if (index < 0) {
                        send_error(kUnknownOpcode, nullptr);

                } else if (assert_valid_arguments(index)) {
                        sent_response_ = false;
                        handlers_[index].callback(this,
                                                  message_parser_.values(),
                                                  message_parser_.string());
                        if (!sent_response_) {
                                send_error(kBadHandler, nullptr);
                        }
                }
        }

        int RomiSerial::get_handler()
        {
                int index = -1;
                for (int i = 0; i < num_handlers_; i++) {
                        if (message_parser_.opcode() == handlers_[i].opcode) {
                                index = i;
                                break;
                        }
                }
                return index;
        }

        bool RomiSerial::assert_valid_arguments(int index)
        {
                return (assert_valid_argument_count(index)
                        && assert_valid_string_argument(index));
        }

        bool RomiSerial::assert_valid_argument_count(int index)
        {
                bool retval = false;
                if (message_parser_.length() == handlers_[index].number_arguments) {
                        retval = true;
                } else {
                        send_error(kBadNumberOfArguments, nullptr);
                }
                return retval;
        }

        bool RomiSerial::assert_valid_string_argument(int index)
        {
                bool retval = false;
                if (message_parser_.has_string() == handlers_[index].requires_string) {
                        retval = true;
                } else {
                        if (handlers_[index].requires_string)
                                send_error(kMissingString, nullptr);
                        else
                                send_error(kBadString, nullptr);
                }
                return retval;
        }

        void RomiSerial::append_start_metadata()
        {        
                crc_.update(':');
                out_.write(':');
        }

        void RomiSerial::append_char(char c)
        {
                if (c == ':')
                        c = '-';
                crc_.update(c);
                out_.write(c);
        }

        void RomiSerial::append_message(const char *s)
        {
                while (*s != '\0')
                        append_char(*s++);
        }

        void RomiSerial::start_message()
        {
                crc_.start();
                append_char('#');
                if (message_parser_.opcode()) 
                        append_char(message_parser_.opcode());
                else
                        append_char('_');
        }

        void RomiSerial::finalize_message()
        {
                append_start_metadata();
                append_id();
                append_crc();
                append_char('\r');
                append_char('\n');
        }

        void RomiSerial::send_error(int code, const char *message)
        {
                char buffer[128];
                if (message)
                        snprintf(buffer, sizeof(buffer), "[%d,\"%s\"]", code, message);
                else
                        snprintf(buffer, sizeof(buffer), "[%d]", code);
                send_message(buffer);
                sent_response_ = true;
        }

        void RomiSerial::send_ok()
        {
                send_message("[0]");
                sent_response_ = true;
                last_id_ = envelope_parser_.id();
        }

        void RomiSerial::send(const char *message)
        {
                send_message(message);
                sent_response_ = true;
                last_id_ = envelope_parser_.id();
        }

        void RomiSerial::send_message(const char *message)
        {
                start_message();
                append_message(message);
                finalize_message();
        }

        void RomiSerial::append_hex(uint8_t value)
        {
                append_char(to_hex((uint8_t)(value >> 4)));
                append_char(to_hex(value));
        }

        void RomiSerial::append_id()
        {
                append_hex(envelope_parser_.id());
        }

        void RomiSerial::append_crc()
        {
                append_hex(crc_.get());
        }

        void RomiSerial::log(const char *message)
        {
                Printer printer(out_);
                printer.print("#!");
                printer.print(message);
                printer.print(":xxxx\r\n");
        }
}
