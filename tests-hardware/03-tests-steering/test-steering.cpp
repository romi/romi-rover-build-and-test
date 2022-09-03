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
#include <exception>
#include <atomic>
#include <csignal>
#include <string>
#include <iostream>
#include <stdexcept>
#include <math.h>
#include <syslog.h>
#include <fstream>

#include <rpc/RcomServer.h>
#include <RomiSerialClient.h>
#include <RSerial.h>
#include <util/ClockAccessor.h>
#include <util/Logger.h>
#include <hal/BrushMotorDriver.h>
#include <rover/Navigation.h>
#include <rover/NavigationSettings.h>
#include <rover/ZeroNavigationController.h>
#include <rover/LocationTracker.h>
#include <configuration/ConfigurationProvider.h>
#include <data_provider/RomiDeviceData.h>
#include <data_provider/SoftwareVersion.h>
#include <data_provider/Gps.h>
#include <data_provider/GpsLocationProvider.h>
#include <rover/RoverOptions.h>
#include <rpc/NavigationAdaptor.h>
#include <fake/FakeMotorDriver.h>
#include <oquam/StepperController.h>
#include <oquam/StepperSettings.h>

std::atomic<bool> quit(false);

void SignalHandler(int signal)
{
        if (signal == SIGSEGV){
                syslog(1, "rcom-registry segmentation fault");
                exit(signal);
        }
        else if (signal == SIGINT){
                r_info("Ctrl-C Quitting Application");
                perror("init_signal_handler");
                quit = true;
        }
        else{
                r_err("Unknown signam received %d", signal);
        }
}

static const char *kSpeedString = "speed";

struct Steering
{
        double width_;
        double length_;
        double left_speed_;
        double right_speed_;
        double left_angle_;
        double right_angle_;

        Steering(double width, double length)
                : width_(width),
                  length_(length),
                  left_speed_(0.0),
                  right_speed_(0.0),
                  left_angle_(0.0),
                  right_angle_(0.0) {
        }

        void forward(double speed) {
                left_speed_ = speed;
                right_speed_ = speed;
                left_angle_ = 0.0;
                right_angle_ = 0.0;
        }

        void turn(double speed, double radius) {
                if (-width_ / 2.0 <= radius && radius <= width_ / 2.0) {
                        r_err("Unhandled radius: %.3f [R<%.3f or R>%.3f]",
                              radius, -width_/2.0, width_/2.0);
                        throw std::runtime_error("Unhandled value for the radius");
                }
                left_speed_ = speed * (1 - width_ / (2.0 * radius));
                right_speed_ = speed * (1 + width_ / (2.0 * radius));
                left_angle_ = atan(length_ / (radius - width_ / 2.0));
                right_angle_ = atan(length_ / (radius + width_ / 2.0));
        }
};


