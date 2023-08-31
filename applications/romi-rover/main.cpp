/*
  romi-rover

  Copyright (C) 2019-2020 Sony Computer Science Laboratories
  Author(s) Peter Hanappe

  romi-rover is the main app for the Romi Rover.

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
#include <atomic>
#include <syslog.h>
#include <string.h>

#include <rcom/Linux.h>
#include <rcom/RegistryServer.h>
#include <rcom/RcomClient.h>

#include <RSerial.h>
#include <RomiSerialClient.h>
#include <util/RomiSerialLog.h>
#include <util/Clock.h>
#include <util/ClockAccessor.h>
#include <configuration/ConfigurationProvider.h>
#include <rover/Rover.h>
#include <rover/RoverOptions.h>
#include <rover/RoverInterface.h>
#include <rover/RoverScriptEngine.h>
#include <rover/RoverStateMachine.h>
#include <rover/SpeedController.h>
#include <rover/ZeroNavigationController.h>
#include <rover/L1NavigationController.h>
#include <rover/PythonTrackFollower.h>
#include <rover/LocationTracker.h>
#include <rover/IMUTrackFollower.h>
#include <rover/WheelOdometry.h>
#include <rover/Navigation.h>
#include <rover/DifferentialSteering.h>
#include <rover/StepperSteering.h>
#include <rover/DoubleSteering.h>
#include <rover/ManualTrackFollower.h>
#include <rover/SteeringController.h>
#include <api/EventTimer.h>
#include <ui/ScriptList.h>
#include <ui/ScriptMenu.h>
#include <rpc/ScriptHubListener.h>
#include <battery_monitor/BatteryMonitor.h>
//#include <notifications/FluidSoundNotifications.h>
#include <notifications/DummyNotifications.h>
#include <ui/CrystalDisplay.h>
#include <ui/JoystickInputDevice.h>
#include <weeder/Weeder.h>
#include <api/IDisplay.h>
#include <ui/LinuxJoystick.h>
#include <ui/UIEventMapper.h>
#include <ui/CrystalDisplay.h>
#include <ui/JoystickInputDevice.h>
#include <ui/RemoteStateInputDevice.h>
#include <camera/FileCamera.h>
#include <camera/USBCamera.h>
#include <rpc/RemoteCamera.h>
#include <rpc/RcomLog.h>
#include <hal/BrushMotorDriver.h>
#include <oquam/StepperController.h>
#include <oquam/StepperSettings.h>
#include <oquam/Oquam.h>
#include <weeder/PipelineFactory.h>
#include <camera/Imager.h>
#include <unet/UnetImager.h>
#include <data_provider/RomiDeviceData.h>
#include <data_provider/SoftwareVersion.h>
#include <session/Session.h>
#include <data_provider/Gps.h>
#include <data_provider/GpsLocationProvider.h>
#include <api/DataLog.h>
#include <api/DataLogAccessor.h>
#include <oquam/FakeCNCController.h>


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

double compute_pixels_per_meter(nlohmann::json& config)
{
        double width = config["oquam"]["cnc-range"][0][1];
        double pixels = config["weeder"]["imagecropper"]["workspace"][2];
        r_debug("Pixels-per-meter: %f px/m", pixels / width);
        return pixels / width;
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
        log_set_application("romi-rover");

        std::signal(SIGSEGV, SignalHandler);
        std::signal(SIGINT, SignalHandler);

        try {
                std::string config_file = options.get_config_file();
                r_info("Romi Rover: Using configuration file: '%s'", config_file.c_str());

                // TBD: Create standard JSON load functionality.
                std::ifstream ifs(config_file);
                nlohmann::json config = nlohmann::json::parse(ifs);

                // Registry
                std::string registry = options.get_value(romi::RoverOptions::registry);
                if (!registry.empty())
                        rcom::RegistryServer::set_address(registry.c_str());
                
                rcom::Linux linux;
                auto rcomlog = std::make_shared<romi::RcomLog>();
                        
                // Session
                r_info("main: Creating session");
                romi::RomiDeviceData romiDeviceData("Rover", "0001"); // FIXME: from config
                romi::SoftwareVersion softwareVersion;
                romi::Gps gps;
                std::unique_ptr<romi::ILocationProvider> locationProvider
                        = std::make_unique<romi::GpsLocationProvider>(gps);
                std::string session_directory = romi::get_session_directory(options, config);

                romi::Session session(linux, session_directory, romiDeviceData,
                                      softwareVersion, std::move(locationProvider));
                session.start("hw_observation_id");

                // Datalog
                const std::filesystem::path filepath = session.create_session_file("datalog.txt");
                auto datalog = std::make_shared<romi::DataLog>(filepath);
                romi::DataLogAccessor::set(datalog);
                r_info("main: Storing datalog in %s", filepath.string().c_str());


                // Log redirection for romi serial
                std::shared_ptr<romiserial::ILog> serial_log
                        = std::make_shared<romi::RomiSerialLog>();
                
                // Display
                r_info("main: Creating display");
                std::string display_device = config["ports"]["display-device"]["port"];

                std::string client_name("display_device");
                auto display_serial
                        = romiserial::RomiSerialClient::create(display_device,
                                                               client_name,
                                                               serial_log);
                romi::CrystalDisplay display(display_serial);
                display.clear(0);
                display.clear(1);
                display.show(0, "Initializing");

                // Joystick
                r_info("main: Creating joystick");
                std::string joystick_device = config["ports"]["joystick"]["port"];
                romi::LinuxJoystick joystick(linux, joystick_device);
                romi::UIEventMapper joystick_event_mapper;
                romi::JoystickInputDevice input_device(joystick, joystick_event_mapper);

                // CNC controller
                r_info("main: Creating CNC controller");
                std::string cnc_device = config["ports"]["oquam"]["port"];
                client_name = "cnc_device";
                auto cnc_serial = romiserial::RomiSerialClient::create(cnc_device,
                                                                       client_name,
                                                                       serial_log);
                romi::StepperController cnc_controller(cnc_serial);
                //romi::FakeCNCController cnc_controller;


                std::unique_ptr<romi::BatteryMonitor> battery_mon(nullptr);

                try {
                    // Battery Monitor
                    r_info("main: Creating Battery Monitor");
                    std::string battery_monotor_device = config["ports"]["battery-monitor"]["port"];
                    client_name = "battery-monitor";
                    auto battery_serial
                            = romiserial::RomiSerialClient::create(battery_monotor_device,
                                                                   client_name,
                                                                   serial_log);

                    battery_mon
                            = std::make_unique<romi::BatteryMonitor>(battery_serial,
                                                                     datalog, quit);
                    battery_mon->enable();
                }
                catch (std::exception& e){
                    r_err("main: Unable to create battery monitor: %s", e.what());
                }

                // CNC
                r_info("main: Creating CNC");
                nlohmann::json r = config.at("oquam").at("cnc-range");
                romi::CNCRange range(r);
                nlohmann::json s = config.at("oquam").at("stepper-settings");
                romi::StepperSettings stepper_settings(s);
                double slice_duration = config["oquam"]["path-slice-duration"];
                double maximum_deviation = config["oquam"]["path-maximum-deviation"];

                romi::AxisIndex homing[3] = { romi::kAxisZ, romi::kAxisX, romi::kAxisY };
                //romi::AxisIndex homing[3] = { romi::kNoAxis, romi::kNoAxis, romi::kNoAxis };
                double max_steps_per_block = 32000.0; // Should be less than 2^16/2
                double max_slice_duration = stepper_settings.compute_minimum_duration(max_steps_per_block);
                romi::OquamSettings oquam_settings(range,
                                                   stepper_settings.maximum_speed,
                                                   stepper_settings.maximum_acceleration,
                                                   stepper_settings.steps_per_meter,
                                                   maximum_deviation,
                                                   slice_duration,
                                                   max_slice_duration,
                                                   homing,
                                                   romi::kHomingDefault);
                romi::Oquam oquam(cnc_controller, oquam_settings, session);

                
                // Camera
                // TBD: Use refactored functions. get_camera_class
                r_info("main: Creating camera");
                std::unique_ptr<romi::ICamera> camera;
                std::string camera_classname = get_camera_classname(options, config);
                
                if (camera_classname == romi::FileCamera::ClassName) {
                        std::string image_file = get_camera_image(options, config);
                        r_info("Loading image %s", image_file.c_str());
                        camera = std::make_unique<romi::FileCamera>(image_file);
                        
                } else if (camera_classname == romi::USBCamera::ClassName) {
                        std::string camera_device = get_camera_device(options, config);
                        double width = (double) config["weeder"]["usb-camera"]["width"];
                        double height = (double) config["weeder"]["usb-camera"]["height"];
                        camera = std::make_unique<romi::USBCamera>(camera_device, width, height);
                } else if (camera_classname == romi::RemoteCamera::ClassName) {
                        auto client = rcom::RcomClient::create("camera", 10.0, rcomlog);
                        camera = std::make_unique<romi::RemoteCamera>(client);
                } else {
                        throw std::runtime_error("Unknown camera classname");
                }
                
                // Weeder pipeline
                r_info("main: Creating weeder pipeline");
                romi::PipelineFactory pipeline_factory;
                romi::IPipeline& pipeline = pipeline_factory.build(range, config);

                // Weeder
                r_info("main: Creating weeder");
                double z0 = (double) config["weeder"]["z0"];
                double speed = (double) config["weeder"]["speed"];
                double diameter_tool = (double) config["weeder"]["diameter-tool"];
                romi::Weeder weeder(*camera, pipeline, oquam, z0, speed,
                                    diameter_tool, session);

                // Motor driver
                r_info("main: Creating motor driver");
                nlohmann::json rover_settings = config.at("navigation").at("rover");
                romi::NavigationSettings rover_config(rover_settings);
                std::string driver_device = config["ports"]["brush-motor-driver"]["port"];
                nlohmann::json driver_settings = config.at("navigation").at("brush-motor-driver");
                client_name = "brush_motor_driver_device";
                auto driver_serial = romiserial::RomiSerialClient::create(driver_device,
                                                                          client_name,
                                                                          serial_log);
                romi::BrushMotorDriver motor_driver(driver_serial,
                                                    driver_settings,
                                                    rover_config.compute_max_angular_speed(),
                                                    rover_config.compute_max_angular_acceleration());
                
                romi::WheelOdometry wheelodometry(rover_config, motor_driver);
                romi::LocationTracker distance_measure(wheelodometry, wheelodometry);

                // Track follower

                std::string track_follower_name = config["navigation"]["track-follower"];
                std::unique_ptr<romi::ITrackFollower> track_follower;

                if (track_follower_name == "odometry") {
                        track_follower = std::make_unique<romi::LocationTracker>(wheelodometry,
                                                                                 wheelodometry);
                } else if (track_follower_name == "manual") {
                        track_follower = std::make_unique<romi::ManualTrackFollower>(input_device,
                                                                        5.0 * M_PI / 180.0);

                } else if (track_follower_name == "python") {
                        auto python_client = rcom::RcomClient::create("python",
                                                                      10.0, rcomlog);
                        double pixels_per_meter = compute_pixels_per_meter(config);
                        track_follower = std::make_unique<romi::PythonTrackFollower>(*camera,
                                                                                     python_client,
                                                                                     "nav",
                                                                                     pixels_per_meter,
                                                                                     session);
                } else if (track_follower_name == "imu") {
                        std::string imu_device = config["ports"]["imu"]["port"];
                        client_name = "imu_device";
                        auto imu_serial = romiserial::RomiSerialClient::create(imu_device,
                                                                               client_name,
                                                                               serial_log);
                        track_follower = std::make_unique<romi::IMUTrackFollower>(imu_serial);
                } else {
                        r_err("Unknown track follower: %s", track_follower_name.c_str());
                        throw std::runtime_error("Unknown track follower");
                }
                
                
                // Navigation controller
                
                //romi::ZeroNavigationController navigation_controller;
                romi::L1NavigationController navigation_controller(2.0);


                // Steering
                
                //romi::DifferentialSteering steering(motor_driver, rover_config);

                romi::DifferentialSteering differential_steering(motor_driver, rover_config);
                
                std::string steering_device = config["ports"]["steering"]["port"];
                client_name = "steering_device";
                auto steering_serial
                        = romiserial::RomiSerialClient::create(steering_device,
                                                               client_name,
                                                               serial_log);
                romi::SteeringController steering_controller(steering_serial);

                // REFACTOR. ADD THESE TO A STEERING CONSTANTS FILE.
                double max_rpm = 500; // From the motor specs
                double max_rps = max_rpm / 60.0;
                double default_rps = max_rps / 2.0; // Turn at 1/2th of max speed
                double steps_per_revolution = 200; // From the motor specs
                double microsteps = 16;  // Driver jumper settings
                double gears = 76.0 + 49.0/64.0; // From the motor specs
                double belt = 34.0 / 34.0;
                int16_t steps_per_second = (int16_t) ceil(default_rps
                                                          * steps_per_revolution
                                                          * microsteps);
                double total_steps_per_revolution = (steps_per_revolution
                                                     * microsteps * gears * belt);
                
                romi::StepperSteering wheel_steering(steering_controller, rover_config,
                                                     steps_per_second,
                                                     total_steps_per_revolution);

                romi::DoubleSteering steering(differential_steering, wheel_steering);
                        
                // Navigation
                r_info("main: Creating navigation");
                
                romi::Navigation navigation(rover_config, distance_measure,
                                            *track_follower, navigation_controller,
                                            steering, session);
                
                // SpeedController
                r_info("main: Creating speed controller");
                romi::SpeedController speed_controller(navigation, config);

                // EventTimer
                r_info("main: Creating event timer");
                romi::EventTimer event_timer(romi::event_timer_timeout);

                // Scripts and script engine
                std::string script_file = get_script_file(options, config);
                r_info("main: Loading scripts from %s", script_file.c_str());
                romi::ScriptList scripts(script_file);
                
                r_info("main: Creating scripts engine");
                romi::ScriptMenu menu(scripts);
                romi::RoverScriptEngine script_engine(scripts, romi::event_script_finished,
                                                      romi::event_script_error);

                // Notifications
                // std::string soundfont = get_sound_font_file(options, config);
                // r_info("main: Loading soundfont %s", soundfont.c_str());
                // nlohmann::json sound_setup = config.at("user-interface").at(["fluid-sounds").at("sounds");
                // r_info("main: Creating sound notifications");
                // romi::FluidSoundNotifications notifications(soundfont, sound_setup);
                romi::DummyNotifications notifications;
                
                // Imager

                std::string imager_name = config["imager"];
                std::unique_ptr<romi::IImager> imager;

                if (imager_name == "unet") {
                        imager = std::make_unique<romi::UnetImager>(session, *camera);
                } else {
                        imager = std::make_unique<romi::Imager>(session, *camera);
                }

                romi::RemoteStateInputDevice remoteStateInputDevice;


                // Rover
                r_info("main: Creating rover");
                romi::Rover rover(input_device,
                                  display,
                                  speed_controller,
                                  navigation,
                                  event_timer,
                                  menu,
                                  script_engine,
                                  notifications,
                                  weeder,
                                  *imager,
                                  remoteStateInputDevice);

                auto scriptHubListener = std::make_shared<romi::ScriptHubListener>(rover);
                auto scriptHub = rcom::MessageHub::create("script",
                                                          scriptHubListener,
                                                          rcomlog,
                                                          romi::ScriptHubListeningPort,
                                                          true);

                
                // State machine
                r_info("main: Creating state machine");
                romi::RoverStateMachine state_machine(rover);

                // User interface
                r_info("main: Creating user interface");
                romi::RoverInterface user_interface(rover, state_machine);

                r_info("main: Sending start event");
                
                if (!state_machine.handle_event(romi::event_start))
                        // FIXME: should not quit but display something
                        throw std::runtime_error("start-up failed");
                
                r_info("main: Starting event loop");

                // FIXME
                std::string kWheelOdometryOrientation = "wheel-odometry-orientation";
                std::string kPCAOrientation = "pca-orientation";
                std::string kIMUOrientation = "imu-orientation";

                motor_driver.start_recording_speeds();

                //imu_track_follower.start_line();
                
                while (!quit) {
                        
                        try {
                                user_interface.handle_events();

                                scriptHub->handle_events();

                                // FIXME
                                //double now = clock->time();
                                //romi::log_data(now, kWheelOdometryOrientation, wheelodometry.get_orientation());
                                // romi::log_data(now, kPCAOrientation, track_follower.get_orientation_error());
                                // romi::log_data(now, kIMUOrientation,
                                //                imu_track_follower.get_orientation_error());
                                
                                clock->sleep(0.020);
                                
                        } catch (std::exception& e) {
                                
                                navigation.stop();
                                throw;
                        }
                }

                retval = 0;

                
        } catch (nlohmann::json::exception& je) {
                r_err("main: Invalid configuration file: %s", je.what());
                
        } catch (std::exception& e) {
                r_err("main: exception: %s", e.what());
        }
        
        return retval;
}
