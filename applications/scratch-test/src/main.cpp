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
//                session.start("hw_observation_id");

//            std::string registry = "192.168.0.179";
//            if (!registry.empty())
                rcom::RegistryServer::set_address("10.0.3.35");

   //             auto rpc_ = romi::RcomClient::create("python", 30);

            double timeout_seconds = 10;
            r_debug("Main::Create Link");
            std::unique_ptr<rcom::IMessageLink> link
                    = std::make_unique<rcom::MessageLink>("python", timeout_seconds);
            r_debug("Main::Create done");

            std::string path("");
            std::string output_name("");
            std::string function_name("unet");
            JsonCpp response;
            romi::RPCError error;
            JsonCpp params = JsonCpp::construct("{\"path\": \"%s\", "
                                                "\"output-name\": \"%s\"}",
                                                path.c_str(), output_name.c_str());



//                auto client = std::make_unique<romi::RcomClient>(link, timeout_seconds);;
//                client->execute(function_name, params, response, error);


//                romi::PythonUnet pythonUnet;
//                std::string image_file("./camera.jpg");
//                std::unique_ptr<romi::FileCamera> camera = std::make_unique<romi::FileCamera>(image_file);
//
//                // Imager
//                //romi::Imager imager(session, *camera);
//                std::string observation_id("observe");
//                romi::UnetImager imager(session, *camera);
//                imager.start_recording(observation_id, 5, 500.00);

                while (!quit) {
                        
                        try {
                            using namespace std::chrono_literals;
//                            if (!imager.is_recording())
//                            {
//                                imager.stop_recording();
//                                quit = true;
//                                r_info("Recording stopped Quitting Application");
//                            }
/////////////////////////////////////// Loop Create Link
//                            r_debug("Main::Create Link");
//                            std::unique_ptr<rcom::IMessageLink> link
//                                    = std::make_unique<rcom::MessageLink>("python", timeout_seconds);
//                            r_debug("Main::Create client");
//                            auto client = std::make_unique<romi::RcomClient>(link, timeout_seconds);;
//                            client->execute(function_name, params, response, error);
/////////////////////////////////////////////////////////////////////////////////////////////////
                            std::this_thread::sleep_for(5ms);
                        } catch (std::exception& e) {
                            quit = true;
                                throw;
                        }
                }
//                imager.stop_recording();

                retval = 0;

                
        } catch (JSONError& je) {
                r_err("main: Invalid configuration file: %s", je.what());
                
        } catch (std::exception& e) {
                r_err("main: exception: %s", e.what());
        }
        
        return retval;
}
