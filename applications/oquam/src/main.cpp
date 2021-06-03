#include <exception>
#include <stdexcept>
#include <string>
#include <atomic>
#include <syslog.h>

#include <rpc/RcomServer.h>
#include "Linux.h"

#include <rover/RoverOptions.h>
#include <rpc/CNCAdaptor.h>
#include <oquam/Oquam.h>
#include <oquam/StepperSettings.h>
#include "OquamFactory.h"
#include "data_provider/RomiDeviceData.h"
#include "data_provider/SoftwareVersion.h"
#include "session/Session.h"
#include "data_provider/Gps.h"
#include "data_provider/GpsLocationProvider.h"

#include "Clock.h"
#include "ClockAccessor.h"

std::atomic<bool> quit(false);

static inline double sign(double v)
{
        return (v < 0)? -1.0 : 1.0;
}

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
        r_log_set_app("oquam");

        std::signal(SIGSEGV, SignalHandler);
        std::signal(SIGINT, SignalHandler);

        try {
                std::string config_file = options.get_config_file();
                r_info("Oquam: Using configuration file: '%s'", config_file.c_str());
                JsonCpp config = JsonCpp::load(config_file.c_str());

                // Session
                rpp::Linux linux;
                romi::RomiDeviceData romiDeviceData;
                romi::SoftwareVersion softwareVersion;
                romi::Gps gps;
                std::unique_ptr<romi::ILocationProvider> locationPrivider
                        = std::make_unique<romi::GpsLocationProvider>(gps);
                std::string session_directory = options.get_value("oquam-session");
                romi::Session session(linux, session_directory,
                                      romiDeviceData, softwareVersion,
                                      std::move(locationPrivider));
                session.start("oquam_observation_id");
                
                // Oquam cnc
                romi::OquamFactory factory;
                
                JsonCpp range_data = config["oquam"]["cnc-range"];
                romi::CNCRange range(range_data);

                JsonCpp stepper_data = config["oquam"]["stepper-settings"];
                romi::StepperSettings stepper_settings(stepper_data);
        
                double slice_duration = (double) config["oquam"]["path-slice-duration"];
                double maximum_deviation = (double) config["oquam"]["path-maximum-deviation"];

                romi::ICNCController& controller = factory.create_controller(options, config);

                double max_steps_per_block = 32000.0; // Should be less than 2^16/2
                double max_slice_duration = stepper_settings.compute_minimum_duration(max_steps_per_block);
                
                romi::AxisIndex homing[3] = { romi::kAxisX, romi::kAxisY, romi::kNoAxis };
                romi::OquamSettings oquam_settings(range,
                                                   stepper_settings.maximum_speed,
                                                   stepper_settings.maximum_acceleration,
                                                   stepper_settings.steps_per_meter,
                                                   maximum_deviation,
                                                   slice_duration,
                                                   max_slice_duration,
                                                   homing);
                romi::Oquam oquam(controller, oquam_settings, session);

                // RPC access
                romi::CNCAdaptor adaptor(oquam);
                auto server = romi::RcomServer::create("cnc", adaptor);
                
                while (!quit) {
                        server->handle_events();
                        clock->sleep(0.1);
                }
                
                retval = 0;
                
        } catch (JSONError& je) {
                r_err("main: Failed to read the config file: %s", je.what());
                
        } catch (std::exception& e) {
                r_err("main: std::exception: %s", e.what());
        }

        return retval;
}

