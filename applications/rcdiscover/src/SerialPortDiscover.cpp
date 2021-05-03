#include <log.h>
#include <iostream>
#include <cstring>
#include <thread>
#include <chrono>
#include <RomiSerialClient.h>
#include "StringUtils.h"
#include "SerialPortDiscover.h"

SerialPortDiscover::SerialPortDiscover(const std::map<std::string,
                                       std::string>& deviceFilter)
        : knownDevicesMap(deviceFilter)
{
}

std::string SerialPortDiscover::ConnectedDevice(const std::string& path)
{
        std::string device;
        try {
                std::cout << "Connecting to " << path << std::endl;
                device = TryConnectedDevice(path);

                if (!device.empty())
                        std::cout << "Found: " << device << std::endl;
                else
                        std::cout << "No device found" << std::endl;
                        
        } catch (std::exception& e) {
                std::cerr << "Exception: " << e.what() << std::endl;
        }
        return device;
}

std::string SerialPortDiscover::TryConnectedDevice(const std::string& path)
{
        const int retrycount = 3;
        int retrycurrent = 0;
        bool checked = false;
        std::string device;
        std::string name;
        JsonCpp response;
        
        auto client = romiserial::RomiSerialClient::create(path);
        
        while ((checked == false) && (retrycurrent++ < retrycount)) {
                std::cout << "Attempt " << retrycurrent << std::endl;
                client->send("?", response);
                if (response.num(romiserial::kStatusCode) == romiserial::kNoError) {
                        name = response.str(1);
                        std::cout << path << " " << name << std::endl;
                        device = FilterDevice(name);
                        if (!device.empty())
                                checked = true;
                } else {
                        std::cout << "Failed: " << response.str(romiserial::kErrorMessage)
                                  << ", err=" << response.num(romiserial::kStatusCode)
                                  << std::endl;
                }
        }
        return device;
}

std::string SerialPortDiscover::FilterDevice(std::string& info)
{
        std::string retval;
        for (const auto& device : knownDevicesMap) {
                if (info.find(device.first) != std::string::npos) {
                        retval = device.second;
                        break;
                }
        }
        return retval;
}
