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
#include <string.h>
#include <rcom.h>

#include <DefaultSpeedController.h>
#include <DefaultEventTimer.h>
#include <ScriptList.h>
#include <ScriptMenu.h>
#include <FluidSoundNotifications.h>
#include <rover/Rover.h>
#include <rover/RoverInterface.h>
#include <rover/RoverOptions.h>
#include <rover/RoverScriptEngine.h>
#include <rover/RoverStateMachine.h>
#include <fake/FakeWeeder.h>

#include "UIFactory.h"

using namespace romi;


const char *get_sound_font_in_config(JsonCpp& config)
{
        try {
                return (const char *)config["user-interface"]["fluid-sounds"]["soundfont"];
                
        } catch (JSONError& je) {
                r_err("FluidSoundNotification: Failed to read the config: %s",
                      je.what());
                throw std::runtime_error("No soundfont in the config file");
        }
}

const char *get_sound_font_file(Options& options, JsonCpp& config)
{
        const char *file = options.get_value("notifications-sound-font");
        if (file == nullptr)
                file = get_sound_font_in_config(config);

        return file;
}

const char *get_config_file(Options& options)
{
        const char *file = options.get_value(RoverOptions::config);
        if (file == nullptr) {
                throw std::runtime_error("No configuration file was given (can't run without one...).");
        }
        return file;
}


int main(int argc, char** argv)
{
        int retval = 1;
        
        RoverOptions options;
        options.parse(argc, argv);
        options.exit_if_help_requested();
        
        app_init(&argc, argv);
        app_set_name("user-interface");

        try {
                const char *config_file = get_config_file(options);
                r_info("User-interface: Using configuration file: '%s'", config_file);
                JsonCpp config = JsonCpp::load(config_file);

                UIFactory ui_factory;

                InputDevice& input_device = ui_factory.create_input_device(options,
                                                                           config);

                Display& display = ui_factory.create_display(options, config);
                display.show(0, "Initializing");

                Navigation& navigation = ui_factory.create_navigation(options,
                                                                      config);
                
                DefaultSpeedController speed_controller(navigation, config);
                DefaultEventTimer event_timer(event_timer_timeout);

                ScriptList scripts(ui_factory.get_script_file(options, config));
                ScriptMenu menu(scripts);
                RoverScriptEngine script_engine(scripts, event_script_finished,
                                                event_script_error);

                const char *soundfont = get_sound_font_file(options, config);
                JsonCpp sound_setup = config["user-interface"]["fluid-sounds"]["sounds"];
                FluidSoundNotifications notifications(soundfont, sound_setup);

                Weeder& weeder = ui_factory.create_weeder(options, config);
                
                Rover rover(input_device,
                            display,
                            speed_controller,
                            navigation,
                            event_timer,
                            menu,
                            script_engine,
                            notifications,
                            weeder);

                RoverStateMachine state_machine(rover);
                RoverInterface interface(rover, state_machine);

                state_machine.handle_event(event_start);
                
                while (!app_quit()) {
                        
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
