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
#include <CRC8.h>
#include <RomiSerialClient.h>
#include <RomiSerialErrors.h>

using namespace std;

#define VALID_OPCODE(_c)      (('a' <= (_c) && (_c) <= 'z') \
                               || ('A' <= (_c) && (_c) <= 'Z')  \
                               || ('0' <= (_c) && (_c) <= '9')  \
                               || ((_c) == '?'))
#define VALID_HEX_CHAR(_c)    (('a' <= (_c) && (_c) <= 'f') \
                               || ('0' <= (_c) && (_c) <= '9'))


RomiSerialClient::RomiSerialClient(IInputStream *in, IOutputStream *out)
        : _in(in), _out(out), _id(255), _debug(false)
{
        _mutex = new_mutex();
        if (_in)
                in->set_timeout(0.1f);
}

RomiSerialClient::~RomiSerialClient()
{
        delete_mutex(_mutex);
}

static char hex(uint8_t value) {
        value &= 0x0f;
        return (value < 10)? '0' + value : 'a' + (value - 10);
}

int RomiSerialClient::make_request(const char *command, std::string &request)
{
        int err = 0;
        CRC8 crc;

        request = "";

        if (command != 0 && strlen(command) > 0) {
                if (strlen(command) <= 58) {
                        if (VALID_OPCODE(command[0])) {
                                
                                _id++;
                                request = "#";
                                request += command;
                                request += ":";
                                request += hex(_id >> 4);
                                request += hex(_id);
                                uint8_t code = crc.compute(request.c_str(),
                                                           request.length());
                                request += hex(code >> 4);
                                request += hex(code);
                                request += "\r";
                                
                        } else {
                                err = romiserialclient_invalid_opcode;
                        }
                
                } else {
                        err = romiserialclient_too_long;
                }
                        
        } else {
                err = romiserial_empty_request;
        }
        
        return err;
}

json_object_t RomiSerialClient::try_sending_request(std::string &request)
{
        json_object_t response = json_null();

        if (_debug) {
                r_debug("RomiSerialClient::try_sending_request: %s", request.c_str());
        }
        
        for (int i = 0; i < 3; i++) {
                if (send_request(request)) {
                        
                        json_object_t r = read_response();

                        /* Check the error code. If the error relates
                         * to the message envelope then send the
                         * message again. Duplicate messages are
                         * intercepted by the firmware, in which case
                         * the romiserial_duplicate error code is
                         * returned.  */
                        int code = (int) json_array_getnum(r, 0);
                        
                        if (code == romiserial_envelope_crc_mismatch
                            || code == romiserial_envelope_invalid_id
                            || code == romiserial_envelope_invalid_crc
                            || code == romiserial_envelope_expected_end
                            || code == romiserial_envelope_too_long
                            || code == romiserial_envelope_missing_metadata) {
                                
                                if (_debug) {
                                        r_debug("RomiSerialClient::try_sending_request: "
                                                "re-sending request: %s", request.c_str());
                                }
                                json_unref(r);
                                
                        } else  {
                                response = r;
                                break;
                        } 
                        
                }
                clock_sleep(0.010);
        }

        return response;
}

bool RomiSerialClient::send_request(std::string &request)
{
        size_t n = _out->print(request.c_str());
        return n == request.length();
}

json_object_t RomiSerialClient::make_error(int code)
{
        const char *message = get_error_message(code);

        if (_debug) {
                r_debug("RomiSerialClient::make_error: %d, %s", code, message);
        }
        
        json_object_t error = json_array_create();
        json_array_setnum(error, code, 0);
        json_array_setstr(error, message, 1);
        return error;
}

enum {
        romiserialclient_start_message,
        romiserialclient_opcode,
        romiserialclient_openbracket,
        romiserialclient_values,
        romiserialclient_colon,
        romiserialclient_id1,
        romiserialclient_id2,
        romiserialclient_crc1,
        romiserialclient_crc2,
        romiserialclient_carriage_return,
        romiserialclient_line_feed,
        romiserialclient_message_complete,
        romiserialclient_error,
        romiserialclient_log_message,
        romiserialclient_log_line_feed,
};

