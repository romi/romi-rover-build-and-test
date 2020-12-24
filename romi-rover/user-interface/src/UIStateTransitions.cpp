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
        
        void initialize_rover(UserInterface& ui)
        {
                r_debug("initialize_rover");
                ui.display.show(0, "Ready");
        }

        void stop_rover(UserInterface& ui)
        {
                r_debug("stop_rover");
                ui.saved_state = ui.state_machine.get_state();
                ui.display.show(0, "[] to reset");
                ui.display.show(1, "X  to continue");
        }
        
        void reset_rover(UserInterface& ui)
        {
                r_debug("reset_rover");
        }
        
        void continue_rover(UserInterface& ui)
        {
                r_debug("continue_rover");
        }
        
        void confirm_navigation_step_1(UserInterface& ui)
        {
                r_debug("confirm_navigation_step_1");
                ui.display.show(0, "Hold to navigate");
                ui.event_timer.set_timeout(3.0);
        }

        void confirm_navigation_step_2(UserInterface& ui)
        {
                r_debug("confirm_navigation_step_2");
                ui.display.show(0, "Navigate? (X)");
                ui.event_timer.set_timeout(2.0);
        }

        void initialize_navigation(UserInterface& ui)
        {
                r_debug("initialize_navigation");
                ui.speed_controller.stop();
                ui.display.show(0, "Navigating");
                // set timer to return to main mode
                ui.event_timer.set_timeout(10.0);
        }

        void leave_navigation_mode(UserInterface& ui)
        {
                r_debug("leave_navigation_mode");
                ui.display.show(0, "Ready");
                ui.event_timer.reset(); 
        }

        void start_driving_forward(UserInterface& ui)
        {
                r_debug("start moving forward");
                ui.event_timer.set_timeout(10.0); // restart timer
        }

        void drive_forward(UserInterface& ui)
        {
                r_debug("update forward speed and direction");
                double speed = ui.input_device.get_forward_speed();
                double direction = ui.input_device.get_direction();
                if (speed < 0.0)
                        speed = 0.0;
                ui.speed_controller.drive_at(speed, direction);
                ui.event_timer.set_timeout(10.0); // restart timer
        }

        void start_driving_backward(UserInterface& ui)
        {
                r_debug("start moving backward");
                ui.event_timer.set_timeout(10.0); // restart timer
        }
        
        void drive_backward(UserInterface& ui)
        {
                r_debug("update backward speed and direction");
                double speed = ui.input_device.get_backward_speed();
                double direction = ui.input_device.get_direction();
                if (speed < 0.0)
                        speed = 0.0;
                ui.speed_controller.drive_at(-speed, direction);
                ui.event_timer.set_timeout(10.0); // restart timer
        }

        void stop_driving(UserInterface& ui)
        {
                r_debug("stop");
                ui.speed_controller.stop();
                ui.event_timer.set_timeout(10.0); // restart timer
        }
        
        void start_driving_forward_accurately(UserInterface& ui)
        {
                r_debug("start moving forward accurately");
                ui.event_timer.set_timeout(10.0); // restart timer
        }
        
        void drive_forward_accurately(UserInterface& ui)
        {
                r_debug("update accurate forward speed and direction");
                double speed = ui.input_device.get_forward_speed();
                double direction = ui.input_device.get_direction();
                if (speed < 0.0)
                        speed = 0.0;
                ui.speed_controller.drive_accurately_at(speed, direction);
                ui.event_timer.set_timeout(10.0); // restart timer
        }

        void start_driving_backward_accurately(UserInterface& ui)
        {
                r_debug("start moving backward accurately");
                ui.event_timer.set_timeout(10.0); // restart timer
        }
        
        void drive_backward_accurately(UserInterface& ui)
        {
                r_debug("update accurate backward speed and direction");
                double speed = ui.input_device.get_backward_speed();
                double direction = ui.input_device.get_direction();
                if (speed < 0.0)
                        speed = 0.0;
                ui.speed_controller.drive_accurately_at(-speed, direction);
                ui.event_timer.set_timeout(10.0); // restart timer
        }
        
        void start_spinning(UserInterface& ui)
        {
                r_debug("start spinning");
                ui.event_timer.set_timeout(10.0); // restart timer
        }
        
        void spin(UserInterface& ui)
        {
                r_debug("spinning");
                ui.speed_controller.spin(ui.input_device.get_direction());
                ui.event_timer.set_timeout(10.0); // restart timer
        }

        
        void confirm_menu_step_1(UserInterface& ui)
        {
                r_debug("confirm_menu_step_1");
                ui.display.show(0, "Hold for menu");
                ui.event_timer.set_timeout(3.0);
        }

        void confirm_menu_step_2(UserInterface& ui)
        {
                r_debug("confirm_menu_step_2");
                ui.display.show(0, "Menu? (X)");
                ui.event_timer.set_timeout(2.0);
        }

        void open_menu(UserInterface& ui)
        {
                r_debug("open_menu");
                std::string name;
                ui.menu.first_menu_item(name);
                ui.display.show(0, name.c_str());
                ui.display.clear(1);
                // set timer to return to main mode
                ui.event_timer.set_timeout(10.0);  
        }

        void show_next_menu(UserInterface& ui)
        {
                r_debug("show_next_menu");
                std::string name;
                ui.menu.next_menu_item(name);
                ui.display.show(0, name.c_str());
                ui.event_timer.set_timeout(10.0);  // restart timer
        }

        void show_previous_menu(UserInterface& ui)
        {
                r_debug("show_previous_menu");
                std::string name;
                ui.menu.previous_menu_item(name);
                ui.display.show(0, name.c_str());
                ui.event_timer.set_timeout(10.0); // restart timer
        }

        void show_current_menu(UserInterface& ui)
        {
                r_debug("show_current_menu");
                std::string name;
                ui.menu.current_menu_item(name);
                ui.display.show(0, name.c_str());
                ui.display.clear(1);
        }

        void select_menu(UserInterface& ui)
        {
                r_debug("select_menu");
                ui.display.show(1, "X to confirm");
                ui.event_timer.set_timeout(1.0);
        }
        
        void execute_menu(UserInterface& ui)
        {
                r_debug("confirm_menu");

                std::string id;
                ui.menu.current_menu_item_id(id);
                ui.script_engine.execute_script(id);
                r_debug("execute_menu: %s", id.c_str());
                
                ui.display.show(1, "Running");
        }
        
        void leave_menu_mode(UserInterface& ui)
        {
                r_debug("leave_menu_mode");
                ui.display.show(0, "Ready");
                ui.event_timer.reset(); 
        }
}
