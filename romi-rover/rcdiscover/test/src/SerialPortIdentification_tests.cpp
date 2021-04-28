#include <filesystem>
#include <fstream>
#include <string>
#include "gtest/gtest.h"
#include "DeviceLister.h"
#include "SerialPortIdentification.h"
#include "SerialPortDiscover_mock.h"

namespace fs = std::filesystem;
using testing::UnorderedElementsAre;
using testing::Return;

class SerialPortIdentification_tests : public ::testing::Test
{
protected:
	SerialPortIdentification_tests() = default;

	~SerialPortIdentification_tests() override = default;

	void SetUp() override
    {
	}

	void TearDown() override
    {
        DeleteFiles();
	}

	void CreateFiles()
    {
        if (!fs::is_directory(directory))
        {
            fs::create_directories(directory);
            std::ofstream(ACM0.c_str());
            std::ofstream(ACM1.c_str());
            std::ofstream(USB1.c_str());
            std::ofstream(TTYS0.c_str());
        }
    }

    void DeleteFiles()
    {
        if (fs::is_directory(fs::path(directory)))
        {
            fs::remove_all(directory.c_str());
        }
    }

protected:
        const std::string directory = "./dev";
	    const std::string ACM0 = directory + "/ACM0";
        const std::string ACM1 = directory + "/ACM1";
        const std::string USB1 = directory + "/USB1";
        const std::string TTYS0 = directory + "/ttyS0";

};

TEST_F(SerialPortIdentification_tests, SerialPortIdentification_can_construct)
{
    // Arrange
    Mocks::SerialPortDiscoverMock serialPortDiscoverMock;

    // Act
    //Assert
    ASSERT_NO_THROW(SerialPortIdentification serialPortIdentification(serialPortDiscoverMock));
}

TEST_F(SerialPortIdentification_tests, Connected_devices_returns_empty_vector_when_no_devices)
{
    // Arrange
    CreateFiles();
    Mocks::SerialPortDiscoverMock serialPortDiscoverMock;
    EXPECT_CALL(serialPortDiscoverMock, ConnectedDevice(testing::_))
            .Times(2)
            .WillRepeatedly(Return(std::string()));

    SerialPortIdentification serialPortIdentification(serialPortDiscoverMock);
    std::string type = "ACM";

    // Act
    std::vector<std::string> actual;
    DeviceLister deviceLister;
    deviceLister.ListFilesOfType(directory, type, actual);
    DeviceMap actual_devices;
    serialPortIdentification.ConnectedDevices(actual, actual_devices);

    //Assert
    ASSERT_EQ(actual_devices.size(), 0);
}

TEST_F(SerialPortIdentification_tests, Connected_devices_returns_valid_devices)
{
    // Arrange
    std::string BrushMotorController("BrushMotorController");
    std::string CNC("CNC");
    Mocks::SerialPortDiscoverMock serialPortDiscoverMock;
    EXPECT_CALL(serialPortDiscoverMock, ConnectedDevice(testing::_))
            .Times(3)
            .WillOnce(Return(BrushMotorController))
            .WillOnce(Return(std::string("")))
            .WillOnce(Return(CNC));

    SerialPortIdentification serialPortIdentification(serialPortDiscoverMock);
    std::vector<std::string> devices;
    devices.push_back(ACM0);
    devices.push_back(ACM1);
    devices.push_back(USB1);

    // Act
    DeviceMap actual_devices;
    serialPortIdentification.ConnectedDevices(devices, actual_devices);

    //Assert
    ASSERT_EQ(actual_devices.size(), 2);
    EXPECT_THAT(actual_devices, UnorderedElementsAre(std::make_pair(ACM0,BrushMotorController), std::make_pair(USB1, CNC)));
}