int main(int argc, char** argv)
{
        
        romi::RoverOptions options;
        // romi::Option distance_option = { kDistanceString, true, "1.0",
        //                                  "The travel distance" };
        // options.add_option(distance_option);
        
        romi::Option speed_option = { kSpeedString, true, "0.1", "The travel speed" };
        options.add_option(speed_option);
        
        // romi::Option ctrl_option = { kControllerString, true, kDriverString,
        //                              "The controller to use (brush-motor-controller "
        //                              "or navigation)" };
        // options.add_option(ctrl_option);
        
        options.parse(argc, argv);
        options.exit_if_help_requested();

        std::string speed_value = options.get_value(kSpeedString);
        double speed = std::stod(speed_value);

        if (speed < -1.0 || speed > 1.0) {
                r_err("Invalid speed: %f (must be in [-1, 1] range", speed);
                return 1;
        }
        
        r_info("Navigating at %.1f% m/s", speed);
        
        log_init();
        log_set_application("navigation");

        std::signal(SIGSEGV, SignalHandler);
        std::signal(SIGINT, SignalHandler);
        
        try {
                std::string config_file = options.get_config_file();
                r_info("Navigation: Using configuration file: '%s'", config_file.c_str());

                // TBD Use FILEUtils Loader.
                std::ifstream ifs(config_file);
                nlohmann::json config = nlohmann::json::parse(ifs);



                // nlohmann::json rover_settings = config["navigation"]["rover"];
                // romi::NavigationSettings rover_config(rover_settings);
                
                // nlohmann::json driver_settings = config["navigation"]["brush-motor-driver"];
                
                // std::string device = romi::get_brush_motor_device(options, config);
                // std::shared_ptr<romiserial::RSerial>serial
                //         = std::make_shared<romiserial::RSerial>(device, 115200, 1);
                // auto romi_serial = romiserial::RomiSerialClient::create(device);
                        
                // romi::BrushMotorDriver driver(romi_serial,
                //                               driver_settings,
                //                               rover_config.compute_max_angular_speed(),
                //                               rover_config.compute_max_angular_acceleration());
                

                
                // const char *steering_device = (const char *) config["ports"]["oquam"]["port"];
                // auto steering_serial = romiserial::RomiSerialClient::create(steering_device);
                // romi::StepperController steering_controller(steering_serial);

                
                //driver.moveat(speed, speed);

                try {
                        
                        std::string line;
                        double R;
                        double max_rpm = 500;
                        double max_rps = max_rpm / 60.0;
                        double default_rps = max_rps / 2.0;
                        double steps_per_revolution = 200; 
                        double microsteps = 8; 
                        double gears = 77.0; 
                        
                        double steering_steps_per_revolution = steps_per_revolution * microsteps * gears;
                        double steering_millis_per_step = 1000.0 / (default_rps * steps_per_revolution * microsteps);

                        //r_debug("steering: s/step=%f, steps/s=%f, ms/step=%f", steering_seconds_per_step, 1.0 / steering_seconds_per_step, steering_millis_per_step);

                        Steering steering(1.0, 1.4);
                        
                        while (true) {
                                std::cout << "R?: ";
                                if (std::getline(std::cin, line)) {
                                        
                                        if (line == "q") {
                                                break;
                                        }
                                        
                                        R = std::stod(line);
                                        if (R == 0) 
                                                steering.forward(speed);
                                        else
                                                steering.turn(speed, R);

                                        int16_t steps_left = (int16_t) (steering.left_angle_ * steering_steps_per_revolution / (2.0 * M_PI)); // TODO: OVERFLOW???
                                        int16_t steps_right = (int16_t) (steering.right_angle_ * steering_steps_per_revolution / (2.0 * M_PI));

                                        int16_t max_steps = steps_left > steps_right? steps_left : steps_right; 
                                        int16_t dt = (int16_t) ceil(steering_millis_per_step * (double) max_steps);
                                                                
                                        r_debug("steering: speed=(%.3f,%.3f), angle=(%.3f,%.3f)=(%.3f°,%.3f°) steps=(%d,%d), dt=%d",
                                                steering.left_speed_, steering.right_speed_,
                                                steering.left_angle_, steering.right_angle_,
                                                180.0 * steering.left_angle_ / M_PI, 180.0 * steering.right_angle_ / M_PI,
                                                (int) steps_left, (int) steps_right,
                                                (int) dt);
                                        
                                        //steering_controller.moveto(dt, steps_left, steps_right, 0); // TODO: duration? max angular speed? Use Oquam for smoothing?
                                        //driver.moveat(steering.left_speed_, steering.right_speed_);
                
                                } else {
                                        break;
                                }
                        }
                } catch (const std::invalid_argument& ia) {
                        std::cerr << "Expected a number: " << ia.what() << std::endl;
                        
                } catch (const std::exception& e) {
                        std::cerr << "Caught exception: " << e.what() << std::endl;
                }

                //steering_controller.moveto(1000, 0, 0, 0);
                romi::ClockAccessor::GetInstance()->sleep(2.0);

                //driver.moveat(0, 0);
                
                
        } catch (std::exception& e) {
                r_err(e.what());
        }

        return 0;
}

