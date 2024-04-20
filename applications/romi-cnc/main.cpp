#include <exception>
#include <stdexcept>
#include <string>
#include <atomic>
#include <syslog.h>

#include <rcom/Linux.h>
#include <rcom/RcomServer.h>
#include <rcom/RegistryServer.h>
#include <rcom/RcomClient.h>

#include <rpc/RcomLog.h>
#include <rover/RoverOptions.h>
#include <rpc/CNCAdaptor.h>
#include <oquam/Oquam.h>
#include <oquam/StepperSettings.h>
#include <data_provider/RomiDeviceData.h>
#include <data_provider/SoftwareVersion.h>
#include <session/Session.h>
#include <data_provider/Gps.h>
#include <data_provider/GpsLocationProvider.h>
#include <util/Clock.h>
#include <util/ClockAccessor.h>
#include <api/LocalConfig.h>
#include <rpc/RemoteConfig.h>
#include "OquamFactory.h"

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


        std::signal(SIGSEGV, SignalHandler);
        std::signal(SIGINT, SignalHandler);

        try {
                // Linux
                rcom::Linux linux;
                std::shared_ptr<rcom::ILog> rcomlog = std::make_shared<romi::RcomLog>();
	        
                romi::RoverOptions options;
                romi::Option session_dir = {
                        "oquam-session", false, ".",
                        "Set the session directory"}; 
                options.add_option(session_dir);
                options.parse(argc, argv);
                options.exit_if_help_requested();

                log_init();
                log_set_application("oquam");
        
                if (options.is_set(kRegistry)) {
                        std::string ip = options.get_value(kRegistry);
                        r_info("Registry IP set to %s", ip.c_str());
                        rcom::RegistryServer::set_address(ip.c_str());
                }
		
                std::shared_ptr<romi::IConfigManager> config;
                
                if (options.is_set(romi::RoverOptions::kConfig)) {
                        std::string config_value = options.get_value(romi::RoverOptions::kConfig);
                        
                        r_info("romi-camera: Using local configuration file: '%s'",
                               config_value.c_str());
                
                        std::filesystem::path config_path = config_value;
                        config = std::make_shared<romi::LocalConfig>(config_path);
                } else {
                        r_info("romi-camera: Using remote configuration");
                        auto client = rcom::RcomClient::create("config", 10.0, rcomlog);
                        config = std::make_shared<romi::RemoteConfig>(client);
                }

                // std::string config_file = options.get_config_file();
                // r_info("Oquam: Using configuration file: '%s'", config_file.c_str());
                // // TBD: USE FileUtils
                // std::ifstream ifs(config_file);
                // nlohmann::json config = nlohmann::json::parse(ifs);
                
                nlohmann::json cnc_config = config->get_section("cnc");
                
                // Session
                romi::RomiDeviceData romiDeviceData("Oquam", "001"); // FIXME: from config
                romi::SoftwareVersion softwareVersion;
                romi::Gps gps;
                std::unique_ptr<romi::ILocationProvider> locationPrivider
                        = std::make_unique<romi::GpsLocationProvider>(gps);
                std::string session_directory = options.get_value("oquam-session");
                r_info("");
                romi::Session session(linux, session_directory,
                                      romiDeviceData, softwareVersion,
                                      std::move(locationPrivider));
                session.start("oquam_observation_id");
                
                // Oquam cnc
                romi::OquamFactory factory;
                
                nlohmann::json range_data = cnc_config["cnc-range"];
                romi::CNCRange range(range_data);

                nlohmann::json stepper_data = cnc_config["stepper-settings"];
                romi::StepperSettings stepper_settings(stepper_data);
        
                double slice_duration = (double) cnc_config["path-slice-duration"];
                double maximum_deviation = (double) cnc_config["path-maximum-deviation"];

                nlohmann::json ports_config = config->get_section("ports");
                romi::ICNCController& controller = factory.create_controller(options,
                                                                             cnc_config,
                                                                             ports_config);

                double max_steps_per_block = 32000.0; // Should be less than 2^15/2-1
                double max_slice_duration = stepper_settings.compute_minimum_duration(max_steps_per_block);
                
                romi::AxisIndex homing_axes[3] = { romi::kAxisX, romi::kAxisY, romi::kAxisZ };

                nlohmann::json homing_settings = cnc_config["homing"];
                homing_axes[0] = homing_settings["axes"][0];
                homing_axes[1] = homing_settings["axes"][1];
                homing_axes[2] = homing_settings["axes"][2];
                        
                romi::HomingMode homing_mode = (romi::HomingMode) homing_settings["mode"];
                        
                romi::OquamSettings oquam_settings(range,
                                                   stepper_settings.maximum_speed,
                                                   stepper_settings.maximum_acceleration,
                                                   stepper_settings.steps_per_meter,
                                                   maximum_deviation,
                                                   slice_duration,
                                                   max_slice_duration,
                                                   homing_axes,
                                                   homing_mode);
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

