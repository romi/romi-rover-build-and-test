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

#include <Linux.h>
#include <Clock.h>
#include <ClockAccessor.h>
#include <MemBuffer.h>
#include <rover/RoverOptions.h>

#include <rpc/RemoteCamera.h>
#include <rpc/RcomClient.h>

#include <oquam/StepperSettings.h>
#include <data_provider/RomiDeviceData.h>
#include <data_provider/SoftwareVersion.h>
#include <session/Session.h>
#include <data_provider/Gps.h>
#include <data_provider/GpsLocationProvider.h>
#include <RegistryServer.h>
#include <MessageLink.h>

#include <ui/ScriptList.h>
#include <ui/ScriptMenu.h>
#include <rpc/ScriptHub.h>
#include <rpc/ScriptHubListener.h>
#include <Socket.h>
#include <SocketFactory.h>


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
bool send_execute_script(std::unique_ptr<rcom::IWebSocket>& webclient, romi::Script& script)
{
            rpp::MemBuffer memBuffer;
            memBuffer.printf(R"({ "request" : "execute", "id" : "%s"})", script.id.c_str());
            std::cout << "sending: " << memBuffer.tostring() << std::endl;

            webclient->send(memBuffer, rcom::kTextMessage );
            auto recv = webclient->recv(memBuffer, 2.0);

            if (recv ==  rcom::RecvStatus::kRecvText)
            {

                std::cout << "success received: " << memBuffer.tostring() << std::endl;
                return true;
            }
            else
            {
                std::cout << "receive error <" << recv << ">" << std::endl;
                return false;
            }
}
int main(int argc, char** argv)
{
        std::shared_ptr<rpp::IClock> clock = std::make_shared<rpp::Clock>();
        rpp::ClockAccessor::SetInstance(clock);

        int retval = 1;

        romi::RoverOptions options;
        options.parse(argc, argv);
        options.exit_if_help_requested();

        r_log_init();
        r_log_set_app("romi-rover");

        std::signal(SIGSEGV, SignalHandler);
        std::signal(SIGINT, SignalHandler);

        try {

                rpp::Linux linux;

                // Session
                r_info("main: Creating session");
                romi::RomiDeviceData romiDeviceData;
                romi::SoftwareVersion softwareVersion;
                romi::Gps gps;
                std::unique_ptr<romi::ILocationProvider> locationProvider
                        = std::make_unique<romi::GpsLocationProvider>(gps);
                std::string session_directory =".";

                romi::Session session(linux, session_directory, romiDeviceData,
                                      softwareVersion, std::move(locationProvider));


                rcom::Address remote_address(ScriptHubListeningPort);

                rcom::SocketFactory  socketFactory;
                auto webclient = socketFactory.new_client_side_websocket(remote_address);
                rpp::MemBuffer memBuffer;
                memBuffer.printf(R"({ "request" : "list" })");
                webclient->send(memBuffer, rcom::kTextMessage );
                auto recv = webclient->recv(memBuffer, 2.0);

                JsonCpp jsonScripts = JsonCpp::parse(memBuffer);

                romi::ScriptList scriptList(jsonScripts);
                if (recv !=  rcom::RecvStatus::kRecvText)
                {
                    std::cout << "Unable to retrive script list error <" << recv << ">" << std::endl;
                    quit = true;
                }

                while (!quit) {

                        try {
                            using namespace std::chrono_literals;

                            std::cout << std::endl << "SCRIPTS" <<  std::endl << "=======" << std::endl << std::endl;
                            size_t index = 0;
                            for (index = 0; index < scriptList.size(); index++)
                            {
                                auto script = scriptList[index];
                                std::cout << index << ">\t" << script.id << "\t(" << script.title << ")" << std::endl;
                            }
                            size_t max_selection = index;
                            std::cout << max_selection << ">\tQuit" << std::endl << std::endl;
                            std::string line;
                            std::cout << "Select > ";
                            std::getline(std::cin, line);

                            if (!line.empty())
                            {
                                size_t selection = std::stoul(line);
                                if (selection == max_selection)
                                    quit = true;
                                if (selection < max_selection)
                                {
                                    auto script = scriptList[selection];
                                    send_execute_script(webclient, script);
                                }
                            }

                            std::this_thread::sleep_for(5ms);

                        } catch (std::exception& e) {
                            quit = true;
                                throw;
                        }
                }

                retval = 0;

                
        } catch (JSONError& je) {
                r_err("main: Invalid configuration file: %s", je.what());
                
        } catch (std::exception& e) {
                r_err("main: exception: %s", e.what());
        }
        
        return retval;
}

