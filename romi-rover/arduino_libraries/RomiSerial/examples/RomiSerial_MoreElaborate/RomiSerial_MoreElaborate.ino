/*
  RomiSerial

  Copyright (C) 2019 Sony Computer Science Laboratories
  Author(s) Peter Hanappe

  RomiSerial is part of the Romi Rover project.

  RomiSerial is free software: you can redistribute it and/or modify
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
#include <RomiSerial.h>

void handler_without_arguments_returning_string(RomiSerial *romiSerial,
                                                int16_t *args,
                                                const char *string_arg)
{
        romiSerial->send("[0,\"Got it!\"]");
}

void handler_with_two_arguments(RomiSerial *romiSerial,
                                int16_t *args,
                                const char *string_arg)
{
        romiSerial->send_ok();
}

void handler_with_int_and_string_argument(RomiSerial *romiSerial,
                                  int16_t *args,
                                  const char *string_arg)
{
        romiSerial->send_ok();
}

void handler_with_two_arguments_returning_value(RomiSerial *romiSerial,
                                                int16_t *args,
                                                const char *string_arg)
{
        static char buffer[32];
        snprintf(buffer, sizeof(buffer), "[0,%d]", args[0] + args[1]);
        romiSerial->send(buffer);
}

void handler_with_string_argument_returning_value(RomiSerial *romiSerial,
                                                  int16_t *args,
                                                  const char *string_arg)
{
        static char buffer[32];
        snprintf(buffer, sizeof(buffer), "[0,\"%s\"]", string_arg);
        romiSerial->send(buffer);
}

void handler_returning_an_error(RomiSerial *romiSerial,
                                int16_t *args,
                                const char *string_arg)
{
        romiSerial->send_error(100, "Just messing with you");
}

const static MessageHandler handlers[] = {
        { 'a', 0, false, handler_without_arguments_returning_string },
        { 'b', 2, false, handler_with_two_arguments },
        { 'c', 1, true, handler_with_int_and_string_argument },
        { 'd', 2, false, handler_with_two_arguments_returning_value },
        { 'e', 0, true, handler_with_string_argument_returning_value },
        { 'f', 0, false, handler_returning_an_error },
};

RomiSerial romiSerial(handlers, 6);

void setup()
{
        romiSerial.log("Ready");
}

void loop()
{
        romiSerial.handle_input();
}
