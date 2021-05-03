#include <filesystem>
#include <fstream>
#include <string>
#include "gtest/gtest.h"
#include "DeviceLister.h"
#include "SerialPortDiscover_mock.h"

namespace fs = std::filesystem;
using testing::UnorderedElementsAre;
using testing::Return;

class DeviceLister_tests : public ::testing::Test
{
protected:
	DeviceLister_tests() = default;

	~DeviceLister_tests() override = default;

	void SetUp() override {
        }

	void TearDown() override {
                DeleteFiles();
        }

	void CreateFiles() {
                if (!fs::is_directory(directory)) {
                        fs::create_directories(directory);
                        std::ofstream(ACM0.c_str());
                        std::ofstream(ACM1.c_str());
                        std::ofstream(USB1.c_str());
                        std::ofstream(TTYS0.c_str());
                }
        }

        void DeleteFiles() {
                if (fs::is_directory(fs::path(directory)))
                {
                        fs::remove_all(directory.c_str());
                }
        }

protected:
        const std::string directory = "./dev";
        const std::string ACM0 = directory + "/ttyACM0";
        const std::string ACM1 = directory + "/ttyACM1";
        const std::string USB1 = directory + "/ttyUSB1";
        const std::string TTYS0 = directory + "/ttyS0";

};

TEST_F(DeviceLister_tests, SerialPortIdentification_can_construct)
{
    // Arrange

    // Act
    //Assert
    ASSERT_NO_THROW(DeviceLister deviceLister);
}

TEST_F(DeviceLister_tests, ListFilesOfType_lists_correct_devices)
{
    // Arrange
    CreateFiles();
    DeviceLister deviceLister;
    std::string type("ACM");

    // Act
    std::vector<std::string> actual;
    deviceLister.ListFilesOfType(directory, type, actual);

    //Assert
    ASSERT_EQ(actual.size(), 2);
    EXPECT_THAT(actual, UnorderedElementsAre(ACM0, ACM1));
}

TEST_F(DeviceLister_tests, ListFilesOfType_returns_empty_when_type_empty)
{
    // Arrange
    CreateFiles();
    DeviceLister deviceLister;
    std::string type("");

    // Act
    std::vector<std::string> actual;
    deviceLister.ListFilesOfType(directory, type, actual);

    //Assert
    ASSERT_EQ(actual.size(), 0);
}

TEST_F(DeviceLister_tests, ListFilesOfType_returns_empty_when_no_devices_match)
{
    // Arrange
    CreateFiles();
    DeviceLister deviceLister;
    std::string type("XXX");

    // Act
    std::vector<std::string> actual;
    deviceLister.ListFilesOfType(directory, type, actual);

    //Assert
    ASSERT_EQ(actual.size(), 0);
}

TEST_F(DeviceLister_tests, ListFilesOfType_returns_empty_when_invalid_path)
{
    // Arrange
    CreateFiles();
    DeviceLister deviceLister;
    std::string type("XXX");
    std::string invalid_directory = "/invalid/root/path";

    // Act
    std::vector<std::string> actual;
    deviceLister.ListFilesOfType(invalid_directory, type, actual);

    //Assert
    ASSERT_EQ(actual.size(), 0);
}
