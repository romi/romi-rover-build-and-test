/*
  Romi motor controller for brushed mortors

  Copyright (C) 2018-2019 Sony Computer Science Laboratories
  Author(s) Peter Hanappe

  The Romi Brush Controller is free software: you can
  redistribute it and/or modify it under the terms of the GNU Lesser
  General Public License as published by the Free Software Foundation,
  either version 3 of the License, or (at your option) any later
  version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see
  <http://www.gnu.org/licenses/>.

 */
#include <RomiSerial.h>
#include <ArduinoSerial.h>
#include "CrystalDisplay.h"
#include "pins.h"

using namespace romiserial;

void send_info(IRomiSerial *romiSerial, int16_t *args, const char *string_arg);
void handle_show(IRomiSerial *romiSerial, int16_t *args, const char *string_arg);
void handle_clear(IRomiSerial *romiSerial, int16_t *args, const char *string_arg);

const static MessageHandler handlers[] = {
        { '?', 0, false, send_info },
        { 'S', 1, true, handle_show },
        { 'C', 1, false, handle_clear },
};

ArduinoSerial serial(Serial);
RomiSerial romiSerial(serial, serial, handlers, sizeof(handlers) / sizeof(MessageHandler));
CrystalDisplay display(PIN_RS,  PIN_EN,  PIN_D4,  PIN_D5,  PIN_D6,  PIN_D7);

void send_info(IRomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        romiSerial->send("[0,\"CrystalDisplay\",\"0.1\",\"" __DATE__ " " __TIME__ "\"]"); 
}

void handle_show(IRomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        int line = args[0];
        if (line >= 0 && line < display.count_lines()) {
                display.show(line, string_arg);
                romiSerial->send_ok();
        } else {
                romiSerial->send_error(1, "Invalid line");
        }
}

void handle_clear(IRomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        int line = args[0];
        if (line >= 0 && line < display.count_lines()) {
                display.clear(line);
                romiSerial->send_ok();
        } else {
                romiSerial->send_error(1, "Invalid line");
        }
}

void setup()
{
        Serial.begin(115200);
}

void loop()
{
        romiSerial.handle_input();        
        delay(1);
}
