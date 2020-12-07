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
        expect_start_envelope,
        expect_payload_or_start_metadata,
        expect_id_char_1,
        expect_id_char_2,
        expect_crc_char_1,
        expect_crc_char_2,
        expect_dummy_metadata_char_2,
        expect_dummy_metadata_char_3,
        expect_dummy_metadata_char_4,
        expect_end_envelope
};

#define START_ENVELOPE(_c)      ((_c) == '#')
#define END_ENVELOPE(_c)        ((_c) == '\r')
#define START_METADATA(_c)      ((_c) == ':')
#define DUMMY_METADATA_CHAR(_c) ((_c) == 'x')
#define VALID_HEX_CHAR(_c)      (('a' <= (_c) && (_c) <= 'f') \
                                 || ('0' <= (_c) && (_c) <= '9'))

void EnvelopeParser::set_error(char character, char what)
{
        _message[_message_length] = '\0';
#if defined(ARDUINO) 
        log_print("error");
        if (character == '\r') {
                log_print("<cr>");
        } else if (character == '\n') {
                log_print("<nl>");
        } else if (character == '\0') {
                log_print("<0>");
        } else {
                log_print(character);
        }
        log_print((int) what);
        log_print(_message);
#endif        
        _error = what;
        _state = expect_start_envelope;
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
        _state = expect_start_envelope;
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

#if !defined(ARDUINO)
        // _message[_message_length] = 0;
        // r_debug("buffer: '%s', process '%c'", _message, c);
#endif
        
        switch(_state) {
        case expect_start_envelope:
#if !defined(ARDUINO)
                // r_debug("expect_start_envelope");
#endif
                if (START_ENVELOPE(c)) {
                        reset();
                        _state = expect_payload_or_start_metadata;
                        _crc.update(c);
                } else {
                        // NOP
                }
                break;
                
        case expect_payload_or_start_metadata:
#if !defined(ARDUINO)
                // r_debug("expect_payload_or_start_metadata");
#endif
                if (START_METADATA(c)) {
                        _state = expect_id_char_1;
                        _crc.update(c);
                } else if (END_ENVELOPE(c)) {
                        set_error(c, romiserial_envelope_missing_metadata);
                } else {
                        append_char(c);
                        _crc.update(c);
                }
                break;
                
        case expect_id_char_1:
#if !defined(ARDUINO)
                // r_debug("expect_id_char_1");
#endif
                if (DUMMY_METADATA_CHAR(c)) {
                        _has_id = false;
                        _state = expect_dummy_metadata_char_2;
                } else if (VALID_HEX_CHAR(c)) {
                        _has_id = true;
                        _id = hex_to_int(c);
                        _state = expect_id_char_2;
                        _crc.update(c);
                } else {
                        set_error(c, romiserial_envelope_invalid_id);
                }
                break;
                
        case expect_id_char_2:
#if !defined(ARDUINO)
                // r_debug("expect_id_char_2");
#endif
                if (VALID_HEX_CHAR(c)) {
                        _id = 16 * _id + hex_to_int(c);
                        _state = expect_crc_char_1;
                        _crc.update(c);
                } else {
                        set_error(c, romiserial_envelope_invalid_id);
                }
                break;
                
        case expect_crc_char_1:
#if !defined(ARDUINO)
                // r_debug("expect_crc_char_1");
#endif
                if (VALID_HEX_CHAR(c)) {
                        _crc_metadata = hex_to_int(c);
                        _state = expect_crc_char_2;
                } else {
                        set_error(c, romiserial_envelope_invalid_crc);
                }
                break;
                
        case expect_crc_char_2:
#if !defined(ARDUINO)
                // r_debug("expect_crc_char_2");
#endif
                if (VALID_HEX_CHAR(c)) {
                        uint8_t crc = _crc.finalize();
                        
                        _crc_metadata = 16 * _crc_metadata + hex_to_int(c);
                        if (_crc_metadata == crc) {
#if defined(ARDUINO)
                                Serial.print("#!");
                                Serial.print("crc=");
                                Serial.print(crc, HEX);
                                Serial.println(":xxxx");
#endif
                                _state = expect_end_envelope;
                        } else {
                                set_error(c, romiserial_envelope_crc_mismatch);
#if defined(ARDUINO)
                                Serial.print("#!");
                                Serial.print("crc=");
                                Serial.print(crc, HEX);
                                Serial.println(":xxxx");
#endif
                        }
                } else {
                        set_error(c, romiserial_envelope_invalid_crc);
                }
                break;
                
        case expect_dummy_metadata_char_2:
#if !defined(ARDUINO)
                // r_debug("expect_dummy_metadata_char_2");
#endif
                if (DUMMY_METADATA_CHAR(c)) {
                        _state = expect_dummy_metadata_char_3;
                } else {
                        set_error(c, romiserial_envelope_invalid_dummy_metadata);
                }
                break;
        case expect_dummy_metadata_char_3:
#if !defined(ARDUINO)
                // r_debug("expect_dummy_metadata_char_3");
#endif
                if (DUMMY_METADATA_CHAR(c)) {
                        _state = expect_dummy_metadata_char_4;
                } else {
                        set_error(c, romiserial_envelope_invalid_dummy_metadata);
                }
                break;
        case expect_dummy_metadata_char_4:
#if !defined(ARDUINO)
                // r_debug("expect_dummy_metadata_char_4");
#endif
                if (DUMMY_METADATA_CHAR(c)) {
                        _state = expect_end_envelope;
                } else {
                        set_error(c, romiserial_envelope_invalid_dummy_metadata);
                }
                break;
                
        case expect_end_envelope:
#if !defined(ARDUINO)
                // r_debug("expect_end_envelope");
#endif
                if (END_ENVELOPE(c)) {
                        append_char('\0');
                        has_message = true;
                        _state = expect_start_envelope;
                } else {
                        set_error(c, romiserial_envelope_expected_end);
                }
        }
        
        return has_message;
}
