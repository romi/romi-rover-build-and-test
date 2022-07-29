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
#include <memory>
#include <atomic>
#include <syslog.h>
#include <string.h>

#include <Linux.h>
#include <RomiSerialClient.h>
#include <Clock.h>
#include <ClockAccessor.h>
#include <configuration/ConfigurationProvider.h>
#include <camera/FileCamera.h>
#include <camera/USBCamera.h>
#include <rpc/RemoteCamera.h>
#include <rpc/RcomClient.h>
#include <oquam/StepperController.h>
#include <oquam/StepperSettings.h>
#include <oquam/Oquam.h>
#include <data_provider/RomiDeviceData.h>
#include <data_provider/SoftwareVersion.h>
#include <session/Session.h>
#include <data_provider/Gps.h>
#include <data_provider/GpsLocationProvider.h>
#include <ui/LinuxJoystick.h>

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

std::string make_filename(double x, double y, double z)
{
        char buffer[100];
        snprintf(buffer, sizeof(buffer), "calibration-%0.3f-%0.3f-%0.3f.jpg", x, y, z);
        return std::string(buffer);
}

void assert_moveto(romi::Oquam& oquam, double x, double y, double z)
{
        if (!oquam.moveto(x, y, z, 0.7))
                throw std::runtime_error("moveto failed");
}

void assert_homing(romi::Oquam& oquam)
{
        if (!oquam.homing())
                throw std::runtime_error("homing failed");
}

void move_and_grab(romi::Session& session,
                   romi::Oquam& oquam,
                   romi::ICamera& camera,
                   double x, double y, double z)
{
        assert_moveto(oquam, x, y, z);
        rcom::MemBuffer& image = camera.grab_jpeg();
        std::string filename = make_filename(x, y, z);
        session.store_jpg(filename, image);
}

void run_calibration(romi::Session& session,
                     romi::CNCRange& range,
                     romi::Oquam& oquam,
                     romi::ICamera& camera)
{
        double x0 = range.xmin();
        double y0 = range.ymin();
        double z0 = range.zmin();
                
        double x1 = range.xmax();
        double y1 = range.ymax();
        double z1 = range.zmax();

        double dx = (x1 - x0) / 10.0;
        double dy = (y1 - y0) / 10.0;

        double x = x0;
        double y = y0;

        assert_moveto(oquam, x0, y0, z0);
                
        for (y = y0; y <= y1; y += dy) {
                move_and_grab(session, oquam, camera, x0, y, z0);
        }

        for (x = x0+dx; x <= x1; x += dx) {
                move_and_grab(session, oquam, camera, x, y1, z0);
        }
                
        for (y = y1-dy; y >= y0; y -= dy) {
                move_and_grab(session, oquam, camera, x1, y, z0);
        }

        for (x = x1-dx; x >= x0; x -= dx) {
                move_and_grab(session, oquam, camera, x, y0, z0);
        }

        assert_moveto(oquam, x0, y0, z1);
}

static const uint8_t kXButton = 0;
static const uint8_t kLeftAxisUpDown = 1;
static const uint8_t kRightAxisUpDown = 3;

bool handle_axis_event(romi::CNCRange& range,
                       romi::Oquam& oquam,
                       romi::IJoystick& joystick,
                       romi::JoystickEvent& event)
{
        bool success = true;
        if (event.number == kLeftAxisUpDown) {
                double v = joystick.get_axis(event.number);
                if (v == 0.0) {
                        success = oquam.moveat(0, 0, 0);
                        romi::v3 pos;
                        success = oquam.get_position(pos);
                        if (success) {
                                r_debug("setting z0 to %f", pos.z());
                                romi::v3 min = range.min();
                                romi::v3 max = range.max();
                                min.z(pos.z());
                                range.init(min, max);
                        } else {
                                r_err("get_position failed");
                        }

                } else if (v < 0.0) {
                        success = oquam.moveat(0, 0, -900);
                        
                } else {
                        success = oquam.moveat(0, 0, 900);
                } 
        }
        if (!success) {
                r_err("moveat failed");
                oquam.moveat(0, 0, 0);
                oquam.power_down();
                
        }
        return !success;
}

bool handle_button_event(romi::IJoystick& joystick, romi::JoystickEvent& event)
{
        if (event.number == kXButton
                && joystick.is_button_pressed(kXButton))
                r_debug("X pressed");
        
        return (event.number == kXButton
                && joystick.is_button_pressed(kXButton));
}

