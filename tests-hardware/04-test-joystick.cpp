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
#include <rcom.h>

#include <RSerial.h>
#include <RomiSerialClient.h>

#include <Linux.h>
#include <rover/Rover.h>
#include <rover/RoverOptions.h>
#include <rover/RoverInterface.h>
#include <rover/RoverStateMachine.h>
#include <SpeedController.h>
#include <EventTimer.h>
#include <ScriptList.h>
#include <ScriptMenu.h>
#include <Navigation.h>
#include <JoystickInputDevice.h>
#include <fake/FakeWeeder.h>
#include <fake/FakeScriptEngine.h>
#include <fake/FakeNotifications.h>
#include <api/IDisplay.h>
#include <LinuxJoystick.h>
#include <UIEventMapper.h>
#include <CrystalDisplay.h>
#include <JoystickInputDevice.h>
#include <BrushMotorDriver.h>

#include "Clock.h"
#include "ClockAccessor.h"

using namespace std;
using namespace rpp;
using namespace romi;


// TBD: Duplicated functions here and in other main.cpp files.
const char *get_config_file(Options& options)
{
        const char *file = options.get_value(RoverOptions::config);
        if (file == nullptr) {
                throw std::runtime_error("No configuration file was given (can't run without one...).");
        }
        return file;
}


const char *get_script_file(Options& options, JsonCpp& config)
{
        const char *file = options.get_value(RoverOptions::script);
        if (file == nullptr) {
                file = (const char *) config["user-interface"]["script-engine"]["script-file"];
        }
        return file;
}


int main(int argc, char** argv)
{
        std::shared_ptr<rpp::IClock> clock = std::make_shared<rpp::Clock>();
        rpp::ClockAccessor::SetInstance(clock);

        int retval = 1;
        
        RoverOptions options;
        options.parse(argc, argv);
        options.exit_if_help_requested();
        
        
        app_init(&argc, argv);
        app_set_name("romi-rover");
        
        try {
                const char *config_file = get_config_file(options);
                r_info("Romi Rover: Using configuration file: '%s'", config_file);
                JsonCpp config = JsonCpp::load(config_file);
                
                // Display
                const char *display_device = (const char *) config["ports"]["crystal-display"]["port"];
                std::shared_ptr<RSerial>display_serial = std::make_shared<RSerial>(display_device, 115200, 1);
                RomiSerialClient display_romiserial(display_serial, display_serial);
                CrystalDisplay display(display_romiserial);
                display.show(0, "Initializing");
                
                // Joystick
                Linux linux;
                const char *joystick_device = (const char *) config["ports"]["joystick"]["port"];
                LinuxJoystick joystick(linux, joystick_device);
                UIEventMapper joystick_event_mapper;
                JoystickInputDevice input_device(joystick, joystick_event_mapper);
                

                FakeWeeder weeder;
                
                // Navigation
                JsonCpp rover_settings = config["navigation"]["rover"];
                NavigationSettings rover_config(rover_settings);
                const char *driver_device = (const char *) config["ports"]["brush-motor-driver"]["port"];
                JsonCpp driver_settings = config["navigation"]["brush-motor-driver"];
                std::shared_ptr<RSerial>driver_serial = std::make_shared<RSerial>(driver_device, 115200, 1);
                RomiSerialClient driver_romiserial(driver_serial, driver_serial);
                BrushMotorDriver driver(driver_romiserial, driver_settings,
                                        static_cast<int>(rover_config.encoder_steps),
                                        rover_config.max_revolutions_per_sec);
                Navigation navigation(driver, rover_config);

                // SpeedController
                SpeedController speed_controller(navigation, config);

                // EventTimer
                EventTimer event_timer(event_timer_timeout);

                // Script engine
                ScriptList scripts(get_script_file(options, config));
                ScriptMenu menu(scripts);
                FakeScriptEngine script_engine(scripts, event_script_finished);

                // Notifications
                FakeNotifications notifications;

                // Rover
                Rover rover(input_device,
                            display,
                            speed_controller,
                            navigation,
                            event_timer,
                            menu,
                            script_engine,
                            notifications,
                            weeder);

                // State machine
                RoverStateMachine state_machine(rover);

                // User interface
                RoverInterface user_interface(rover, state_machine);

                
                if (!state_machine.handle_event(event_start))
                        // FIXME: should not quit but display something
                        throw std::runtime_error("start-up failed");
                
                while (!app_quit()) {
                        
                        try {
                                user_interface.handle_events();
                        
                        } catch (exception& e) {
                                
                                navigation.stop();
                                throw e;
                        }
                }

                retval = 0;

                
        } catch (JSONError& je) {
                r_err("main: Failed to read the configuration file: %s", je.what());
                
        } catch (exception& e) {
                r_err("main: exception: %s", e.what());
        }
        
        return retval;
}
