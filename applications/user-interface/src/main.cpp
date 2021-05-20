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
#include <atomic>
#include <syslog.h>

#include <rover/SpeedController.h>
#include <api/EventTimer.h>
#include <ui/ScriptList.h>
#include <ui/ScriptMenu.h>
#include <notifications/FluidSoundNotifications.h>
#include "configuration/ConfigurationProvider.h"
#include <rover/Rover.h>
#include <rover/RoverInterface.h>
#include <rover/RoverOptions.h>
#include <rover/RoverScriptEngine.h>
#include <rover/RoverStateMachine.h>
#include <fake/FakeWeeder.h>
#include <camera/FakeImager.h>
#include <data_provider/RomiDeviceData.h>
#include <data_provider/SoftwareVersion.h>
#include <session/Session.h>
#include <data_provider/Gps.h>
#include <data_provider/GpsLocationProvider.h>

#include "UIFactory.h"

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
        int retval = 1;

        romi::RoverOptions options;
        options.parse(argc, argv);
        options.exit_if_help_requested();
        r_log_init();
        r_log_set_app("user-interface");

        std::signal(SIGSEGV, SignalHandler);
        std::signal(SIGINT, SignalHandler);

        // TBD: Check with Peter.
//        app_init(&argc, argv);
//        app_set_name("user-interface");

        try {
                std::string config_file = options.get_config_file();
                r_info("Navigation: Using configuration file: '%s'", config_file.c_str());
                JsonCpp config = JsonCpp::load(config_file.c_str());

                romi::UIFactory ui_factory;

                romi::IInputDevice& input_device = ui_factory.create_input_device(options,
                                                                            config);

                romi::IDisplay& display = ui_factory.create_display(options, config);
                display.show(0, "Initializing");

                romi::INavigation& navigation = ui_factory.create_navigation(options,
                                                                       config);

                romi::SpeedController speed_controller(navigation, config);
                romi::EventTimer event_timer(romi::event_timer_timeout);

                romi::ScriptList scripts(get_script_file(options, config));
                romi::ScriptMenu menu(scripts);
                romi::RoverScriptEngine script_engine(scripts, romi::event_script_finished,
                                                      romi::event_script_error);

                std::string soundfont = get_sound_font_file(options, config);
                JsonCpp sound_setup = config["user-interface"]["fluid-sounds"]["sounds"];
                romi::FluidSoundNotifications notifications(soundfont, sound_setup);

                romi::IWeeder& weeder = ui_factory.create_weeder(options, config);


                rpp::Linux linux;
                romi::RomiDeviceData romiDeviceData;
                romi::SoftwareVersion softwareVersion;
                romi::Gps gps;
                std::unique_ptr<romi::ILocationProvider> locationPrivider
                        = std::make_unique<romi::GpsLocationProvider>(gps);
                std::string session_directory = romi::get_session_directory(options, config);

                romi::Session session(linux, session_directory, romiDeviceData,
                                      softwareVersion, std::move(locationPrivider));
                
                romi::FakeImager imager;

                romi::Rover rover(input_device,
                                  display,
                                  speed_controller,
                                  navigation,
                                  event_timer,
                                  menu,
                                  script_engine,
                                  notifications,
                                  weeder,
                                  imager);
                
                romi::RoverStateMachine state_machine(rover);
                romi::RoverInterface interface(rover, state_machine);

                state_machine.handle_event(romi::event_start);

                while (!quit) {
                        
                        try {
                                interface.handle_events();
                        
                        } catch (std::exception& e) {
                                
                                navigation.stop();
                                throw e;
                        }
                }

                retval = 0;

                
        } catch (std::exception& e) {
                r_err("main: std::exception: %s", e.what());
        }
        
        return retval;
}
