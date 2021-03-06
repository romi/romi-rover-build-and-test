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

#include "RoverOptions.h"
#include "UIFactory.h"
#include "DefaultSpeedController.h"
#include "UserInterface.h"
#include "DefaultEventTimer.h"
#include "ScriptList.h"
#include "ScriptMenu.h"
#include "FluidSoundNotifications.h"
#include "FakeWeeder.h"
#include "RoverScriptEngine.h"
#include "UIStateMachine.h"
#include "Rover.h"

using namespace romi;


const char *get_sound_font_in_config(JsonCpp& config)
{
        try {
                return config["user-interface"]["fluid-sounds"]["sound-font"];
                
        } catch (JSONError& je) {
                r_err("FluidSoundNotification: Failed to read the config: %s",
                      je.what());
                throw std::runtime_error("No soundfont in the config file");
        }
}

const char *get_sound_font_file(Options& options, JsonCpp& config)
{
        const char *file = options.get_value("notifications-sound-font");
        if (file == 0)
                file = get_sound_font_in_config(config);

        return file;
}

int main(int argc, char** argv)
{
        int retval = 1;
        
        GetOpt options(rover_options, rover_options_length);
        options.parse(argc, argv);
        
        app_init(&argc, argv);
        app_set_name("user-interface");

        try {
                r_debug("UserInterface: Using configuration file: '%s'",
                        options.get_value("config"));

                UIFactory ui_factory;
                JsonCpp config = JsonCpp::load(options.get_value("config"));

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

                UIStateMachine state_machine(rover);
                UserInterface user_interface(rover, state_machine);

                state_machine.handle_event(event_start);
                
                while (!app_quit()) {
                        
                        try {
                                user_interface.handle_events();
                        
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
