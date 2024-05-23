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
#include <thread>

#include <rcom/Linux.h>
#include <rcom/MemBuffer.h>
#include <rcom/RegistryServer.h>
#include <rcom/MessageLink.h>
#include <rcom/Socket.h>
#include <rcom/SocketFactory.h>
#include <rcom/RcomClient.h>

#include <util/Clock.h>
#include <util/ClockAccessor.h>
#include <rover/RoverOptions.h>
#include <rpc/RemoteFunctionCallNames.h>
#include <rpc/RcomLog.h>
#include <oquam/StepperSettings.h>
#include <data_provider/RomiDeviceData.h>
#include <data_provider/SoftwareVersion.h>
#include <session/Session.h>
#include <data_provider/Gps.h>
#include <data_provider/GpsLocationProvider.h>
#include <ui/ScriptList.h>
#include <ui/ScriptMenu.h>
#include <rpc/ScriptHubListener.h>

std::atomic<bool> quit(false);
using namespace std::chrono_literals;

void SignalHandler(int signal)
{
        if (signal == SIGSEGV){
                syslog(1, "rcom-registry segmentation fault");
                exit(signal);
        } else if (signal == SIGINT) {
                r_info("Ctrl-C Quitting Application");
                perror("init_signal_handler");
                quit = true;
        } else {
                r_err("Unknown signam received %d", signal);
        }
}

bool send_membuffer(std::unique_ptr<rcom::IWebSocket>& webclient,
                    rcom::MemBuffer& memBuffer)
{

        std::cout << "sending: " << memBuffer.tostring() << std::endl;

        webclient->send(memBuffer, rcom::kTextMessage );
        
        // TBD: 100 second for debugging remotely, change to something
        // reasonable before checkin.
        auto recv = webclient->recv(memBuffer, 100.0);

        if (recv ==  rcom::RecvStatus::kRecvText) {
                std::cout << "success received: " << memBuffer.tostring() << std::endl;
                return true;
        } else {
                std::cout << "receive error <" << recv << ">" << std::endl;
                return false;
        }
}

bool send_execute_script(std::unique_ptr<rcom::IWebSocket>& webclient, romi::Script& script)
{
        rcom::MemBuffer memBuffer;
        memBuffer.printf(R"({ "%s" : "%s", "%s" : "%s"})",
                         romi::RemoteMessgeTypes::message_type_request,
                         romi::RemoteMessgeTypes::request_type_execute,
                         romi::RemoteMessgeTypes::execute_type_id,
                         script.id.c_str());
        return send_membuffer(webclient, memBuffer);
}


bool send_execute_state(std::unique_ptr<rcom::IWebSocket>& webclient, const char *state)
{
        rcom::MemBuffer memBuffer;
        memBuffer.printf(R"({ "%s" : "%s", "%s" : "%s"})",
                         romi::RemoteMessgeTypes::message_type_request,
                         romi::RemoteMessgeTypes::request_type_execute,
                         romi::RemoteMessgeTypes::execute_type_id,
                         state);
        return send_membuffer(webclient, memBuffer);
}


std::unique_ptr<rcom::IWebSocket>
try_create_remote_connection(rcom::Address& remote_address)
{
        std::shared_ptr<rcom::ILinux> linux = std::make_shared<rcom::Linux>();
        std::shared_ptr<rcom::ILog> log = std::make_shared<romi::RcomLog>();
        rcom::SocketFactory socketFactory(linux, log);
        std::unique_ptr<rcom::IWebSocket> webclient(nullptr);
        while (!quit && (webclient == nullptr)) {
                try {
                        webclient = socketFactory.new_client_side_websocket(remote_address);
                } catch (std::runtime_error& e) {
                        r_warn("Failed to connect to romi-rover retry in 2 seconds.");
                        std::this_thread::sleep_for(2s);
                }
        }
        return webclient;
}




