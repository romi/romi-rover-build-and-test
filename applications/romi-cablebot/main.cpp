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
#include <cablebot/Cablebot.h>
#include <configuration/GetOpt.h>
#include <ClockAccessor.h>
#include <RomiSerialClient.h>

static bool quit = false;
static void set_quit(int sig, siginfo_t *info, void *ucontext);
static void quit_on_control_c();

static const char *kMode = "mode";
static const char *kVideo = "video";
static const char *kStill = "still";
static const char *kWidth = "width";
static const char *kHeight = "height";
static const char *kFPS = "fps";
static const char *kBitrate = "bitrate";

static std::vector<romi::Option> option_list = {
        { "help", false, nullptr,
          "Print help message" },
                
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
};

static const constexpr double kScanLength = 18.0;
static const constexpr double kScanInterval = 0.25;
static const constexpr double kStartPosition = 1.0;
static const constexpr double kEndPosition = kStartPosition + kScanLength;


void scan(romi::IImagingDevice& cablebot)
{
        cablebot.power_up();

        double position = kStartPosition;
        while (position < kEndPosition) {
                cablebot.cnc_->moveto(position, 0.0, 0.0, 0.5);
                rpp::MemBuffer& image = cablebot.camera_->grab_jpeg();
                (void) image;
                position += kScanInterval;
        }
        
        cablebot.cnc_->moveto(0.1, 0.0, 0.0, 0.5);
        cablebot.cnc_->homing();
        cablebot.power_down();
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

                std::string mode_string = options.get_value(kMode);
                std::string width_value = options.get_value(kWidth);
                std::string height_value = options.get_value(kHeight);
                std::string fps_value = options.get_value(kFPS);
		std::string version = options.get_value(kCameraVersion);
		std::string bitrate_value = options.get_value(kBitrate);

                size_t width = (size_t) std::stoul(width_value);
                size_t height = (size_t) std::stoul(height_value);
                int32_t fps = (int32_t) std::stoi(fps_value);
                uint32_t bitrate = (uint32_t) std::stoi(bitrate_value);
                romi::Cablebot::CameraMode mode;

                if (mode_string == kVideo) {
                        mode = romi::Cablebot::kVideoMode;
                } else if (mode_string == kStill) {
                        mode = romi::Cablebot::kStillMode;
                } else {
                        throw std::runtime_error("Invalid camera mode: %s",
                                                 mode_string.c_str());
                }
		
                r_info("Camera: %zux%zu.", width, height);
		  
                auto cablebot = romi::Cablebot::create(mode, width, height, fps, bitrate);
                
                scan(*cablebot);
                
                // quit_on_control_c();
                
                // while (!quit) {
                // }

        } catch (std::exception& e) {
                r_err("RomiCamera: caught exception: %s", e.what());
        }
}

// static void set_quit(int sig, siginfo_t *info, void *ucontext)
// {
//         (void) sig;
//         (void) info;
//         (void) ucontext;
//         quit = true;
// }

// static void quit_on_control_c()
// {
//         struct sigaction act;
//         memset(&act, 0, sizeof(struct sigaction));

//         act.sa_flags = SA_SIGINFO;
//         act.sa_sigaction = set_quit;
//         if (sigaction(SIGINT, &act, nullptr) != 0) {
//                 perror("init_signal_handler");
//                 exit(1);
//         }
// }
