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
#include <r.h>
#include "Joystick.h"
#include "SpeedController.h"
#include "Display.h"
#include "EventsAndStates.h"
#include "UserInterface.h"

namespace romi {
        
        void navigation_ready(UserInterface& ui)
        {
                r_debug("NavigationReady");
                ui.speed_controller.stop();
                ui.display.show(0, "Ready");
        }

        void start_driving_forward(UserInterface& ui)
        {
                r_debug("start moving forward");
        }

        void drive_forward(UserInterface& ui)
        {
                r_debug("update forward speed and direction");
                double speed = ui.input_device.get_forward_speed();
                double direction = ui.input_device.get_direction();
                if (speed < 0.0)
                        speed = 0.0;
                ui.speed_controller.drive_at(speed, direction);
        }

        void start_driving_backward(UserInterface& ui)
        {
                r_debug("start moving backward");
        }
        
        void drive_backward(UserInterface& ui)
        {
                r_debug("update backward speed and direction");
                double speed = ui.input_device.get_backward_speed();
                double direction = ui.input_device.get_direction();
                if (speed < 0.0)
                        speed = 0.0;
                ui.speed_controller.drive_at(-speed, direction);
        }

        void stop_driving(UserInterface& ui)
        {
                r_debug("stop");
                ui.speed_controller.stop();
        }
        
        void start_driving_forward_accurately(UserInterface& ui)
        {
                r_debug("start moving forward accurately");
        }
        
        void drive_forward_accurately(UserInterface& ui)
        {
                r_debug("update accurate forward speed and direction");
                double speed = ui.input_device.get_forward_speed();
                double direction = ui.input_device.get_direction();
                if (speed < 0.0)
                        speed = 0.0;
                ui.speed_controller.drive_accurately_at(speed, direction);
        }

        void start_driving_backward_accurately(UserInterface& ui)
        {
                r_debug("start moving backward accurately");
        }
        
        void drive_backward_accurately(UserInterface& ui)
        {
                r_debug("update accurate backward speed and direction");
                double speed = ui.input_device.get_backward_speed();
                double direction = ui.input_device.get_direction();
                if (speed < 0.0)
                        speed = 0.0;
                ui.speed_controller.drive_accurately_at(-speed, direction);
        }
        
        void start_spinning(UserInterface& ui)
        {
                r_debug("start spinning");
        }
        
        void spin(UserInterface& ui)
        {
                r_debug("spinning");
                ui.speed_controller.spin(ui.input_device.get_direction());
        }
        
}
