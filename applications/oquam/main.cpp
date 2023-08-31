#include <exception>
#include <stdexcept>
#include <string>
#include <atomic>
#include <syslog.h>

#include <rcom/Linux.h>
#include <rcom/RcomServer.h>
#include <rcom/RegistryServer.h>

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
#include "util/Clock.h"
#include "util/ClockAccessor.h"

std::atomic<bool> quit(false);
static const char *kRegistry = "registry";

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
        std::shared_ptr<romi::IClock> clock = std::make_shared<romi::Clock>();
        romi::ClockAccessor::SetInstance(clock);

        int retval = 1;

        romi::RoverOptions options;
        options.parse(argc, argv);
        options.exit_if_help_requested();

        log_init();
        log_set_application("oquam");

        std::signal(SIGSEGV, SignalHandler);
        std::signal(SIGINT, SignalHandler);

        try {
	        
                if (options.is_set(kRegistry)) {
                        std::string ip = options.get_value(kRegistry);
                        r_info("Registry IP set to %s", ip.c_str());
                        rcom::RegistryServer::set_address(ip.c_str());
                }
		
                std::string config_file = options.get_config_file();
                r_info("Oquam: Using configuration file: '%s'", config_file.c_str());
                // TBD: USE FileUtils
                std::ifstream ifs(config_file);
                nlohmann::json config = nlohmann::json::parse(ifs);

                // Session
                rcom::Linux linux;
                romi::RomiDeviceData romiDeviceData("Oquam", "001"); // FIXME: from config
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
                
                nlohmann::json range_data = config.at("oquam").at("cnc-range");
                romi::CNCRange range(range_data);

                nlohmann::json stepper_data = config.at("oquam").at("stepper-settings");
                romi::StepperSettings stepper_settings(stepper_data);
        
                double slice_duration = (double) config["oquam"]["path-slice-duration"];
                double maximum_deviation = (double) config["oquam"]["path-maximum-deviation"];

                romi::ICNCController& controller = factory.create_controller(options, config);

                double max_steps_per_block = 32000.0; // Should be less than 2^15/2-1
                double max_slice_duration = stepper_settings.compute_minimum_duration(max_steps_per_block);
                
                romi::AxisIndex homing_axes[3] = { romi::kAxisX, romi::kAxisY, romi::kAxisZ };

                nlohmann::json homing_settings = config["oquam"]["homing"];
                homing_axes[0] = homing_settings["axes"][0];
                homing_axes[1] = homing_settings["axes"][1];
                homing_axes[2] = homing_settings["axes"][2];
                        
                romi::OquamSettings oquam_settings(range,
                                                   stepper_settings.maximum_speed,
                                                   stepper_settings.maximum_acceleration,
                                                   stepper_settings.steps_per_meter,
                                                   maximum_deviation,
                                                   slice_duration,
                                                   max_slice_duration,
                                                   homing_axes);
                romi::Oquam oquam(controller, oquam_settings, session);

                // RPC access
                romi::CNCAdaptor adaptor(oquam);
                auto server = rcom::RcomServer::create("cnc", adaptor);
                
                while (!quit) {
                        server->handle_events();
                        clock->sleep(0.1);
                }
                
                retval = 0;
                
        } catch (nlohmann::json::exception& je) {
                r_err("main: Failed to read the config file: %s", je.what());
                
        } catch (std::exception& e) {
                r_err("main: std::exception: %s", e.what());
        }

        return retval;
}

