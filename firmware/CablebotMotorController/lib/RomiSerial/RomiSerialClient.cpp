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

#if !defined(ARDUINO)

#include <stdexcept>
#include <memory>
#include <algorithm>
#include <CRC8.h>
#include <RomiSerialClient.h>
#include <RomiSerialErrors.h>
#include <log.h>
#include <ClockAccessor.h>
#include "RomiSerialUtil.h"
#include "RSerial.h"
#include <stdlib.h>
#include <time.h>

using namespace std;

namespace romiserial {

        std::unique_ptr<IRomiSerialClient>
        RomiSerialClient::create(const  std::string& device, const std::string& client_name)
        {
                std::shared_ptr<RSerial> serial
                        = std::make_shared<RSerial>(device, kDefaultBaudRate, kDontReset);
                std::unique_ptr<IRomiSerialClient> romi_serial
                        = std::make_unique<RomiSerialClient>(serial, serial, any_id(), client_name);
                return romi_serial;
        }

        uint8_t RomiSerialClient::any_id()
        {
                srand((unsigned int) time(nullptr));
                return (uint8_t) (rand() % 255);
        }

        RomiSerialClient::RomiSerialClient(std::shared_ptr<IInputStream> in,
                                           std::shared_ptr<IOutputStream> out,
                                           uint8_t start_id,
                                           const std::string& client_name)
                :   _in(in),
                    _out(out),
                    _mutex(),
                    _id(start_id),
                    _debug(false),
                    _parser(),
                    default_response_(),
                    timeout_(kRomiSerialClientTimeout),
                    client_name_(client_name)
        {
                in->set_timeout(0.1f);
                default_response_ = make_default_response();
        }

        RomiSerialClient::~RomiSerialClient()
        {

        }

        std::string RomiSerialClient::substitute_metachars(const std::string &command)
        {
                std::string copy = command;
                std::replace(copy.begin(), copy.end(), ':', '-');
                return copy;
        }
        
        int RomiSerialClient::make_request(const std::string &command, std::string &request)
        {
                int err = 0;
                CRC8 crc;

                request = "";
                if (command.length() > 0) {
                        if (command.length() <= MAX_MESSAGE_LENGTH) {
                                if (is_valid_opcode(command[0])) {
                                
                                        _id++;
                                        request = "#";
                                        request += substitute_metachars(command);
                                        request += ":";
                                        request += to_hex((uint8_t)(_id >> 4));
                                        request += to_hex(_id);
                                        uint8_t code = crc.compute(request.c_str(),
                                                                   request.length());
                                        request += to_hex((uint8_t)(code >> 4));
                                        request += to_hex(code);
                                        request += "\r";
                                        request += "\n";
                                
                                } else {
                                        err = kInvalidOpcode;
                                }
                
                        } else {
                                err = kClientTooLong;
                        }
                } else {
                        err = kEmptyRequest;
                }
        
                return err;
        }

        JsonCpp RomiSerialClient::try_sending_request(std::string &request)
        {
                JsonCpp response(default_response_);

                if (_debug) {
                        r_debug("RomiSerialClient<%s>::try_sending_request: %s", client_name_.c_str(), request.c_str());
                }
        
                for (int i = 0; i < 3; i++) {
                        if (send_request(request)) {
                        
                                response = read_response();

                                /* Check the error code. If the error relates
                                 * to the message envelope then send the
                                 * message again. Duplicate messages are
                                 * intercepted by the firmware, in which case
                                 * the kDuplicate error code is
                                 * returned.  */
                                int code = (int) response.num(0);
                        
                                if (code == kEnvelopeCrcMismatch
                                    || code == kEnvelopeInvalidId
                                    || code == kEnvelopeInvalidCrc
                                    || code == kEnvelopeExpectedEnd
                                    || code == kEnvelopeTooLong
                                    || code == kEnvelopeMissingMetadata) {
                                
                                        if (_debug) {
                                                r_debug("RomiSerialClient<%s>::try_sending_request: "
                                                        "re-sending request: %s", client_name_.c_str(), request.c_str());
                                        }
                                
                                } else  {
                                        break;
                                } 
                        
                        }
                        rpp::ClockAccessor::GetInstance()->sleep(0.010);
                }

                return response;
        }

        bool RomiSerialClient::send_request(std::string &request)
        {
                bool success = true;
                for (size_t i = 0; i < request.length(); i++) {
                        if (!_out->write(request[i])) {
                                success = false;
                                break;
                        }
                }
                return success;
        }

