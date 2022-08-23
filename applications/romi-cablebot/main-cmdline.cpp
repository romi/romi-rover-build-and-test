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
#include <Linux.h>
#include <data_provider/RomiDeviceData.h>
#include <data_provider/SoftwareVersion.h>
#include <session/Session.h>
#include <data_provider/CNCLocationProvider.h>
#include <picamera/PiCameraSettings.h>

static const char *kMode = "mode";
static const char *kVideo = "video";
static const char *kStill = "still";
static const char *kWidth = "width";
static const char *kHeight = "height";
static const char *kFPS = "fps";
static const char *kBitrate = "bitrate";
static const char *kStart = "start";
static const char *kLength = "length";
static const char *kInterval = "interval";
static const char *kDirectory = "dir";

static std::vector<romi::Option> option_list = {
        { "help", false, nullptr,
          "Print help message" },
                
        { kMode, true, kStill,
          "The camera mode: 'video' or 'still'. (still)"},

        { kWidth, true, "4056",
          "The image width (4056)"},

        { kHeight, true, "3040",
          "The image height (3040)"},

        { kFPS, true, "5",
          "The frame rate, for video mode only (5 fps)"},

        { kBitrate, true, "25000000",
          "The average bitrate (video only) (25000000)"},

        { kStart, true, "0",
          "The start position of the scan (in meter)"},

        { kLength, true, "1",
          "The length of the scan (in meter)"},

        { kInterval, true, "0.25",
          "The interval between images (in meter)"},
        
        { kDirectory, true, ".",
          "The session directory (.)"},
};

struct ScanOptions
{
        romi::Cablebot::CameraMode mode;
        size_t width;
        size_t height;
        int32_t fps;
        uint32_t bitrate;
        double start;
        double length;
        double interval;
        std::string directory;

        ScanOptions()
                : mode(romi::Cablebot::kVideoMode),
                  width(romi::kHQFullWidth),
                  height(romi::kHQFullHeight),
                  fps(5),
                  bitrate(romi::kMaxBitrateMJPEG),
                  start(0.0),
                  length(1.0),
                  interval(0.25),
                  directory(".")
                {}
        
};

std::string make_image_name(size_t counter)
{
        char buffer[64];
	snprintf(buffer, sizeof(buffer), "camera-%06zu.jpg", counter);
	return std::string(buffer);
}

bool store_image(rcom::MemBuffer& image, romi::Session& session, size_t counter)
{
        bool success = true;
        if (image.size() > 0) {
                std::string filename = make_image_name(counter);
                success = session.store_jpg(filename, image);
                if (!success) {
                        r_err("romi-cablebot: Failed to store the image");
                } else {
                        r_debug("romi-cablebot: Stored %s", filename.c_str());
                }
        }
        return success;
}

bool grab_image(romi::ImagingDevice& cablebot, romi::Session& session, size_t counter)
{
        bool success = true;
        try {
                rcom::MemBuffer& image = cablebot.camera_->grab_jpeg();
                success = store_image(image, session, counter);
        } catch (std::runtime_error& e) {
                r_err("grab_image failed: %s", e.what());
                success = false;
        }
        return success;
}

bool init_scan(romi::ImagingDevice& cablebot, romi::Session& session)
{
        r_debug("romi-cablebot: Scan starting");
	session.start("test");
        r_debug("romi-cablebot: Power up");
        return cablebot.power_up();
}

bool moveto(romi::ImagingDevice& cablebot, double position)
{
        r_debug("romi-cablebot: Move to %.2f", position);
        bool success = cablebot.cnc_->moveto(position, 0.0, 0.0, 1.0);
        if (!success) {
                r_err("moveto: failed");
        }
        return success;
}

bool run_scan(romi::ImagingDevice& cablebot, romi::Session& session,
              double start, double length, double interval)
{
	size_t counter = 0;
        double position = start;
        double end = start + length;

        r_debug("romi-cablebot: Scan run");
        
        bool success = moveto(cablebot, position);
        while (success && position <= end + 0.005f) {
                success = (grab_image(cablebot, session, counter)
                           && moveto(cablebot, position));
                position += interval;
                counter++;
        }
        cablebot.cnc_->moveto(0.1, 0.0, 0.0, 1.0);
        return success;
}

void end_scan(romi::ImagingDevice& cablebot, romi::Session& session)
{
        cablebot.cnc_->homing();
        cablebot.power_down();
	session.stop();
        r_debug("romi-cablebot: Scan done");
}

bool scan(romi::ImagingDevice& cablebot, romi::Session& session,
          double start, double length, double interval)
{
        bool success = (init_scan(cablebot, session)
                        && run_scan(cablebot, session, start, length, interval));
        end_scan(cablebot, session);
        return success;
}

void check_help(romi::GetOpt& options)
{
        if (options.is_help_requested()) {
                options.print_usage();
                exit(0);
        }
}

romi::Cablebot::CameraMode parse_mode(romi::GetOpt& options)
{
        romi::Cablebot::CameraMode mode;
        std::string mode_string = options.get_value(kMode);
        if (mode_string == kVideo) {
                mode = romi::Cablebot::kVideoMode;
        } else if (mode_string == kStill) {
                mode = romi::Cablebot::kStillMode;
        } else {
                throw std::runtime_error("Invalid camera mode");
        }
        return mode;
}

double parse_value(romi::GetOpt& options, const char *name)
{
        std::string string_value = options.get_value(name);
        return std::stod(string_value);
}

void parse_options(int argc, char **argv, ScanOptions& scan_options)
{
        romi::GetOpt cmdline_options(option_list);
        cmdline_options.parse(argc, argv);
        
        check_help(cmdline_options);
        
        scan_options.mode = parse_mode(cmdline_options);
        scan_options.width = (size_t) parse_value(cmdline_options, kWidth);
        scan_options.height = (size_t) parse_value(cmdline_options, kHeight);
        scan_options.fps = (int32_t) parse_value(cmdline_options, kFPS);
        scan_options.bitrate = (uint32_t) parse_value(cmdline_options, kBitrate);
        scan_options.start = parse_value(cmdline_options, kStart);
        scan_options.length = parse_value(cmdline_options, kLength);
        scan_options.interval = parse_value(cmdline_options, kInterval);
        scan_options.directory = cmdline_options.get_value(kDirectory);
}

int main(int argc, char **argv)
{
        std::shared_ptr<rpp::IClock> clock = std::make_shared<rpp::Clock>();
        rpp::ClockAccessor::SetInstance(clock);
        
        try {
                // Options
                ScanOptions options;
                parse_options(argc, argv, options);
		  
                // Linux
                rpp::Linux linux;

                // Cablebot
                r_debug("romi-cablebot: Initializing cablebot");
                auto cablebot = romi::Cablebot::create(options.mode, options.width,
                                                       options.height, options.fps,
                                                       options.bitrate);

                // Session
                romi::RomiDeviceData romiDeviceData;
                romi::SoftwareVersion softwareVersion;
                std::unique_ptr<romi::ILocationProvider> locationProvider
                        = std::make_unique<romi::CameraMountLocationProvider>(cablebot->mount_);

                romi::Session session(linux, options.directory, romiDeviceData,
                                      softwareVersion, std::move(locationProvider));

                // Scan
                scan(*cablebot, session, options.start, options.length, options.interval);

        } catch (std::exception& e) {
                r_err("romi-cablebot: caught exception: %s", e.what());
        }
}
