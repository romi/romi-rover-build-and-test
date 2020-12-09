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
#ifndef __ROMI_SERIAL_CLIENT_H
#define __ROMI_SERIAL_CLIENT_H

#if !defined(ARDUINO)

#include <string>
#include <IRomiSerialClient.h>
#include <IInputStream.h>
#include <IOutputStream.h>
#include <EnvelopeParser.h>

// A 2.0 second timeout to read the response messages.
#define ROMISERIALCLIENT_TIMEOUT 2.0

class RomiSerialClient : public IRomiSerialClient
{
protected:
        IInputStream *_in;
        IOutputStream *_out;
        mutex_t *_mutex;
        uint8_t _id; 
        bool _debug;
        EnvelopeParser _parser;

        int make_request(const char *command, std::string &request);
        json_object_t try_sending_request(std::string &request);
        bool send_request(std::string &request);
        const char *get_error_message(int code);
        json_object_t make_error(int code);
        bool handle_one_char();
        bool parse_char(int c);
        json_object_t parse_response();
        json_object_t read_response();
        bool can_write();
        bool filter_log_message();
        json_object_t check_error_response(json_object_t data);

public:
        RomiSerialClient(IInputStream *in = 0, IOutputStream *out = 0);
        virtual ~RomiSerialClient() override;

        void init(IInputStream *in, IOutputStream *out) {
                _in = in;
                _out = out;
                if (_in)
                        _in->set_timeout(0.1f);
        }
        
        uint8_t id() {
                return _id;
        }
        
        json_object_t send(const char *command) override;

        
        void set_debug(bool value) {
                _debug = value;
        }        
};

#endif
#endif