bool handle_joystick_event(romi::CNCRange& range,
                           romi::Oquam& oquam,
                           romi::IJoystick& joystick)
{
        bool done = false;
        romi::JoystickEvent& event = joystick.get_next_event();

        switch (event.type) {
        default:
        case romi::JoystickEvent::kNone:
                break;
        case romi::JoystickEvent::kButton:
                done = handle_button_event(joystick, event);
                break;
        case romi::JoystickEvent::kAxis:
                done = handle_axis_event(range, oquam, joystick, event);
                break;
        }
        
        return done;
}

void set_height(romi::CNCRange& range, romi::Oquam& oquam, romi::IJoystick& joystick)
{
        bool done = false;

        while (!done) {
                done = handle_joystick_event(range, oquam, joystick);
        }

        r_info("Finished setting the height");
}

int main(int argc, char** argv)
{
        std::shared_ptr<rpp::IClock> clock = std::make_shared<rpp::Clock>();
        rpp::ClockAccessor::SetInstance(clock);

        int retval = 1;

        romi::RoverOptions options;
        options.parse(argc, argv);
        options.exit_if_help_requested();

        log_init();
        log_set_application("move-and-grab");

        std::signal(SIGSEGV, SignalHandler);
        std::signal(SIGINT, SignalHandler);

        try {
                std::string config_file = options.get_config_file();
                r_info("Romi Rover: Using configuration file: '%s'", config_file.c_str());
                // TBD: USE FileUtils
                std::ifstream ifs(config_file);
                nlohmann::json config = nlohmann::json::parse(ifs);

                // Session
                r_info("main: Creating session");
                rpp::Linux linux;
                romi::RomiDeviceData romiDeviceData;
                romi::SoftwareVersion softwareVersion;
                romi::Gps gps;
                std::unique_ptr<romi::ILocationProvider> locationProvider
                        = std::make_unique<romi::GpsLocationProvider>(gps);
                std::string session_directory = romi::get_session_directory(options, config);

                romi::Session session(linux, session_directory, romiDeviceData,
                                      softwareVersion, std::move(locationProvider));
                session.start("calibration");

                // Joystick
                r_info("main: Creating joystick");
                std::string joystick_device = config["ports"]["joystick"]["port"];
                romi::LinuxJoystick joystick(linux, joystick_device);

                
                // CNC controller
                r_info("main: Creating CNC controller");
                std::string cnc_device = config["ports"]["oquam"]["port"];
                std::string client_name("cnc_device");
                auto cnc_serial = romiserial::RomiSerialClient::create(cnc_device, client_name);
                romi::StepperController cnc_controller(cnc_serial);

                
                // CNC
                r_info("main: Creating CNC");
                nlohmann::json r = config.at("oquam").at("cnc-range");
                romi::CNCRange range(r);
                nlohmann::json s = config.at("oquam").at("stepper-settings");
                romi::StepperSettings stepper_settings(s);
                double slice_duration = (double) config["oquam"]["path-slice-duration"];
                double maximum_deviation = (double) config["oquam"]["path-maximum-deviation"];

                romi::AxisIndex homing[3] = { romi::kAxisZ, romi::kAxisX, romi::kAxisY };
                //romi::AxisIndex homing[3] = { romi::kNoAxis, romi::kNoAxis, romi::kNoAxis };
                romi::OquamSettings oquam_settings(range,
                                                   stepper_settings.maximum_speed,
                                                   stepper_settings.maximum_acceleration,
                                                   stepper_settings.steps_per_meter,
                                                   maximum_deviation,
                                                   slice_duration,
                                                   1.0,
                                                   homing);
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
                        camera = std::make_unique<romi::USBCamera>(camera_device,
                                                                   width, height);
                } else if (camera_classname == romi::RemoteCamera::ClassName) {
                        auto client = romi::RcomClient::create("camera", 10.0);
                        camera = std::make_unique<romi::RemoteCamera>(client);
                } else {
                        throw std::runtime_error("Unknown camera classname");
                }

                
                oquam.power_up();
                //assert_homing(oquam);

                r_info("Set the height of the z-axis.");
                set_height(range, oquam, joystick);
                
                r_info("Running the calibration.");
                run_calibration(session, range, oquam, *camera);

                r_info("Finished.");

                retval = 0;

                
        } catch (nlohmann::json::exception& je) {
                r_err("main: Failed to read the configuration file: %s", je.what());
                
        } catch (std::exception& e) {
                r_err("main: exception: %s", e.what());
        }
        
        return retval;
}
