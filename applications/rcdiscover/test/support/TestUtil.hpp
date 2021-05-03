///
/// \file 			TestUtil.hpp
/// \author 		Geoffrey Hunter <gbmhunter@gmail.com> (www.mbedded.ninja)
/// \created		2017-11-24
/// \last-modified 	2017-11-24
/// \brief			Contains utility methods to help with testing.
/// \details
///					See README.rst in repo root dir for more info.

#ifndef MN_CPP_LINUX_SERIAL_TEST_UTIL_H_
#define MN_CPP_LINUX_SERIAL_TEST_UTIL_H_

// System includes
#include <string>
#include <array>
#include <memory>
#include <iostream>
#include <thread>
#include <chrono>

using namespace std::literals;

namespace CppLinuxSerial {

        class TestUtil {

        public:

            static TestUtil& GetInstance() {
                static TestUtil testUtil;
                return testUtil;
            }

            /// \brief      Executes a command on the Linux command-line.
            /// \details    Blocks until command is complete.
            /// \throws     std::runtime_error is popen() fails.
            std::string Exec(const std::string &cmd) {
                std::array<char, 128> buffer;
                std::string result;
                std::shared_ptr<FILE> pipe(popen(cmd.c_str(), "r"), pclose);
                if (!pipe) throw std::runtime_error("popen() failed!");

                while (!feof(pipe.get())) {
                    if (fgets(buffer.data(), 128, pipe.get()) != nullptr)
                        result += buffer.data();
                }

                return result;
            }

            void CreateVirtualSerialPortPair() {
                std::cout << "Creating virtual serial port pair..." << std::endl;
                std::string command = std::string("nohup sudo socat -d -d pty,raw,echo=0,link=")
                             + device0Name_ + std::string(" pty,raw,echo=0,link=")
                             + device1Name_ + std::string(" &");
                std::cout << command << std::endl;
                std::system(command.c_str() );

                // Hacky! Since socat is detached, we have no idea at what point it has created
                // ttyS10 and ttyS11. Assume 1 seconds is long enough...
                std::this_thread::sleep_for(2s);
                std::system((std::string("sudo chmod a+rw ") + GetDevice0Name()).c_str());
                std::system((std::string("sudo chmod a+rw ") + GetDevice1Name()).c_str());
            }

            void CloseSerialPorts() {
                // Dangerous! Kills all socat processes running
                // on computer
                std::system("sudo pkill socat");
            }

            std::string GetDevice0Name() {
                return device0Name_;
            }

            std::string GetDevice1Name() {
                return device1Name_;
            }

            std::string device0Name_ = "/dev/ttyS10";
            std::string device1Name_ = "/dev/ttyS11";

        protected:

            TestUtil() {

            }

        };
} // namespace CppLinuxSerial

#endif // #ifndef MN_CPP_LINUX_SERIAL_TEST_UTIL_H_