bool RomiSerialClient::parse_char(int c)
{
        return _parser.process((char) c);
}

json_object_t RomiSerialClient::check_error_response(json_object_t data)
{
        int code = (int) json_array_getnum(data, 0);
        
        if (_debug) {
                r_debug("RomiSerialClient::parse_response: "
                        "Firmware returned error code: %d (%s)",
                        code, get_error_message(code));
        }

        if (json_array_length(data) == 1) {
                const char *message = get_error_message(code);
                json_array_setstr(data, message, 1);
                
        } else if (json_array_length(data) == 2) {
                json_object_t message = json_array_get(data, 1);
                if (json_isstring(message)) {
                        if (_debug) {
                                r_debug("RomiSerialClient::parse_response: "
                                        "Firmware returned error message: '%s'",
                                        json_string_value(message));
                        }
                } else {
                        r_warn("RomiSerialClient::parse_response: "
                               "error with invalid message: '%s'",
                               _parser.message());
                        json_unref(data);
                        data = make_error(romiserial_invalid_error_response);
                }  

        } else {
                r_warn("RomiSerialClient::parse_response: "
                       "error with invalid arguments: '%s'",
                       _parser.message());
                json_unref(data);
                data = make_error(romiserial_invalid_error_response);
        }
        
        return data;
}

json_object_t RomiSerialClient::parse_response()
{
        json_object_t data = json_null();
        
        if (_parser.length() > 1) {
                
                data = json_parse(_parser.message() + 1);

                // Check that the data is valid. If not, return an error.
                if (json_isnull(data)) {
                        data = make_error(romiserial_invalid_json);
                        
                } else if (!json_isarray(data)) {
                        r_warn("RomiSerialClient::parse_response: invalid response: '%s'",
                               _parser.message());
                        json_unref(data);
                        data = make_error(romiserial_invalid_json);
                } else if (json_isnumber(json_array_get(data, 0))) {

                        // If the response is an error message, make sure it is valid,
                        // too: an array of length 2, with a string as second element.
                        int code = (int) json_array_getnum(data, 0);
                        if (code != 0) 
                                data  = check_error_response(data);
                        
                } else {
                        json_unref(data);
                        data = make_error(romiserial_invalid_response);
                }
                
        } else {
                r_warn("RomiSerialClient::parse_response: "
                       "invalid response: no values: '%s'", _parser.message());
                data = make_error(romiserial_empty_response);
        }

        return data;
}

