/*
  romi-camera

  Copyright (C) 2019-2020 Sony Computer Science Laboratories
  Author(s) Peter Hanappe

  romi-camera is a camera app for the Romi platform.

  romi-camera is free software: you can redistribute it and/or modify
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
#include <stdexcept>
#include <memory>
#include <fstream>
#include <r.h>

#include <rcom/RegistryServer.h>
#include <rcom/RcomServer.h>

#include <RomiSerialClient.h>

#include <picamera/PiCamera.h>
#include <picamera/PiCameraSettings.h>
#include <rpc/CameraAdaptor.h>
#include <rpc/CameraMountAdaptor.h>
#include <hal/BldcGimbal.h>
#include <configuration/GetOpt.h>
#include <util/ClockAccessor.h>
#include <camera/ICameraSettings.h>
#include <camera/CameraInfoIO.h>
#include <cablebot/Cablebot.h>

static bool quit = false;
static void set_quit(int sig, siginfo_t *info, void *ucontext);
static void quit_on_control_c();

static const char *kRegistry = "registry";
static const char *kConfig = "config";
static const char *kMode = "mode";
static const char *kVideo = "video";
static const char *kStill = "still";
static const char *kWidth = "width";
static const char *kHeight = "height";
static const char *kFramerate = "framerate";
static const char *kBitrate = "bitrate";

static std::vector<romi::Option> option_list = {
        { "help", false, nullptr,
          "Print help message" },
                
        { kRegistry, true, nullptr,
          "The IP address of the registry."},
                
        { kConfig, true, "config.json",
          "The config file (config.json)"}                
};

int main(int argc, char **argv)
{
        std::shared_ptr<romi::IClock> clock = std::make_shared<romi::Clock>();
        romi::ClockAccessor::SetInstance(clock);
        
        try {
                romi::GetOpt options(option_list);
                options.parse(argc, argv);
                if (options.is_help_requested()) {
                        options.print_usage();
                        exit(0);
                }
		  
                // Linux
                //rcom::Linux linux;
                
                if (options.is_set(kRegistry)) {
                        std::string ip = options.get_value(kRegistry);
                        r_info("Registry IP set to %s", ip.c_str());
                        rcom::RegistryServer::set_address(ip.c_str());
                }

                
                std::string config_file = options.get_value(kConfig);
                r_info("Romi Cablebot: Using configuration file: '%s'", config_file.c_str());

                // TBD: Create standard JSON load functionality.
                std::ifstream ifs(config_file);
                nlohmann::json config = nlohmann::json::parse(ifs);

                romi::CameraInfoIO info_io;
                std::unique_ptr<romi::ICameraInfo> camera_info = info_io.load(config["camera"]);

                romi::ICameraSettings& camera_settings = camera_info->get_settings();
                
                romi::Cablebot::CameraMode mode;
                std::string mode_str;
                camera_settings.get_option(kMode, mode_str);
                if (mode_str == kStill)
                        mode = romi::Cablebot::kStillMode;
                else if (mode_str == kVideo)
                        mode = romi::Cablebot::kVideoMode;
                
                size_t width = (size_t) camera_settings.get_value(kWidth);
                size_t height = (size_t) camera_settings.get_value(kHeight);
                // FIXME
                int32_t fps = (int32_t) camera_settings.get_value(kFramerate);
                uint32_t bitrate = (uint32_t) camera_settings.get_value(kBitrate);
                
                r_info("Camera: %zux%zu.", width, height);

                
                // Cablebot
                r_debug("romi-cablebot: Initializing cablebot");
                auto cablebot = romi::Cablebot::create(mode, width, height, fps, bitrate);

                
                // std::unique_ptr<romi::PiCameraSettings> settings;

                // if (mode == "video") {

                //         r_info("Camera: video mode, %d fps, %d bps", (int) fps, (int) bitrate);
                //         settings = std::make_unique<romi::HQVideoCameraSettings>(width,
                //                                                                  height,
                //                                                                  fps);
                //         settings->bitrate_ = bitrate;

                // } else if (mode == "still") {
                //         r_info("Camera: still mode.");
                //         settings = std::make_unique<romi::HQStillCameraSettings>(width, height);
                // }
                
                // auto camera = romi::PiCamera::create(*settings);

                
                romi::CameraAdaptor remote_camera(*cablebot->camera_);
                auto camera_server = romi::RcomServer::create("camera", remote_camera);
                
                romi::CameraMountAdaptor remote_camera_mount(*cablebot->mount_);
                auto cablebot_server = romi::RcomServer::create("cablebot", remote_camera_mount);
                
                quit_on_control_c();
                
                while (!quit) {
                        camera_server->handle_events();
                        cablebot_server->handle_events();
                        clock->sleep(0.001);
                }

        } catch (std::exception& e) {
                r_err("RomiCamera: caught exception: %s", e.what());
        }
}

static void set_quit(int sig, siginfo_t *info, void *ucontext)
{
        (void) sig;
        (void) info;
        (void) ucontext;
        quit = true;
}

static void quit_on_control_c()
{
        struct sigaction act;
        memset(&act, 0, sizeof(struct sigaction));

        act.sa_flags = SA_SIGINFO;
        act.sa_sigaction = set_quit;
        if (sigaction(SIGINT, &act, nullptr) != 0) {
                perror("init_signal_handler");
                exit(1);
        }
}
