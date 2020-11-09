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
#include <CRC8.h>
#include <RomiSerialClient.h>
#include <RomiSerialErrors.h>

#if !defined(ARDUINO)

using namespace std;

RomiSerialClient::RomiSerialClient(IInputStream *in, IOutputStream *out)
        : _in(in), _out(out)
{
        _id_request = 255;
        _mutex = new_mutex();
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

bool RomiSerialClient::make_request(const char *command)
{
        CRC8 crc;
        _id_request++;
        _request = "#";
        _request += command;
        _request += ":";
        _request += hex(_id_request >> 4);
        _request += hex(_id_request);
        uint8_t code = crc.compute(_request.c_str(), _request.length());
        _request += hex(code >> 4);
        _request += hex(code);
        _request += "\r";

        if (_request.length() > 64)
                return false;

        return true;
}

bool RomiSerialClient::send_request()
{
        size_t n = _out->print(_request.c_str());
        return n == _request.length();
}

json_object_t RomiSerialClient::make_error(int code, const char *message)
{
        json_object_t error = json_array_create();
        json_array_setnum(error, code, 0);
        json_array_setstr(error, get_error_message(code, message), 1);
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

#define VALID_OPCODE(_c)      (('a' <= (_c) && (_c) <= 'z') \
                               || ('A' <= (_c) && (_c) <= 'Z')  \
                               || ('0' <= (_c) && (_c) <= '9')  \
                               || ((_c) == '?'))
#define VALID_HEX_CHAR(_c)    (('a' <= (_c) && (_c) <= 'f') \
                               || ('0' <= (_c) && (_c) <= '9'))

// This function doesn't return an error code is the character is
// invalid. Use the macro VALID_HEX_CHAR() for that purpose.
static uint8_t hex_to_int(char c)
{
        if ('a' <= c && c <= 'f') {
                return 10 + (c - 'a');
        } else if ('0' <= c && c <= '9') {
                return c - '0';
        } else {
                return 0;
        }
}

void RomiSerialClient::parse_char(int c)
{
        switch (_state) {
        case romiserialclient_start_message:
                if (c == '#') {
                        _response += c;
                        _state = romiserialclient_opcode;
                } else if (c == '!') {
                        _log_message = "";
                        _state = romiserialclient_log_message;
                } else {
                        // NOP, wait for the hashtag
                }
                break;

        case romiserialclient_opcode:
                if (VALID_OPCODE(c)) {
                        _response += c;
                        _state = romiserialclient_openbracket;
                } else {
                        _error = romiserialclient_invalid_opcode;
                }
                break;

        case romiserialclient_openbracket:
                if (c == '[') {
                        _response += c;
                        _state = romiserialclient_values;
                } else {
                        _error = romiserialclient_unexpected_char;
                }
                break;

        case romiserialclient_values:
                if (c == ']') {
                        _response += c;
                        _state = romiserialclient_colon;
                } else {
                        _response += c;
                }
                break;

        case romiserialclient_colon:
                if (c == ':') {
                        _response += c;
                        _state = romiserialclient_id1;
                } else {
                        _error = romiserialclient_unexpected_char;
                }
                break;

        case romiserialclient_id1:
                if (VALID_HEX_CHAR(c)) {
                        _id_response = hex_to_int(c);
                        _response += c;
                        _state = romiserialclient_id2;
                } else {
                        _error = romiserialclient_unexpected_char;
                }
                break;

        case romiserialclient_id2:
                if (VALID_HEX_CHAR(c)) {
                        _id_response = 16 * _id_response + hex_to_int(c);
                        _response += c;
                        _state = romiserialclient_crc1;
                } else {
                        _error = romiserialclient_unexpected_char;
                }
                break;

        case romiserialclient_crc1:
                if (VALID_HEX_CHAR(c)) {
                        _crc_response = hex_to_int(c);
                        _state = romiserialclient_crc2;
                } else {
                        printf("romiserialclient_crc1: romiserialclient_unexpected_char\n");
                        _error = romiserialclient_unexpected_char;
                }
                break;

        case romiserialclient_crc2:
                if (VALID_HEX_CHAR(c)) {
                        CRC8 crc;                        
                        _crc_response = 16 * _crc_response + hex_to_int(c);
                        crc.compute(_response.c_str(), _response.length());
                        if (_crc_response != crc.get()) 
                                _error = romiserialclient_crc_mismatch;
                        else
                                _state = romiserialclient_carriage_return;
                } else {
                        printf("romiserialclient_crc2: romiserialclient_unexpected_char\n");
                        _error = romiserialclient_unexpected_char;
                }
                break;

        case romiserialclient_carriage_return:
                if (c == '\r') {
                        _state = romiserialclient_line_feed;
                } else {
                        printf("romiserialclient_carriage_return: romiserialclient_unexpected_char: 0x%02x\n", (int) c);
                        _error = romiserialclient_unexpected_char;
                }
                break;

        case romiserialclient_line_feed:
                if (c == '\n') {
                        _state = romiserialclient_message_complete;
                } else {
                        printf("romiserialclient_line_feed: romiserialclient_unexpected_char: 0x%02x\n", (int) c);
                        _error = romiserialclient_unexpected_char;
                }
                break;

        case romiserialclient_log_message:
                if (c == '\r') {
                        _state = romiserialclient_log_line_feed;
                } else {
                        _log_message += c;
                }
                break;
                
        case romiserialclient_log_line_feed:
                if (c == '\n') {
                        r_debug("RomiSerial: %s", _log_message.c_str());
                        _state = romiserialclient_start_message;
                } else {
                        _error = romiserialclient_unexpected_char;
                }
                break;
        }
}

