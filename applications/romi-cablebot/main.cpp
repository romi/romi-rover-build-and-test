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
#include <r.h>
#include <picamera/PiCamera.h>
#include <picamera/PiCameraSettings.h>
#include <hal/BldcGimbal.h>
#include <cablebot/Cablebot.h>
#include <cablebot/CablebotBase.h>
#include <configuration/GetOpt.h>
#include <ClockAccessor.h>
#include <RomiSerialClient.h>

static bool quit = false;
static void set_quit(int sig, siginfo_t *info, void *ucontext);
static void quit_on_control_c();

static const char *kBase = "base";
static const char *kCameraVersion = "camera-version";
static const char *kMode = "mode";
static const char *kVideo = "video";
static const char *kStill = "still";
static const char *kWidth = "width";
static const char *kHeight = "height";
static const char *kFPS = "fps";
static const char *kBitrate = "bitrate";
static const char *kGimbal = "gimbal";
static const char *kTopic = "topic";

static std::vector<romi::Option> option_list = {
        { "help", false, nullptr,
          "Print help message" },
                
        { kBase, true, nullptr,
          "The path of the cablebot base device."},

        { kCameraVersion, true, "hq",
          "The camera version: 'v2' or 'hq' (hq)"},
                
        { kMode, true, kVideo,
          "The camera mode: 'video' or 'still'."},

        { kWidth, true, "4056",
          "The image width (4056)"},

        { kHeight, true, "3040",
          "The image height (3040)"},

        { kFPS, true, "5",
          "The frame rate, for video mode only (5 fps)"},

        { kBitrate, true, "25000000",
          "The average bitrate (video only) (25000000)"},

        { kGimbal, true, nullptr,
          "Connect to the gimbal with the given device path"}
};

static const constexpr double kScanLength = 18.0;
static const constexpr double kScanInterval = 0.25;
static const constexpr double kStartPosition = 1.0;
static const constexpr double kEndPosition = kStartPosition + kScanLength;


void scan(romi::IImagingDevice& cablebot)
{
        cablebot.wake_up();
        cablebot.homing();

        double position = kStartPosition;
        while (position < kEndPosition) {
                cablebot.moveto(position, 0.0, 0.0, 0.5);
                rpp::MemBuffer& image = cablebot.grab_jpeg();
                (void) image;
                position += kScanInterval;
        }
        
        cablebot.moveto(0.1, 0.0, 0.0, 0.5);
        cablebot.homing();
        cablebot.stand_by();
}

int main(int argc, char **argv)
{
        std::shared_ptr<rpp::IClock> clock = std::make_shared<rpp::Clock>();
        rpp::ClockAccessor::SetInstance(clock);
        
        try {
                romi::GetOpt options(option_list);
                options.parse(argc, argv);
                if (options.is_help_requested()) {
                        options.print_usage();
                        exit(0);
                }

                std::string mode = options.get_value(kMode);
                std::string width_value = options.get_value(kWidth);
                std::string height_value = options.get_value(kHeight);
                std::string fps_value = options.get_value(kFPS);
		std::string version = options.get_value(kCameraVersion);
		std::string bitrate_value = options.get_value(kBitrate);

                size_t width = (size_t) std::stoul(width_value);
                size_t height = (size_t) std::stoul(height_value);
                int32_t fps = (int32_t) std::stoi(fps_value);
                uint32_t bitrate = (uint32_t) std::stoi(bitrate_value);
		
                r_info("Camera: %zux%zu.", width, height);
                
                std::unique_ptr<romi::PiCameraSettings> settings;

                if (version == "v2") {
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
		  
		} else if (version == "hq") {
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
		}

                // Gimbal
                std::unique_ptr<romiserial::IRomiSerialClient> gimbal_serial;
                std::unique_ptr<romi::IGimbal> gimbal;
                
                if (options.is_set(kGimbal)) {
                        std::string device = options.get_value(kGimbal);
                        r_info("Connecting to gimbal at %s", device.c_str());
                        gimbal_serial = romiserial::RomiSerialClient::create(device,
                                                                             "BldcGimbal");
                        gimbal = std::make_unique<romi::BldcGimbal>(*gimbal_serial);
                }
                
                // Camera
                std::unique_ptr<romi::ICamera> camera = romi::PiCamera::create(*settings);

                // Base
                std::unique_ptr<romiserial::IRomiSerialClient> base_serial;
                std::unique_ptr<romi::ICNC> base;
                std::string device = options.get_value(kBase);
                r_info("Connecting to base at %s", device.c_str());
                base_serial = romiserial::RomiSerialClient::create(device,
                                                                   "CablebotBase");
                base = std::make_unique<romi::CablebotBase>(base_serial);

                // Cablebot
                romi::Cablebot cablebot(base, gimbal, camera);

                
                scan(cablebot);
                
                // quit_on_control_c();
                
                // while (!quit) {
                // }

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
