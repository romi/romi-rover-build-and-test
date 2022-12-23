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
#ifndef __ROMISERIAL_ROMISERIALCLIENT_H
#define __ROMISERIAL_ROMISERIALCLIENT_H

#if !defined(ARDUINO)

#include <string>
#include <memory>
#include <mutex>
#include <IRomiSerialClient.h>
#include <IInputStream.h>
#include <IOutputStream.h>
#include <EnvelopeParser.h>
#include <RomiSerialErrors.h>

namespace romiserial {

        enum {
                kStatusCode = 0,
                kErrorMessage = 1
        };

        // A 2.0 second timeout to read the response messages.
        static const double kRomiSerialClientTimeout = 2.0;
        static const uint32_t kDefaultBaudRate = 115200;

        using SynchronizedCodeBlock = std::lock_guard<std::mutex>;
        class RomiSerialClient : public IRomiSerialClient
        {
        protected:
                std::shared_ptr<IInputStream> _in;
                std::shared_ptr<IOutputStream> _out;
                std::mutex _mutex;
                uint8_t _id; 
                bool _debug;
                EnvelopeParser _parser;
                JsonCpp default_response_;
                double timeout_;
                const std::string client_name_;
                
                int make_request(const std::string &command, std::string &request);
                JsonCpp try_sending_request(std::string &request);
                bool send_request(std::string &request);
                JsonCpp make_error(int code);
                bool handle_one_char();
                bool parse_char(int c);
                JsonCpp parse_response();
                JsonCpp read_response();
                bool can_write();
                bool filter_log_message();
                JsonCpp check_error_response(JsonCpp& data);
                JsonCpp make_default_response();
                std::string substitute_metachars(const std::string& command);

        public:
        
                static std::unique_ptr<IRomiSerialClient> create(const std::string& device, const std::string& client_name);
                static uint8_t any_id();
                
                explicit RomiSerialClient(std::shared_ptr<IInputStream> in,
                                          std::shared_ptr<IOutputStream> out,
                                          uint8_t start_id,
                                          const std::string& client_name);
                RomiSerialClient(const RomiSerialClient&) = delete;
                RomiSerialClient& operator=(const RomiSerialClient&) = delete;
                ~RomiSerialClient() override;

                uint8_t id() {
                        return _id;
                }
        
                void send(const char *command, JsonCpp& response) override;
        
                /* bool read(uint8_t *data, size_t length) override; */
                /* bool write(const uint8_t *data, size_t length) override; */
        
                void set_debug(bool value) override {
                        _debug = value;
                }
        
                static const char *get_error_message(int code);        
        };
}

#endif 
#endif // __ROMISERIAL_ROMISERIALCLIENT_H