        JsonCpp RomiSerialClient::make_error(int code)
        {
                const char *message = get_error_message(code);

                if (_debug) {
                        r_debug("RomiSerialClient<%s>::make_error: %d, %s", client_name_.c_str(), code, message);
                }

                return JsonCpp::construct("[%d,'%s']", code, message);
        }

        bool RomiSerialClient::parse_char(int c)
        {
                return _parser.process((char) c);
        }

        JsonCpp RomiSerialClient::check_error_response(JsonCpp &data)
        {
                int code = (int) data.num(0);
        
                if (_debug) {
                        r_debug("RomiSerialClient<%s>::check_error_response: "
                                "Firmware returned error code: %d (%s)", client_name_.c_str(),
                                code, get_error_message(code));
                }

                if (data.length() == 1) {
                        const char *message = get_error_message(code);
                        data.setstr(message, 1);
                
                } else if (data.length() == 2) {
                        if (data.get(1).isstring()) {
                                if (_debug) {
                                        r_debug("RomiSerialClient<%s>::check_error_response: "
                                                "Firmware returned error message: '%s'", client_name_.c_str(),
                                                data.str(1));
                                }
                        } else {
                                r_warn("RomiSerialClient<%s>::check_error_response: "
                                       "error with invalid message: '%s'", client_name_.c_str(),
                                       _parser.message());
                                data = make_error(kInvalidErrorResponse);
                        }  

                } else {
                        r_warn("RomiSerialClient<%s>::check_error_response: "
                               "error with invalid arguments: '%s'", client_name_.c_str(),
                               _parser.message());
                        data = make_error(kInvalidErrorResponse);
                }
        
                return data;
        }

        JsonCpp RomiSerialClient::parse_response()
        {
                JsonCpp data;
        
                if (_parser.length() > 1) {
                
                        data = JsonCpp::parse(_parser.message() + 1);

                        // Check that the data is valid. If not, return an error.
                        if (data.isarray()
                            && data.length() > 0
                            && data.get(0).isnumber()) {
                        
                                // If the response is an error message, make
                                // sure it is valid, too: it should be an
                                // array of length 2, with a string as second
                                // element.
                                int code = (int) data.num(0);
                                if (code != 0) 
                                        data  = check_error_response(data);
                        
                        } else {
                                r_warn("RomiSerialClient<%s>::parse_response: "
                                       "invalid response: '%s'",
                                       client_name_.c_str(),
                                       _parser.message());
                                data = make_error(kInvalidResponse);
                        }
                
                } else {
                        r_warn("RomiSerialClient<%s>::parse_response: "
                               "invalid response: no values: '%s'", client_name_.c_str(), _parser.message());
                        data = make_error(kEmptyResponse);
                }

                return data;
        }

        bool RomiSerialClient::filter_log_message()
        {
                bool is_message = true;
                const char *message = _parser.message();
                if (_parser.length() > 1 && message[0] == '!') {
                        if (_parser.length() > 2) {
                                r_debug("RomiSerialClient<%s>: Firmware says: '%s'",
                                        client_name_.c_str(),
                                        message + 1);
                                is_message = false;
                        } else {
                                is_message = false;
                        }
                }
                return is_message;
        }

        bool RomiSerialClient::handle_one_char()
        {
                bool has_message = false;
                char c;
                if (_in->read(c)) {

                        has_message = parse_char(c);
                
                } else {
                        // This timeout results from reading a single
                        // character. The timeout value was set in the
                        // constructor: _in->set_timeout().

                        // This timeout is ignored here. We will only
                        // check the total timeout for the whole
                        // message.
                        
                        //result = make_error(romiserialclient_connection_timeout);
                        //break;
                }
                return has_message;
        }

        JsonCpp RomiSerialClient::make_default_response()
        {
                int default_code = kConnectionTimeout;
                const char *default_message = get_error_message(default_code);
                json_object_t array = json_array_create();
                json_array_setnum(array, default_code, 0); 
                json_array_setstr(array, default_message, 1);
                JsonCpp response(array);
                json_unref(array);
                return response;
        }

