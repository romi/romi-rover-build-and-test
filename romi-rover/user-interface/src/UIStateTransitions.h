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
#ifndef __ROMI_UI_STATE_TRANSITIONS_H
#define __ROMI_UI_STATE_TRANSITIONS_H

namespace romi {

        class UserInterface;
        
        void initialize_rover(UserInterface& ui);

        void stop_rover(UserInterface& ui);
        void reset_rover(UserInterface& ui);
        void continue_rover(UserInterface& ui);
        
        void confirm_navigation_step_1(UserInterface& ui);
        void confirm_navigation_step_2(UserInterface& ui);
        void initialize_navigation(UserInterface& ui);        
        void leave_navigation_mode(UserInterface& ui);        
        void start_driving_forward(UserInterface& ui);
        void drive_forward(UserInterface& ui);
        void start_driving_backward(UserInterface& ui);
        void drive_backward(UserInterface& ui);
        void stop_driving(UserInterface& ui);
        void start_driving_forward_accurately(UserInterface& ui);
        void drive_forward_accurately(UserInterface& ui);
        void start_driving_backward_accurately(UserInterface& ui);
        void drive_backward_accurately(UserInterface& ui);
        void start_spinning(UserInterface& ui);
        void spin(UserInterface& ui);
        
        void confirm_menu_step_1(UserInterface& ui);
        void confirm_menu_step_2(UserInterface& ui);
        void open_menu(UserInterface& ui);        
        void show_next_menu(UserInterface& ui);
        void show_previous_menu(UserInterface& ui);
        void show_current_menu(UserInterface& ui);
        void select_menu(UserInterface& ui);
        void execute_menu(UserInterface& ui);
        void leave_menu_mode(UserInterface& ui);        
}

#endif // __ROMI_UI_STATE_TRANSITIONS_H