// OH MAY I BE ETERNALLY PUNISHED for committing this. It's a quick test, but I'm sure it will end up in the code base.
// Must be refactored before being added to real code base.
// 1) Build up a map of menu items -> commands to send
// 2) Refactor send functionality.
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

                rcom::Linux linux;

                // Session
                r_info("main: Creating session");
                romi::RomiDeviceData romiDeviceData("RemoteClient", "NA");
                romi::SoftwareVersion softwareVersion;
                romi::Gps gps;
                std::unique_ptr<romi::ILocationProvider> locationProvider
                        = std::make_unique<romi::GpsLocationProvider>(gps);
                std::string session_directory = ".";

                romi::Session session(linux, session_directory, romiDeviceData,
                                      softwareVersion, std::move(locationProvider));

                rcom::Address remote_address(romi::ScriptHubListeningPort);

                if (options.is_set(romi::RoverOptions::kRegistry)) {
                        std::string ip = options.get_value(romi::RoverOptions::kRegistry);
                        r_info("Registry IP set to %s", ip.c_str());
                        remote_address.set(ip.c_str(), romi::ScriptHubListeningPort);
                }

                auto webclient = try_create_remote_connection(remote_address);

                rcom::MemBuffer memBuffer;
                memBuffer.printf(R"({ "request" : "list" })");
                webclient->send(memBuffer, rcom::kTextMessage);
                // TBD: 100 second for debugging remotely, change to
                // something reasonable before checkin.
                auto recv = webclient->recv(memBuffer, 100.0);

                nlohmann::json jsonScripts = nlohmann::json::parse(memBuffer.tostring());

                romi::ScriptList scriptList(jsonScripts);
                if (recv != rcom::RecvStatus::kRecvText) {
                        std::cout << "Unable to retrive script list error <"
                                  << recv << ">" << std::endl;
                        quit = true;
                }

                while (!quit) {
                        try {
                                using namespace std::chrono_literals;

                                // TBD: Build the menu properly please.
                                std::cout << std::endl << "SCRIPTS" << std::endl
                                          << "=======" << std::endl << std::endl;
                                size_t index = 0;
                                for (index = 0; index < scriptList.size(); index++) {
                                        auto script = scriptList[index];
                                        std::cout << index << ">\t" << script.id << "\t("
                                                  << script.title << ")" << std::endl;
                                }
                                std::cout << index++ << ">\t" << "script" << "\t("
                                          << romi::RemoteMessgeTypes::execute_type_start_navigation
                                          << ")" << std::endl;
                                std::cout << index++ << ">\t" << "script" << "\t("
                                          << romi::RemoteMessgeTypes::execute_type_stop_navigation
                                          << ")" << std::endl;
                                size_t max_selection = index;
                                std::cout << max_selection << ">\tQuit" << std::endl << std::endl;
                                std::string line;
                                std::cout << "Select > ";
                                std::getline(std::cin, line);

                                if (!line.empty()) {
                                        size_t selection = std::stoul(line);
                                        if (selection == max_selection)
                                                quit = true;
                                        else if (selection < scriptList.size()) {
                                                auto script = scriptList[selection];
                                                send_execute_script(webclient, script);
                                        } else if (selection == scriptList.size()) {
                                                send_execute_state(webclient, romi::RemoteMessgeTypes::execute_type_start_navigation);
                                        } else if (selection == scriptList.size()+1) {
                                                send_execute_state(webclient, romi::RemoteMessgeTypes::execute_type_stop_navigation);
                                        }
                                }
                                std::this_thread::sleep_for(5ms);
                        } catch (std::exception & e) {
                                quit = true;
                                throw;
                        }
                } // End While
                
                retval = 0;
                
        } catch (nlohmann::json::exception& je) {
                r_err("main: Invalid configuration file: %s", je.what());

        } catch (std::exception& e) {
                r_err("main: exception: %s", e.what());
        }
        
        return retval;
}

