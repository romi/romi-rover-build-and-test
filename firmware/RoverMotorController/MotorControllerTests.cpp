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
#include <Arduino.h>
#include "MotorControllerTests.h"

void test_pi_controller(IMotorController& controller);

int ask_test_id()
{
        Serial.println("Press enter the number of the test to run: ");
        Serial.println("8. Test PI controller");
        Serial.println("9. Stop");

        int test = 0;
        while (test <= 0) {
                test = Serial.parseInt();
        }
        
        Serial.print("Running test ");
        Serial.println(test);

        return test;
}

bool run_test(IMotorController& controller, int test)
{
        bool quit = false;
        
        switch (test){
        case 8:
                test_pi_controller(controller);
                break;

        case 9:
                controller.stop();
                quit = true;
                break;
                
        default:
                Serial.println("Unknown test");
                break;
        }

        return quit;
}

void show_tests_menu(IMotorController& controller)
{
        bool quit = false;

        while (!quit) {
                int test = ask_test_id();
                quit = run_test(controller, test);
        }
}

void run_tests(IMotorController& controller, int num)
{
        if (num == 0)
                show_tests_menu(controller);
        else
                run_test(controller, num);
}

void test_pi_controller(IMotorController& controller)
{
        int update_counter = 0;
        int max_updates = 400;
        int speed_counter = 0;
        int max_speed_changes = 5;
        int min_speed = 50;
        int max_speed = 500;
        
        PIController& left_controller = controller.left_controller();
        PIController& right_controller = controller.right_controller();
        SpeedEnvelope& left_envelope = controller.left_speed_envelope();
        SpeedEnvelope& right_envelope = controller.right_speed_envelope();
        
        int16_t target = random(min_speed, max_speed);
        controller.set_target_speeds(target, target);
         
        while (true)  {
                if (controller.update()) {
                        
                        Serial.print(millis());
                        Serial.print(' ');
                        Serial.print(left_envelope.target_);
                        Serial.print(' ');
                        Serial.print(right_envelope.target_);
                        Serial.print(' ');
                        Serial.print(left_envelope.current_);
                        Serial.print(' ');
                        Serial.print(right_envelope.current_);
                        Serial.print(' ');
                        Serial.print(left_controller.delta_target_);
                        Serial.print(' ');
                        Serial.print(right_controller.delta_target_);
                        Serial.print(' ');
                        Serial.print(left_controller.delta_);
                        Serial.print(' ');
                        Serial.print(right_controller.delta_);
                        Serial.print(' ');
                        Serial.print(left_controller.error_);
                        Serial.print(' ');
                        Serial.print(right_controller.error_);
                        Serial.print(' ');
                        Serial.print(left_controller.sum_);
                        Serial.print(' ');
                        Serial.print(right_controller.sum_);
                        Serial.print(' ');
                        Serial.print(left_controller.max_sum_);
                        Serial.print(' ');
                        Serial.print(right_controller.max_sum_);
                        Serial.print(' ');
                        Serial.print(left_controller.p_);
                        Serial.print(' ');
                        Serial.print(right_controller.p_);
                        Serial.print(' ');
                        Serial.print(left_controller.i_);
                        Serial.print(' ');
                        Serial.print(right_controller.i_);
                        Serial.print(' ');
                        Serial.print(left_controller.out_);
                        Serial.print(' ');
                        Serial.print(right_controller.out_);
                        Serial.print(' ');
                        Serial.print(left_controller.pulsewidth_);
                        Serial.print(' ');
                        Serial.print(right_controller.pulsewidth_);
                        Serial.println();
        
                        update_counter++;
                        if (update_counter == max_updates) {
                                update_counter = 0;
                                if (++speed_counter == max_speed_changes)
                                        break;
                                target = random(min_speed, max_speed);
                                controller.set_target_speeds(target, target);
                        }                        
                }
        }
        
        controller.set_target_speeds(0, 0);
        
        int wait = 0;
        while (wait < 100) {
                if (controller.update())
                        wait++;
        }
        
        controller.stop();
        Serial.println("Done");
}
