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
#ifndef __ROMI_SERIAL_ERRORS_H
#define __ROMI_SERIAL_ERRORS_H

enum {
        romiserial_error_none = 0,

        // Errors generated by the EnvelopeParser 
        romiserial_envelope_too_long = -1,
        romiserial_envelope_invalid_id = -2,
        romiserial_envelope_invalid_crc = -3,
        romiserial_envelope_crc_mismatch = -4,
        romiserial_envelope_expected_end = -5,
        romiserial_envelope_missing_metadata = -6,
        romiserial_envelope_invalid_dummy_metadata = -7,
        
        // Errors generated by the MessageParser 
        romiserial_unexpected_char = -100,
        romiserial_vector_too_long = -101,
        romiserial_value_out_of_range = -102,
        romiserial_string_too_long = -103,
        romiserial_invalid_string = -104,
        romiserial_too_many_strings = -105,
        romiserial_invalid_opcode = -106,

        // Errors generated by RomiSerial 
        romiserial_duplicate = -150,
        romiserial_unknown_opcode = -151,
        romiserial_bad_number_of_arguments = -152,
        romiserial_missing_string = -153,
        romiserial_bad_string = -154,
        romiserial_bad_handler = -155,

        // Errors generated by RomiSerialClient 
        romiserialclient_invalid_opcode = -201,
        romiserialclient_too_long = -202,
        romiserial_empty_response = -203,
        romiserial_empty_request = -204,
        romiserial_connection_timeout = -205,        
        romiserial_invalid_json = -206,
        romiserial_invalid_response = -207,
        romiserial_invalid_error_response = -208,
};

#endif