json_object_t RomiSerialClient::parse_response()
{
        string value = _response.substr(2, _response.length() - 5);
        
        json_object_t data = json_parse(value.c_str());

        // Check that the data is valid. If not, return an error.
        if (json_isnull(data))
                data = make_error(romiserialclient_invalid_json);
        else if (!json_isarray(data)) 
                data = make_error(romiserialclient_invalid_json);

        // If the response is an error message, make sure it is valid,
        // too: an array of length 2, with a string as second element.
        int code = (int) json_array_getnum(data, 0);
        if (code != 0) {
                if (json_array_length(data) == 1) {
                        const char *message = get_error_message(code);
                        json_array_setstr(data, message, 1);
                } else if (json_array_length(data) == 2) {
                        json_object_t message = json_array_get(data, 1);
                        if (!json_isstring(message)) {
                                data = make_error(romiserialclient_invalid_error);
                        }
                } else {
                        data = make_error(romiserialclient_invalid_error);
                }
        }

        return data;
}

void RomiSerialClient::read_one_char()
{
        int c = _in->read();
        if (c >= 0) {
                parse_char(c);
                        
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
}

json_object_t RomiSerialClient::read_response()
{
        json_object_t result = json_null();
        double start_time;

        start_time = clock_time();
        _state = romiserialclient_start_message;
        _error = 0;
        _response = "";
        
        while (true) {
                
                if (_in->available() > 0) {
                        
                        read_one_char();

                        if (_state == romiserialclient_message_complete) {

                                if (_id_response == _id_request) {
                                        result = parse_response();
                                        break;
                                        
                                } else {
                                        r_warn("RomiSerialClient: ID mismatch: "
                                               "request(%d) != response(%d)",
                                               _id_request, _id_response);
                                        
                                        // Try again
                                        _state = romiserialclient_start_message;
                                        _error = 0;
                                        _response = "";
                                }
                                
                        } else if (_error != 0) {
                                result = make_error(_error);
                                break;
                        }
                }

                // This timeout results from reading the complete
                // message. Return an error if the reading requires
                // more than the ROMISERIALCLIENT_TIMEOUT seconds.
                if (clock_time() - start_time > ROMISERIALCLIENT_TIMEOUT) {
                        result = make_error(romiserialclient_connection_timeout);
                        break;
                }
        }
        return result;
}

json_object_t RomiSerialClient::send(const char *command)
{
        json_object_t response = json_null();

        mutex_lock(_mutex);

        if (make_request(command)) {
                if (send_request()) {
                        response = read_response();
                } else {
                        response = make_error(romiserialclient_connection_failed);
                }
        } else {
                response = make_error(romiserialclient_too_long);
        }
        
        mutex_unlock(_mutex);
        return response;
}

const char *RomiSerialClient::get_error_message(int code, const char *message)
{
        const char *r = message;
        if (message == 0) {
                switch (code) {
                case romiserial_error_none:
                        r = "No error";
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
                case romiserial_invalid_id:
                        r = "Invalid ID in request";
                        break;
                case romiserial_invalid_crc:
                        r = "Invalid CRC in request";
                        break;
                case romiserial_crc_mismatch:
                        r = "CRC mismatch in request";
                        break;
                case romiserial_too_long:
                        r = "Request too long";
                        break;
                case romiserial_invalid_opcode:
                        r = "Invalid opcode";
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
                case romiserialclient_connection_failed:
                        r = "The connection failed";
                        break;
                case romiserialclient_connection_timeout:
                        r = "The connection timed out";
                        break;
                case romiserialclient_unexpected_char:
                        r = "Unexpected character in the response";
                        break;
                case romiserialclient_invalid_opcode:
                        r = "Invalid opcode in the response";
                        break;
                case romiserialclient_crc_mismatch:
                        r = "CRC mismatch in the response";
                        break;
                case romiserialclient_invalid_json:
                        r = "Invalid JSON";
                        break;
                case romiserialclient_invalid_error:
                        r = "Response contains an invalid error message";
                        break;
                case romiserialclient_too_long:
                        r = "Request too long";
                        break;
                default:
                        r = "Unknown error";
                        break;
                }
        }
        return r;
}

#endif
