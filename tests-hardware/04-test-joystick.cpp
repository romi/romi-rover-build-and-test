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
#include <exception>
#include <stdexcept>
#include <memory>
#include <string.h>
#include <atomic>
#include <syslog.h>

#include <RSerial.h>
#include <RomiSerialClient.h>

#include <Linux.h>
#include "configuration/ConfigurationProvider.h"
#include <rover/Rover.h>
#include <rover/RoverOptions.h>
#include <rover/RoverInterface.h>
#include <rover/RoverStateMachine.h>
#include <rover/SpeedController.h>
#include <api/EventTimer.h>
#include <ui/ScriptList.h>
#include <ui/ScriptMenu.h>
#include <rover/Navigation.h>
#include <ui/JoystickInputDevice.h>
#include <fake/FakeWeeder.h>
#include <fake/FakeScriptEngine.h>
#include <fake/FakeNotifications.h>
#include <api/IDisplay.h>
#include <ui/LinuxJoystick.h>
#include <ui/UIEventMapper.h>
#include <ui/CrystalDisplay.h>
#include <ui/JoystickInputDevice.h>
#include <hal/BrushMotorDriver.h>

#include "Clock.h"
#include "ClockAccessor.h"

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

int main(int argc, char** argv)
{
        std::shared_ptr<rpp::IClock> clock = std::make_shared<rpp::Clock>();
        rpp::ClockAccessor::SetInstance(clock);

        int retval = 1;

        romi::RoverOptions options;
        options.parse(argc, argv);
        options.exit_if_help_requested();

        r_log_init();
        r_log_set_app("joystick-test");

        std::signal(SIGSEGV, SignalHandler);
        std::signal(SIGINT, SignalHandler);
        // TBD: Check with Peter.
//        app_init(&argc, argv);
//        app_set_name("romi-rover");
        
        try {
                std::string config_file = options.get_config_file();
                r_info("Navigation: Using configuration file: '%s'", config_file.c_str());
                JsonCpp config = JsonCpp::load(config_file.c_str());
                
                // Display
                const char *display_device = (const char *) config["ports"]["crystal-display"]["port"];
                
                auto display_serial = romiserial::RomiSerialClient::create(display_device);
                romi::CrystalDisplay display(display_serial);
                display.show(0, "Initializing");
                
                // Joystick
                rpp::Linux linux;
                const char *joystick_device = (const char *) config["ports"]["joystick"]["port"];
                romi::LinuxJoystick joystick(linux, joystick_device);
                romi::UIEventMapper joystick_event_mapper;
                romi::JoystickInputDevice input_device(joystick, joystick_event_mapper);


                romi::FakeWeeder weeder;
                
                // Navigation
                JsonCpp rover_settings = config["navigation"]["rover"];
                romi::NavigationSettings rover_config(rover_settings);
                const char *driver_device = (const char *) config["ports"]["brush-motor-driver"]["port"];
                JsonCpp driver_settings = config["navigation"]["brush-motor-driver"];
                auto driver_serial = romiserial::RomiSerialClient::create(driver_device);
                romi::BrushMotorDriver driver(driver_serial, driver_settings,
                                              static_cast<int>(rover_config.encoder_steps),
                                              rover_config.max_revolutions_per_sec);
                romi::Navigation navigation(driver, rover_config);

                // SpeedController
                romi::SpeedController speed_controller(navigation, config);

                // EventTimer
                romi::EventTimer event_timer(romi::event_timer_timeout);

                // Script engine
                romi::ScriptList scripts(get_script_file(options, config));
                romi::ScriptMenu menu(scripts);
                romi::FakeScriptEngine script_engine(scripts, romi::event_script_finished);

                // Notifications
                romi::FakeNotifications notifications;

                // Rover
                romi::Rover rover(input_device,
                            display,
                            speed_controller,
                            navigation,
                            event_timer,
                            menu,
                            script_engine,
                            notifications,
                            weeder);

                // State machine
                romi::RoverStateMachine state_machine(rover);

                // User interface
                romi::RoverInterface user_interface(rover, state_machine);

                
                if (!state_machine.handle_event(romi::event_start))
                        // FIXME: should not quit but display something
                        throw std::runtime_error("start-up failed");

                while (!quit) {
                        
                        try {
                                user_interface.handle_events();
                        
                        } catch (std::exception& e) {
                                
                                navigation.stop();
                                throw;
                        }
                }

                retval = 0;

                
        } catch (JSONError& je) {
                r_err("main: Failed to read the configuration file: %s", je.what());
                
        } catch (std::exception& e) {
                r_err("main: exception: %s", e.what());
        }
        
        return retval;
}
