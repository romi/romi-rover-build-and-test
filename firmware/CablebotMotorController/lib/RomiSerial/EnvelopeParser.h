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
#ifndef __ROMISERIAL_ENVELOPE_PARSER_H
#define __ROMISERIAL_ENVELOPE_PARSER_H

#include "CRC8.h"

namespace romiserial {

#define MAX_MESSAGE_LENGTH 58

        enum envelope_parser_state_t {
                expect_start_envelope = 0,
                expect_payload_or_start_metadata,
                expect_id_char_1,
                expect_id_char_2,
                expect_crc_char_1,
                expect_crc_char_2,
                expect_dummy_metadata_char_2,
                expect_dummy_metadata_char_3,
                expect_dummy_metadata_char_4,
                expect_end_metadata,
                expect_end_envelope
        };

        class EnvelopeParser
        {
        protected:
                envelope_parser_state_t _state;
                int8_t _error;
                CRC8 _crc;
                uint8_t _id;
                bool _has_id;
                uint8_t _crc_metadata;
                char _message[MAX_MESSAGE_LENGTH+1];
                uint8_t _message_length;
                
                void set_error(char character, int8_t what);
                void append_char(char c);

        public:
        
                EnvelopeParser();
                ~EnvelopeParser() = default;

                uint8_t id() {
                        return _id;
                }

                bool has_id() {
                        return _has_id;
                }
        
                uint8_t crc() {
                        return _crc.get();
                }
        
                int8_t error() {
                        return _error;
                }
        
                const char *message() {
                        return _message;
                }
        
                uint8_t length() {
                        return _message_length;
                }
        
                void reset();
                bool process(char c);
        };
}

#endif // __ROMISERIAL_ENVELOPE_PARSER_H
