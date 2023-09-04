/*
  romi-config

  Copyright (C) 2019-2020 Sony Computer Science Laboratories
  Author(s) Peter Hanappe

  romi-config provide a single config file to distributed Romi apps.

  romi-config is free software: you can redistribute it and/or modify
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

#include <rcom/RcomServer.h>
#include <rcom/RemoteObjectsAdaptor.h>

#include <api/LocalConfig.h>
#include <configuration/GetOpt.h>
#include <util/ClockAccessor.h>
#include <util/Logger.h>
#include <rpc/ConfigAdaptor.h>

static bool quit = false;
static void set_quit(int sig, siginfo_t *info, void *ucontext);
static void quit_on_control_c();

static const char *kConfig = "config";

static std::vector<romi::Option> option_list = {
        { "help", false, nullptr,
          "Print help message" },
                
        { kConfig, true, "config.json",
          "The config file (config.json)"},
};

int main(int argc, char **argv)
{
        std::shared_ptr<romi::IClock> clock = std::make_shared<romi::Clock>();
        romi::ClockAccessor::SetInstance(clock);
        
        try {
                // Options
                romi::GetOpt options(option_list);
                options.parse(argc, argv);
                if (options.is_help_requested()) {
                        options.print_usage();
                        exit(0);
                }

                // Config
                std::string config_value = options.get_value(kConfig);

                r_info("Romi Cablebot: Using configuration file: '%s'",
                        config_value.c_str());
                
                std::filesystem::path config_path = config_value;
                romi::LocalConfig config(config_path);
                
                // Server                
                romi::ConfigAdaptor config_adaptor(config);
                auto server = rcom::RcomServer::create("config", config_adaptor);

                
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
