#include <exception>
#include <stdexcept>
#include <string>
#include <rcom.h>

#include <RPCServer.h>
#include "Linux.h"

#include <rover/RoverOptions.h>
#include <rpc/CNCAdaptor.h>
#include <oquam/Oquam.h>
#include <oquam/StepperSettings.h>
#include "OquamFactory.h"
#include "data_provider/RomiDeviceData.h"
#include "data_provider/SoftwareVersion.h"
#include "weeder_session/Session.h"
#include "data_provider/Gps.h"
#include "data_provider/GpsLocationProvider.h"

#include "Clock.h"
#include "ClockAccessor.h"

using namespace romi;

static inline double sign(double v)
{
        return (v < 0)? -1.0 : 1.0;
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
        std::shared_ptr<rpp::IClock> clock = std::make_shared<rpp::Clock>();
        rpp::ClockAccessor::SetInstance(clock);

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
        
                double slice_duration = (double) config["oquam"]["path-slice-duration"];
                double maximum_deviation = (double) config["oquam"]["path-maximum-deviation"];

                CNCController& controller = factory.create_controller(options, config);

                rpp::Linux linux;
                RomiDeviceData romiDeviceData;
                SoftwareVersion softwareVersion;
                romi::Gps gps;
                std::unique_ptr<ILocationProvider> locationPrivider = std::make_unique<GpsLocationProvider>(gps);
                std::string session_directory = options.get_value("oquam-session");
                romi::Session session(linux, session_directory, romiDeviceData, softwareVersion, std::move(locationPrivider));
                session.start("oquam_observation_id");
                Oquam oquam(controller, range,
                            stepper_settings.maximum_speed,
                            stepper_settings.maximum_acceleration,
                            stepper_settings.steps_per_meter,
                            maximum_deviation,
                            slice_duration,
                            session);

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

