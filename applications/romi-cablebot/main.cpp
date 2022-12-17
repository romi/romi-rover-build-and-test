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
#include <thread>
#include <fstream>
#include <filesystem>

#include <rcom/RegistryServer.h>
#include <rcom/RcomServer.h>
#include <rcom/Linux.h>

#include <RomiSerialClient.h>

#include <session/Session.h>
#include <rpc/CameraAdaptor.h>
#include <rpc/CameraMountAdaptor.h>
#include <rpc/RemoteObjectsAdaptor.h>
#include <configuration/GetOpt.h>
#include <util/ClockAccessor.h>
#include <camera/ICameraSettings.h>
#include <camera/CameraInfoIO.h>
#include <cablebot/Cablebot.h>
#include <cablebot/CablebotProgram.h>
#include <cablebot/CablebotProgramIO.h>
#include <cablebot/ICablebotProgramList.h>
#include <cablebot/CablebotController.h>
#include <cablebot/CablebotRunner.h>
#include <util/Logger.h>
#include <util/AlarmClock.h>
#include <util/IAlarmClockListener.h>
#include <api/ConfigManager.h>
#include <data_provider/RomiDeviceData.h>
#include <data_provider/SoftwareVersion.h>
#include <data_provider/CameraMountLocationProvider.h>
#include <cablebot/CablebotProgramListAdaptor.h>

static bool quit = false;
static void set_quit(int sig, siginfo_t *info, void *ucontext);
static void quit_on_control_c();

static const char *kRegistry = "registry";
static const char *kConfig = "config";
static const char *kDirectory = "directory";

static std::vector<romi::Option> option_list = {
        { "help", false, nullptr,
          "Print help message" },
                
        { kRegistry, true, nullptr,
          "The IP address of the registry."},
                
        { kConfig, true, "config.json",
          "The config file (config.json)"},
                
        { kDirectory, true, ".",
          "The directory to store the images and log files (current directory)"}                
};


int main(int argc, char **argv)
{
        std::shared_ptr<romi::IClock> clock = std::make_shared<romi::Clock>();
        romi::ClockAccessor::SetInstance(clock);
        
        try {
                // Linux
                rcom::Linux linux;

                // Options
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

                // Config
                std::string config_value = options.get_value(kConfig);

                r_info("Romi Cablebot: Using configuration file: '%s'",
                       config_value.c_str());
                
                std::filesystem::path config_path = config_value;
                
                std::shared_ptr<romi::IConfigManager> config
                        = std::make_shared<romi::ConfigManager>(config_path);
                
                // Camera settings
                std::shared_ptr<romi::ICameraInfoIO> info_io =
                        std::make_shared<romi::CameraInfoIO>(config, "camera");

                
                std::unique_ptr<romi::ICameraInfo> camera_info = info_io->load();
                
                // Cablebot
                r_debug("romi-cablebot: Initializing cablebot");
                auto cablebot = romi::Cablebot::create(info_io);

                // Session
                romi::RomiDeviceData romiDeviceData;
                romi::SoftwareVersion softwareVersion;
                std::unique_ptr<romi::ILocationProvider> locationProvider
                        = std::make_unique<romi::CameraMountLocationProvider>(cablebot->mount_);
                std::string directory = options.get_value(kDirectory);
                romi::Session session(linux, directory, romiDeviceData,
                                      softwareVersion, std::move(locationProvider));
                
                // Programs
                std::shared_ptr<romi::ICablebotProgramIO> program_io =
                        std::make_shared<romi::CablebotProgramIO>(config, "programs");
                
                std::unique_ptr<romi::ICablebotProgramList> programs = program_io->load();
                std::shared_ptr<romi::ICablebotProgramList> shared_programs
                        = std::move(programs);

                // Runner
                std::shared_ptr<romi::IAlarmClockListener> runner
                        = std::make_shared<romi::CablebotRunner>(shared_programs, *cablebot, session);

                // Alarm
                std::shared_ptr<romi::IAlarmClock> alarmclock
                        = std::make_shared<romi::AlarmClock>(runner);
        
                // Controller                
                romi::CablebotController controller(shared_programs, alarmclock);
                
                // Server                
                std::shared_ptr<rcom::IRPCHandler> remote_camera
                        = std::make_shared<romi::CameraAdaptor>(*cablebot->camera_);
                
                
                std::shared_ptr<rcom::IRPCHandler> remote_camera_mount
                        = std::make_shared<romi::CameraMountAdaptor>(*cablebot->mount_);
                
                std::shared_ptr<rcom::IRPCHandler> remote_programs_adaptor
                        = std::make_shared<romi::CablebotProgramListAdaptor>(shared_programs, program_io);

                romi::RemoteObjectsAdaptor adaptor;
                adaptor.add("camera", remote_camera);
                adaptor.add("camera-mount", remote_camera_mount);
                adaptor.add("programs", remote_programs_adaptor);
                
                auto server = rcom::RcomServer::create("cablebot", adaptor);
                
                quit_on_control_c();
                
                while (!quit) {
                        server->handle_events();
                        romi::ClockAccessor::GetInstance()->sleep(0.002);
                }

        } catch (std::exception& e) {
                r_err("romi-cablebot: caught exception: %s", e.what());
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
