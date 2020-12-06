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
//#include <stdio.h>
#include <string.h>
#include "EnvelopeParser.h"
#include "RomiSerialErrors.h"
#include "Log.h"

enum parser_state_t {
        wait_start_message,
        wait_message_or_end_or_metadata,
        wait_id1,
        wait_id2,
        wait_crc1,
        wait_crc2,
        wait_end_message
};

#define START_MESSAGE(_c)     ((_c) == '#')
#define START_METADATA(_c)    ((_c) == ':')
#define END_MESSAGE(_c)       ((_c) == '\r')
#define VALID_HEX_CHAR(_c)    (('a' <= (_c) && (_c) <= 'f') \
                               || ('0' <= (_c) && (_c) <= '9'))

void EnvelopeParser::set_error(char character, char what)
{
        _error = what;
        _state = wait_start_message;
}

// This function doesn't return an error code is the character is
// invalid. Use the macro VALID_HEX_CHAR() for that purpose.
static inline uint8_t hex_to_int(char c)
{
        if ('a' <= c && c <= 'f') {
                return 10 + (c - 'a');
        } else if ('0' <= c && c <= '9') {
                return c - '0';
        } else {
                return 0;
        }
}

void EnvelopeParser::append_char(char c)
{
        if (_message_length < MAX_MESSAGE_LENGTH) {
                _message[_message_length++] = c;
        } else {
                set_error(c, romiserial_envelope_too_long);
        }
}

void EnvelopeParser::reset()
{
        _state = wait_start_message;
        _message_length = 0;
        _error = romiserial_error_none;
        _crc.start();
        _crc_metadata = 0;
        _id = 0;
        _has_id = false;
}

bool EnvelopeParser::process(char c)
{
        bool has_message = false;

        _error = 0;
        
        switch(_state) {
        case wait_start_message:
                if (START_MESSAGE(c)) {
                        reset();
                        _state = wait_message_or_end_or_metadata;
                        _crc.update(c);
                } else {
                        // NOP
                }
                break;
                
        case wait_message_or_end_or_metadata:
                if (START_METADATA(c)) {
                        _state = wait_id1;
                        _crc.update(c);
                } else if (END_MESSAGE(c)) {
                        append_char('\0');
                        has_message = true;
                        _state = wait_start_message;
                } else {
                        append_char(c);
                        _crc.update(c);
                }
                break;
                
        case wait_id1:
                if (VALID_HEX_CHAR(c)) {
                        _id = hex_to_int(c);
                        _state = wait_id2;
                        _crc.update(c);
                } else {
                        set_error(c, romiserial_envelope_invalid_id);
                }
                break;
                
        case wait_id2:
                if (VALID_HEX_CHAR(c)) {
                        _id = 16 * _id + hex_to_int(c);
                        _has_id = true;
                        _state = wait_crc1;
                        _crc.update(c);
                } else {
                        set_error(c, romiserial_envelope_invalid_id);
                }
                break;
                
        case wait_crc1:
                if (VALID_HEX_CHAR(c)) {
                        _crc_metadata = hex_to_int(c);
                        _state = wait_crc2;
                } else {
                        set_error(c, romiserial_envelope_invalid_crc);
                }
                break;
                
        case wait_crc2:
                if (VALID_HEX_CHAR(c)) {
                        _crc_metadata = 16 * _crc_metadata + hex_to_int(c);
                        if (_crc_metadata == _crc.finalize())
                                _state = wait_end_message;
                        else
                                set_error(c, romiserial_envelope_crc_mismatch);
                } else {
                        set_error(c, romiserial_envelope_invalid_crc);
                }
                break;
        case wait_end_message:
                if (END_MESSAGE(c)) {
                        append_char('\0');
                        has_message = true;
                        _state = wait_start_message;
                } else {
                        set_error(c, romiserial_envelope_expected_end);
                }
        }
        
        return has_message;
}
