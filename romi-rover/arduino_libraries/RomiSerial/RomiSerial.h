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
#ifndef __ROMI_SERIAL_H
#define __ROMI_SERIAL_H

#include "IRomiSerial.h"
#include "IInputStream.h"
#include "IOutputStream.h"
#include "Parser.h"
#include "CRC8.h"

class RomiSerial;

typedef void (*MessageCallback)(RomiSerial *romi_serial,
                                int16_t *args,
                                const char *string_arg);

struct MessageHandler 
{
        uint8_t opcode;
        uint8_t number_arguments;
        bool requires_string;
        MessageCallback callback;
};

class RomiSerial : public IRomiSerial
{
protected:
        IInputStream *_in;
        IOutputStream *_out;
        const MessageHandler *_handlers;
        uint8_t _num_handlers;
        Parser _parser;
        bool _sent_response;
        CRC8 _crc;
        uint8_t _last_id;
        
        void handle_message();

        void start_message();
        void append_char(const char c);
        void append_message(const char *s);
        void finalize_message();

public:
        
        RomiSerial(const MessageHandler *handlers, uint8_t num_handlers)
                : _in(0),
                  _out(0),
                  _handlers(handlers),
                  _num_handlers(num_handlers),
                  _sent_response(false),
                  _last_id(255)
        {
        }

        RomiSerial(IInputStream *in, IOutputStream *out,
                   const MessageHandler *handlers, uint8_t num_handlers)
                : _in(in), _out(out),
                  _handlers(handlers),
                  _num_handlers(num_handlers),
                  _sent_response(false)
        {
        }

        virtual ~RomiSerial() override = default;

        void handle_input() override;
        void send_ok() override;
        void send_error(int code, const char *message) override;
        void send(const char *message) override;
        void log(const char *message) override;

protected:
        
        char convert_4bits_to_hex(uint8_t value) {
                value &= 0x0f;
                return (value < 10)? '0' + value : 'a' + (value - 10);
        }

        void append_hex(uint8_t value) {
                append_char(convert_4bits_to_hex(value >> 4));
                append_char(convert_4bits_to_hex(value));
        }

        void append_id() {
                append_hex(_parser.id());
        }

        void append_crc() {
                append_hex(_crc.get());
        }

};

#endif
