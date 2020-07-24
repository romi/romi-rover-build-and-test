/*
  romi-rover

  Copyright (C) 2019-2020 Sony Computer Science Laboratories
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
#include "ControlPanelTransitions.h"
#include "pins.h"

int StartUp::doTransition()
{
        _arduino->digitalWrite(PIN_RELAY1, HIGH);
        _arduino->digitalWrite(PIN_RELAY2, LOW);
        return 0;
}

int PowerUp::doTransition()
{
        _arduino->digitalWrite(PIN_RELAY1, HIGH);
        _arduino->digitalWrite(PIN_RELAY2, HIGH);
        return 0;
}

int Shutdown::doTransition()
{
        _arduino->digitalWrite(PIN_RELAY1, HIGH);
        _arduino->digitalWrite(PIN_RELAY2, LOW);
        _timer->setTimeout(5000, EVENT_POWERDOWN);
        return 0;
}

int SoftPowerDown::doTransition()
{
        _arduino->digitalWrite(PIN_RELAY1, LOW);
        _arduino->digitalWrite(PIN_RELAY2, LOW);
        return 0;
}

int HardPowerDown::doTransition()
{
        _arduino->digitalWrite(PIN_RELAY1, LOW);
        _arduino->digitalWrite(PIN_RELAY2, LOW);
        return 0;
}