bool RomiSerialClient::filter_log_message()
{
        bool is_message = true;
        const char *message = _parser.message();
        if (_parser.length() > 1 && message[0] == '!') {
                if (_parser.length() > 2) {
                        r_debug("RomiSerialClient: Firmware says: '%s'", message + 1);
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
        int c = _in->read();
        if (c >= 0) {

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

json_object_t RomiSerialClient::read_response()
{
        json_object_t result = json_null();
        double start_time;
        bool has_response = false;
        
        start_time = clock_time();
        
        while (!has_response) {

                int available = _in->available();
                
                if (available > 0) {
                        
                        bool has_message = handle_one_char();
                        
                        // r_debug("Has message (pre-filter)?:  %s",
                        //         has_message? "true": "false");

                        if (has_message) 
                                has_message = filter_log_message();

                        // r_debug("Has message (post-filter)?: %s",
                        //         has_message? "true": "false");
                        
                        if (has_message) {

                                if (_debug) {
                                        r_debug("RomiSerialClient::read_response: %s",
                                                _parser.message());
                                }

                                result = parse_response();

                                // Check whether we have a valid response.
                                if (_parser.id() == _id) {
                                        has_response = true;
                                        
                                } else if (json_array_getnum(result, 0) != 0.0) {
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
                                        r_warn("RomiSerialClient: ID mismatch: "
                                               "request(%d) != response(%d): "
                                               "response: '%s'",
                                               _id, _parser.id(), 
                                               _parser.message());

                                        // Try again
                                        _parser.reset();
                                        json_unref(result);
                                }
                                
                        } else if (_parser.error() != 0) {
                                r_warn("invalid response: '%s'", _parser.message());
                                result = make_error(_parser.error());
                                has_response = true;
                        }
                }

                // This timeout results from reading the complete
                // message. Return an error if the reading requires
                // more than the ROMISERIALCLIENT_TIMEOUT seconds.
                double now = clock_time();
                if (now - start_time > ROMISERIALCLIENT_TIMEOUT) {
                        result = make_error(romiserial_connection_timeout);
                        has_response = true;
                }
        }
        
        return result;
}

json_object_t RomiSerialClient::send(const char *command)
{
        std::string request;
        json_object_t response = json_null();

        if (_in == 0 || _out == 0)
                throw std::runtime_error("RomiSerialClient: Streams not initialized");
        
        mutex_lock(_mutex);
        
        int err = make_request(command, request);
        if (err == 0) {
                response = try_sending_request(request);
        } else {
                response = make_error(err);
        }
        
        mutex_unlock(_mutex);
        return response;
}

const char *RomiSerialClient::get_error_message(int code)
{
        const char *r = 0;
        switch (code) {
                
        case romiserial_error_none:
                r = "No error";
                break;
                
        case romiserial_envelope_too_long:
                r = "Request too long";
                break;
        case romiserial_envelope_invalid_id:
                r = "Invalid ID in request envelope";
                break;
        case romiserial_envelope_invalid_crc:
                r = "Invalid CRC in request envelope";
                break;
        case romiserial_envelope_crc_mismatch:
                r = "CRC mismatch in request envelope";
                break;
        case romiserial_envelope_expected_end:
                r = "Expected the end of the request envelope";
                break;
        case romiserial_envelope_missing_metadata:
                r = "Request envelope has no metadata";
                break;
        case romiserial_envelope_invalid_dummy_metadata:
                r = "Request envelope invalid dummy metadata";
                break;

        case romiserial_unexpected_char:
                r = "Unexpected character in request";
                break;
        case romiserial_vector_too_long:
                r = "Too many arguments";
                break;
        case romiserial_value_out_of_range:
                r = "Value out of range";
                break;
        case romiserial_string_too_long:
                r = "String too long";
                break;
        case romiserial_invalid_string:
                r = "Invalid string";
                break;
        case romiserial_too_many_strings:
                r = "Too many strings";
                break;
        case romiserial_invalid_opcode:
                r = "Invalid opcode";
                break;
                
        case romiserial_duplicate:
                r = "Duplicate message";
                break;
        case romiserial_unknown_opcode:
                r = "Unknown opcode";
                break;
        case romiserial_bad_number_of_arguments:
                r = "Bad number of arguments";
                break;
        case romiserial_missing_string:
                r = "Missin string argument";
                break;
        case romiserial_bad_string:
                r = "Bad string";
                break;
        case romiserial_bad_handler:
                r = "Corrupt request handler";
                break;
                
        case romiserialclient_invalid_opcode:
                r = "Invalid opcode";
                break;
        case romiserialclient_too_long:
                r = "Request too long";
                break;
        case romiserial_connection_timeout:
                r = "The connection timed out";
                break;
        case romiserial_empty_request:
                r = "Null or zero-length request";
                break;
        case romiserial_empty_response:
                r = "Null or zero-length response";
                break;
        case romiserial_invalid_json:
                r = "Invalid JSON";
                break;
        case romiserial_invalid_response:
                r = "Response is badly formed";
                break;
        case romiserial_invalid_error_response:
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

#endif
