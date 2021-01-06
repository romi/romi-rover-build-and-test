#include <exception>
#include <stdexcept>
#include <string.h>
#include <rcom.h>

#include "Oquam.h"
#include "OquamOptions.h"
#include "OquamFactory.h"
#include "DebugWeedingSession.h"
#include "RPCServer.h"
#include "CNCAdaptor.h"
#include "StepperSettings.h"
#include "RoverOptions.h"

using namespace romi;

static inline double sign(double v)
{
        return (v < 0)? -1.0 : 1.0;
}

int main(int argc, char** argv)
{
        int retval = 1;

        GetOpt options(rover_options, rover_options_length);
        options.parse(argc, argv);
        
        app_init(&argc, argv);
        app_set_name("oquam");

        try {
                r_debug("Oquam: Using configuration file: '%s'",
                        options.get_value("config"));
                
                OquamFactory factory;
                JsonCpp config = JsonCpp::load(options.get_value("config"));
                
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

