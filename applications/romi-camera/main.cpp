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

#define HAS_LEGACY_PICAMERA 0


#include <rcom/RegistryServer.h>
#include <rcom/RcomServer.h>

#include <RomiSerialClient.h>

#include <rpc/CameraAdaptor.h>
#include <hal/BldcGimbal.h>
#include <configuration/GetOpt.h>
#include <util/ClockAccessor.h>
#include <util/Logger.h>

#if HAS_LEGACY_PICAMERA
#include <picamera/PiCamera.h>
#include <picamera/PiCameraSettings.h>
#endif

#include <camera/ExternalCamera.h>

static bool quit = false;
static void set_quit(int sig, siginfo_t *info, void *ucontext);
static void quit_on_control_c();

static const char *kCameraType = "camera-type";  // picamera-hq, picamera-v2, file-camera, external-camera
static const char *kWidth = "width";
static const char *kHeight = "height";
static const char *kRegistry = "registry";
static const char *kTopic = "topic";

#if HAS_LEGACY_PICAMERA
static const char *kMode = "mode";
static const char *kVideo = "video";
static const char *kStill = "still";
static const char *kFPS = "fps";
static const char *kBitrate = "bitrate";
#endif

static std::vector<romi::Option> option_list = {
        { "help", false, nullptr,
          "Print help message" },
                
        { kRegistry, true, nullptr,
          "The IP address of the registry."},
                
        { kTopic, true, "camera",
          "The topic used for the registration."},

        { kCameraType, true, "external-camera",
          "The camera type: picamera-hq, picamera-v2, file-camera, external-camera (external-camera)"},

        { kWidth, true, "1640",
          "The image width (1640)"},

        { kHeight, true, "1232",
          "The image height (1232)"},

#if HAS_LEGACY_PICAMERA
        { kMode, true, kVideo,
          "The camera mode: 'video' or 'still'."},
        
        { kFPS, true, "5",
          "The frame rate, for video mode only (5 fps)"},

        { kBitrate, true, "17000000",
          "The average bitrate (video only) (17000000)"}
#endif
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
                
                if (options.is_set(kRegistry)) {
                        std::string ip = options.get_value(kRegistry);
                        r_info("Registry IP set to %s", ip.c_str());
                        rcom::RegistryServer::set_address(ip.c_str());
                }

		std::string type = options.get_value(kCameraType);
                std::string width_value = options.get_value(kWidth);
                std::string height_value = options.get_value(kHeight);

                size_t width = (size_t) std::stoul(width_value);
                size_t height = (size_t) std::stoul(height_value);
		
                r_info("Camera: %s(%zux%zu).", type.c_str(), width, height);
                
                std::unique_ptr<romi::ICamera> camera;
                        
#if HAS_LEGACY_PICAMERA
                std::string mode = options.get_value(kMode);
                std::string fps_value = options.get_value(kFPS);
		std::string bitrate_value = options.get_value(kBitrate);
                int32_t fps = (int32_t) std::stoi(fps_value);
                uint32_t bitrate = (uint32_t) std::stoi(bitrate_value);
                std::unique_ptr<romi::PiCameraSettings> settings;
                
                if (type == "picamera-v2") {
                        if (mode == kVideo) {
                                r_info("Camera: video mode, %d fps, %d bps", (int) fps, (int) bitrate);
                                settings = std::make_unique<romi::V2VideoCameraSettings>(width,
                                                                                         height,
                                                                                         fps);
                                settings->bitrate_ = bitrate;

                        } else if (mode == kStill) {
                                r_info("Camera: still mode.");
                                settings = std::make_unique<romi::V2StillCameraSettings>(width, height);
                        }

                        camera = romi::PiCamera::create(*settings);

		} else if (type == "picamera-hq") {
                        if (mode == kVideo) {
                                r_info("Camera: video mode, %d fps, %d bps", (int) fps, (int) bitrate);
                                settings = std::make_unique<romi::HQVideoCameraSettings>(width,
                                                                                         height,
                                                                                         fps);
                                settings->bitrate_ = bitrate;

                        } else if (mode == kStill) {
                                r_info("Camera: still mode.");
                                settings = std::make_unique<romi::HQStillCameraSettings>(width, height);
                        }
                        
                        camera = romi::PiCamera::create(*settings);
                        
		}
#endif
                if (type == "external-camera") {
                        camera = std::make_unique<romi::ExternalCamera>("/home/hanappe/projects/ROMI/github/rover/romi-rover-build-and-test/applications/romi-cablebot/vgrabbj/grab.sh");
		}
                
                std::string topic = options.get_value(kTopic);
                romi::CameraAdaptor adaptor(*camera);
                auto camera_server = rcom::RcomServer::create(topic, adaptor);
                
                quit_on_control_c();
                
                while (!quit) {
                        camera_server->handle_events();
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
