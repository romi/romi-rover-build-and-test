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
#include <RomiSerial.h>
#include <ArduinoSerial.h>
#include "ArduinoUno.h"
#include "MotorController.h"
#include "MotorControllerCommands.h"

using namespace romiserial;

ArduinoUno arduino_;
MotorController controller_(arduino_, MotorController::kDefaultUpdateInterval);
ArduinoSerial serial(Serial);
RomiSerial romi_serial_(serial, serial);

void setup()
{
        Serial.begin(115200);
        while (!Serial)
                ;
        
        arduino_.setup();
        controller_.setup();
        setup_commands(&controller_, &romi_serial_);
}

void loop()
{
        controller_.update();
        handle_commands();
}
