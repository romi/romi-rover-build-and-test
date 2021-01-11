#include <exception>
#include <stdexcept>
#include <string.h>
#include <rcom.h>

#include <RPCServer.h>

#include "OquamFactory.h"
#include "DebugWeedingSession.h"
#include "RoverOptions.h"
#include "rpc/CNCAdaptor.h"
#include "oquam/Oquam.h"
#include "oquam/StepperSettings.h"

using namespace romi;

static inline double sign(double v)
{
        return (v < 0)? -1.0 : 1.0;
}

const char *get_config_file(Options& options)
{
        const char *file = options.get_value(RoverOptions::config);
        if (file == 0) {
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
        app_set_name("oquam");

        try {
                const char *config_file = get_config_file(options);
                r_info("Oquam: Using configuration file: '%s'", config_file);
                JsonCpp config = JsonCpp::load(config_file);
                
                OquamFactory factory;
                
                JsonCpp r = config["oquam"]["cnc-range"];
                CNCRange range(r);

                r = config["oquam"]["stepper-settings"];
                StepperSettings stepper_settings(r);
        
                double slice_duration = config["oquam"]["path-slice-duration"];
                double maximum_deviation = config["oquam"]["path-maximum-deviation"];

                CNCController& controller = factory.create_controller(options, config);

                Oquam oquam(controller, range,
                            stepper_settings.maximum_speed,
                            stepper_settings.maximum_acceleration,
                            stepper_settings.steps_per_meter,
                            maximum_deviation,
                            slice_duration);

                DebugWeedingSession debug(options.get_value("session-directory"), "oquam");
                oquam.set_file_cabinet(&debug);
                
                CNCAdaptor adaptor(oquam);
                rcom::RPCServer server(adaptor, "oquam", "cnc");
                
                while (!app_quit())
                        clock_sleep(0.1);

                retval = 0;
                
        } catch (JSONError& je) {
                r_err("main: Failed to read the config file: %s", je.what());
                
        } catch (std::exception& e) {
                r_err("main: std::exception: %s", e.what());
        }

        return retval;
}

