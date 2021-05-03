#include <filesystem>
#include <fstream>
#include <string>
#include <future>
#include <thread>

#include <r.h>
#include <RomiSerial.h>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "SerialPortDiscover.h"
#include "TestUtil.hpp"

namespace fs = std::filesystem;
using testing::UnorderedElementsAre;
using testing::Return;

bool fake_serial_ready = false;
std::mutex thread_mutex;

class SerialPortDiscover_tests : public ::testing::Test
{
protected:
	SerialPortDiscover_tests() = default;

	~SerialPortDiscover_tests() override = default;

	void SetUp() override {
                fake_serial_ready = false;
	}

	void TearDown() override {
	}

	void CreateFiles() {
        }

        void DeleteFiles() {
        }

protected:
    const std::map<std::string, std::string> device_to_json_key =
            {
                    { "Grbl", "grbl" },
                    { "BrushMotorController","brush_motors" },
                    { "RomiControlPanel",  "control_panel" }
            };

};

TEST_F(SerialPortDiscover_tests, SerialPortDiscover_can_construct)
{
        // Arrange
        // Act
        //Assert
        ASSERT_NO_THROW(SerialPortDiscover SerialPortDiscover(device_to_json_key));
}

TEST_F(SerialPortDiscover_tests, SerialPortDiscover_ConnectedDevice_bad_port_returns_empty_string)
{
        // Arrange
        SerialPortDiscover SerialPortDiscover(device_to_json_key);
        // Act
        auto device = SerialPortDiscover.ConnectedDevice(std::string("/dev/notreal"));

        //Assert
        ASSERT_TRUE(device.empty());
}

TEST_F(SerialPortDiscover_tests, SerialPortDiscover_ConnectedDevice_times_out_returns_empty_string)
{
        // Arrange
        SerialPortDiscover SerialPortDiscover(device_to_json_key);
        std::string port0 = CppLinuxSerial::TestUtil::GetInstance().GetDevice0Name();
        // Act
        auto device = SerialPortDiscover.ConnectedDevice(port0);

        //Assert
        ASSERT_TRUE(device.empty());
}

// FIXME
std::string info_string_;
bool sent_info = false;

void MakeInfoString(const std::string& device_name)
{
        info_string_ = "[0,\"";
        info_string_ += device_name;
        info_string_ += "\"]";
}

void SendInfo(romiserial::RomiSerial *romi_serial, int16_t *, const char *)
{
        romi_serial->send(info_string_.c_str());
        sent_info = true;
}

int FakeSerialDeviceFunction(const std::string& port,
                             const std::string& device_name,
                             std::condition_variable *cv)
{
        // FIXME
        MakeInfoString(device_name);
        sent_info = false;
        
        romiserial::MessageHandler handlers[] = {
                { '?', 0, false, SendInfo }
        };
        
        romiserial::RSerial serial(port, 115200, 0);
        romiserial::RomiSerial romi_serial(serial, serial, handlers, 1);
        
        std::cout << "FakeSerialDeviceFunction: Notify " << std::endl;
        {
                std::lock_guard<std::mutex> lk(thread_mutex);
                fake_serial_ready = true;
                cv->notify_one();
        }
        
        while (!sent_info) {
                romi_serial.handle_input();
        }

        return sent_info? 0 : -1;
}

void wait_notification(std::condition_variable& cv)
{
        std::cout << "Wait for notify " << std::endl;
        std::unique_lock<std::mutex> lk(thread_mutex);
        cv.wait(lk, [] { return fake_serial_ready == true; });
        std::cout << "Received notify " << std::endl;
}

TEST_F(SerialPortDiscover_tests, SerialPortDiscover_ConnectedDevice_returns_when_device_not_known)
{
        // Arrange
        std::condition_variable cv;
        std::string actual_device;
        std::string serial_device_name("unknown_device_name");
        std::string expected_device;
        std::string port0 = CppLinuxSerial::TestUtil::GetInstance().GetDevice0Name();
        std::string port1 = CppLinuxSerial::TestUtil::GetInstance().GetDevice1Name();

        SerialPortDiscover SerialPortDiscover(device_to_json_key);
        auto future = std::async(FakeSerialDeviceFunction, port1, serial_device_name, &cv);

        wait_notification(cv);
        
        // Act
        actual_device = SerialPortDiscover.ConnectedDevice(port0);
        int thread_success = future.get();

        // Assert
        ASSERT_EQ(expected_device, actual_device);
        ASSERT_EQ(thread_success, 0);
}

TEST_F(SerialPortDiscover_tests, SerialPortDiscover_ConnectedDevice_returns_when_device_whitespace)
{
        // Arrange
        std::condition_variable cv;
        std::string actual_device;
        std::string serial_device_name(" ");
        std::string expected_device;
        std::string port0 = CppLinuxSerial::TestUtil::GetInstance().GetDevice0Name();
        std::string port1 = CppLinuxSerial::TestUtil::GetInstance().GetDevice1Name();

        SerialPortDiscover SerialPortDiscover(device_to_json_key);
        auto future = std::async(FakeSerialDeviceFunction, port1, serial_device_name, &cv);

        wait_notification(cv);
        
        // Act
        actual_device = SerialPortDiscover.ConnectedDevice(port0);
        int thread_success = future.get();

        // Assert
        ASSERT_EQ(expected_device, actual_device);
        ASSERT_EQ(thread_success, 0);
}

TEST_F(SerialPortDiscover_tests, SerialPortDiscover_ConnectedDevice_returns_connected_device)
{
        // Arrange
        std::condition_variable cv;
        std::string actual_device;
        std::string serial_device_name(device_to_json_key.begin()->first);
        std::string expected_device(device_to_json_key.begin()->second);
        std::string port0 = CppLinuxSerial::TestUtil::GetInstance().GetDevice0Name();
        std::string port1 = CppLinuxSerial::TestUtil::GetInstance().GetDevice1Name();

        SerialPortDiscover SerialPortDiscover(device_to_json_key);
        auto future = std::async(FakeSerialDeviceFunction, port1, serial_device_name, &cv);

        wait_notification(cv);
        
        // Act
        actual_device = SerialPortDiscover.ConnectedDevice(port0);
        int thread_success = future.get();

        // Assert
        ASSERT_EQ(expected_device, actual_device);
        ASSERT_EQ(thread_success, 0);
}
