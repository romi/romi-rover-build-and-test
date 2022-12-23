/*
  Romi motor controller for brushed motors

  Copyright (C) 2021 Sony Computer Science Laboratories
  Author(s) Peter Hanappe

  MotorController is free software: you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation, either version 3 of the
  License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see
  <http://www.gnu.org/licenses/>.

 */
#ifndef _MOTORCONTROLLER_MOTORCONTROLLERCOMMANDS_H
#define _MOTORCONTROLLER_MOTORCONTROLLERCOMMANDS_H

#include <IRomiSerial.h>
#include "IMotorController.h"

void setup_commands(IMotorController *controller, romiserial::IRomiSerial *romiSerial);
void handle_commands();

void send_info(romiserial::IRomiSerial *romiSerial,
               int16_t *args, const char *string_arg);
void send_encoders(romiserial::IRomiSerial *romiSerial,
                   int16_t *args, const char *string_arg);
void handle_configure(romiserial::IRomiSerial *romiSerial,
                      int16_t *args, const char *string_arg);
void handle_enable(romiserial::IRomiSerial *romiSerial,
                   int16_t *args, const char *string_arg);
void handle_moveat(romiserial::IRomiSerial *romiSerial,
                   int16_t *args, const char *string_arg);
void handle_stop(romiserial::IRomiSerial *romiSerial,
                 int16_t *args, const char *string_arg);
void handle_tests(romiserial::IRomiSerial *romiSerial,
                  int16_t *args, const char *string_arg);
void send_speeds(romiserial::IRomiSerial *romiSerial,
                 int16_t *args, const char *string_arg);

// void send_pid(romiserial::IRomiSerial *romiSerial, int16_t *args, const char *string_arg);
// void send_status(romiserial::IRomiSerial *romiSerial, int16_t *args, const char *string_arg);
// void send_configure(romiserial::IRomiSerial *romiSerial, int16_t *args, const char *string_arg);

#endif // _MOTORCONTROLLER_MOTORCONTROLLERCOMMANDS_H