        JsonCpp RomiSerialClient::read_response()
        {
                JsonCpp response(default_response_);
                double start_time;
                bool has_response = false;

                auto clock = rpp::ClockAccessor::GetInstance();
                start_time = clock->time();

        
                while (!has_response) {
                
                        if (_in->available()) {
                        
                                bool has_message = handle_one_char();
                        
                                // r_debug("Has message (pre-filter)?:  %s",
                                //         has_message? "true": "false");

                                if (has_message) 
                                        has_message = filter_log_message();

                                // r_debug("Has message (post-filter)?: %s",
                                //         has_message? "true": "false");
                        
                                if (has_message) {

                                        if (_debug) {
                                                r_debug("RomiSerialClient<%s>::read_response: %s",
                                                        client_name_.c_str(),
                                                        _parser.message());
                                        }

                                        response = parse_response();

                                        // Check whether we have a valid response.
                                        if (_parser.id() == _id) {
                                                has_response = true;
                                        
                                        } else if (response.num(0) != 0) {
                                                /* It's OK if the ID in the
                                                 * response is not equal to
                                                 * the ID in the request when
                                                 * the response is an error
                                                 * because errors can be sent
                                                 * before the complete request
                                                 * is parsed. */
                                                has_response = true;
                                        
                                        } else {
                                                /* There's an ID
                                                 * mismatch. Drop this
                                                 * response and try reading
                                                 * the next one. */
                                                r_warn("RomiSerialClient<%s>: ID mismatch: "
                                                       "request(%d) != response(%d): "
                                                       "response: '%s'",
                                                       client_name_.c_str(),
                                                       _id, _parser.id(), 
                                                       _parser.message());

                                                // Try again
                                                _parser.reset();
                                        }
                                
                                } else if (_parser.error() != 0) {
                                        r_warn("RomiSerialClient<%s>: invalid response: '%s'",
                                               client_name_.c_str(),
                                               _parser.message());
                                        response = make_error(_parser.error());
                                        has_response = true;
                                }
                        }

                        // This timeout responses from reading the complete
                        // message. Return an error if the reading requires
                        // more than the timeout seconds.
                        double now = clock->time();
                        if (timeout_ > 0.0 && now - start_time > timeout_) {
                                response = make_error(kConnectionTimeout);
                                has_response = true;
                        }
                }
        
                return response;
        }

        void RomiSerialClient::send(const char *command, JsonCpp& response)
        {
                std::string request;

                SynchronizedCodeBlock sync(_mutex);
        
                int err = make_request(command, request);
                if (err == 0) {
                        response = try_sending_request(request);
                } else {
                        response = make_error(err);
                }

        }

        const char *RomiSerialClient::get_error_message(int code)
        {
                const char *r = nullptr;
                switch (code) {
                
                case kNoError:
                        r = "No error";
                        break;
                
                case kEnvelopeTooLong:
                        r = "Request too long";
                        break;
                case kEnvelopeInvalidId:
                        r = "Invalid ID in request envelope";
                        break;
                case kEnvelopeInvalidCrc:
                        r = "Invalid CRC in request envelope";
                        break;
                case kEnvelopeCrcMismatch:
                        r = "CRC mismatch in request envelope";
                        break;
                case kEnvelopeExpectedEnd:
                        r = "Expected the end of the request envelope";
                        break;
                case kEnvelopeMissingMetadata:
                        r = "Request envelope has no metadata";
                        break;
                case kEnvelopeInvalidDummyMetadata:
                        r = "Request envelope invalid dummy metadata";
                        break;

                case kUnexpectedChar:
                        r = "Unexpected character in request";
                        break;
                case kVectorTooLong:
                        r = "Too many arguments";
                        break;
                case kValueOutOfRange:
                        r = "Value out of range";
                        break;
                case kStringTooLong:
                        r = "String too long";
                        break;
                case kInvalidString:
                        r = "Invalid string";
                        break;
                case kTooManyStrings:
                        r = "Too many strings";
                        break;
                case kInvalidOpcode:
                        r = "Invalid opcode";
                        break;
                
                case kDuplicate:
                        r = "Duplicate message";
                        break;
                case kUnknownOpcode:
                        r = "Unknown opcode";
                        break;
                case kBadNumberOfArguments:
                        r = "Bad number of arguments";
                        break;
                case kMissingString:
                        r = "Missin string argument";
                        break;
                case kBadString:
                        r = "Bad string";
                        break;
                case kBadHandler:
                        r = "Corrupt request handler";
                        break;
                
                case kClientInvalidOpcode:
                        r = "Invalid opcode";
                        break;
                case kClientTooLong:
                        r = "Request too long";
                        break;
                case kConnectionTimeout:
                        r = "The connection timed out";
                        break;
                case kEmptyRequest:
                        r = "Null or zero-length request";
                        break;
                case kEmptyResponse:
                        r = "Null or zero-length response";
                        break;
                case kInvalidJson:
                        r = "Invalid JSON";
                        break;
                case kInvalidResponse:
                        r = "Response is badly formed";
                        break;
                case kInvalidErrorResponse:
                        r = "Response contains an invalid error message";
                        break;
                default:
                        if (code > 0)
                                r = "Application error";
                        else
                                r = "Unknown error code";
                        break;
                }
                return r;
        }
}

#endif
