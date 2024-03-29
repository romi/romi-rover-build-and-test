#include <string>
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <json.hpp>
#include "PortConfigurationGenerator.h"

namespace fs = std::filesystem;
using testing::UnorderedElementsAre;
using testing::Return;

class PortConfigurationGenerator_tests : public ::testing::Test
{
protected:
	PortConfigurationGenerator_tests() = default;

	~PortConfigurationGenerator_tests() override = default;

	void SetUp() override {
	}

	void TearDown() override {
	}

        std::string normalize_json(const char *s) {
                nlohmann::json json_text = nlohmann::json::parse(s);
                std::string out = json_text.dump(4);
                return out;
        }
protected:

};

TEST_F(PortConfigurationGenerator_tests, PortConfigurationGenerator_can_construct)
{
    // Arrange
    // Act
    //Assert
    ASSERT_NO_THROW(PortConfigurationGenerator PortConfigurationGenerator);
}

TEST_F(PortConfigurationGenerator_tests, PortConfigurationGenerator_json_throws_as_expected)
{
    // Arrange
    nlohmann::json config;
    // Act
    // Assert
    ASSERT_THROW(double width = config["oquam"]["cnc-range"][0][1]; std::cout << width << std::endl;, nlohmann::json::exception);
    ASSERT_THROW(std::string display_device = config["ports"]["display-device"]["port"];std::cout << display_device << std::endl;, nlohmann::json::exception);
}

TEST_F(PortConfigurationGenerator_tests, PortConfigurationGenerator_when_no_devices_present_creates_skeleton)
{
    // Arrange
    std::string json_configuration;
    std::vector<std::pair<std::string, std::string>> attached_devices;
    PortConfigurationGenerator PortConfigurationGenerator;
    std::string expected_json = normalize_json("{\"ports\": {}}");
    std::string file_name("serial_config.json");
    remove(file_name.c_str());

    // Act
    int actual_create = PortConfigurationGenerator.CreateConfigurationFile(json_configuration, attached_devices, file_name);
    std::string actual = PortConfigurationGenerator.LoadConfiguration(file_name);

    //Assert
    ASSERT_THAT(actual, testing::HasSubstr(expected_json));
    ASSERT_EQ(actual_create, true);
}

TEST_F(PortConfigurationGenerator_tests, PortConfigurationGenerator_when_devices_present_creates_config)
{
    // Arrange
    std::string json_configuration;
    std::vector<std::pair<std::string, std::string>> attached_devices;
    PortConfigurationGenerator PortConfigurationGenerator;

    attached_devices.emplace_back(std::string("/dev/ACM0"), std::string("brushmotorcontroller"));
    attached_devices.emplace_back(std::string("/dev/ACM1"), std::string("cnc"));
    std::string expected_json = normalize_json(R"({"ports": {"brushmotorcontroller": {"port": "/dev/ACM0","type": "serial"},"cnc": {"port": "/dev/ACM1","type": "serial"}}})");

    std::string file_name("serial_config.json");
    remove(file_name.c_str());

    // Act
    auto actual_create = PortConfigurationGenerator.CreateConfigurationFile(json_configuration, attached_devices, file_name);
    auto actual = PortConfigurationGenerator.LoadConfiguration(file_name);

    //Assert
    ASSERT_EQ(actual, expected_json);
    ASSERT_EQ(actual_create, true);
}

TEST_F(PortConfigurationGenerator_tests, PortConfigurationGenerator_when_existing_invalid_json_creates_new_config)
{
    // Arrange
    std::string json_configuration("This is not json data: is it? : I don't think so");
    std::vector<std::pair<std::string, std::string>> attached_devices;
    PortConfigurationGenerator PortConfigurationGenerator;

    attached_devices.emplace_back(std::string("/dev/ACM0"), std::string("brushmotorcontroller"));
    attached_devices.emplace_back(std::string("/dev/ACM1"), std::string("cnc"));
    std::string expected_json = normalize_json(R"({"ports": {"brushmotorcontroller": {"port": "/dev/ACM0","type": "serial"},"cnc": {"port": "/dev/ACM1","type": "serial"}}})");
    std::string file_name("serial_config.json");
    remove(file_name.c_str());

    // Act
    auto actual_create = PortConfigurationGenerator.CreateConfigurationFile(json_configuration, attached_devices, file_name);
    auto actual = PortConfigurationGenerator.LoadConfiguration(file_name);

    //Assert
    ASSERT_EQ(actual, expected_json);
    ASSERT_EQ(actual_create, true);
}

TEST_F(PortConfigurationGenerator_tests, PortConfigurationGenerator_when_existing_valid_json_adds_to_config)
{
    // Arrange
    std::string json_configuration(R"({"some_json_key":{"cnc":{"port":"/dev/ACM1","type":"serial"}}})");
    std::vector<std::pair<std::string, std::string>> attached_devices;
    PortConfigurationGenerator PortConfigurationGenerator;

    attached_devices.emplace_back(std::string("/dev/ACM0"), std::string("brushmotorcontroller"));
    attached_devices.emplace_back(std::string("/dev/ACM1"), std::string("cnc"));
    std::string expected_json = normalize_json(R"({"some_json_key": {"cnc": {"port": "/dev/ACM1","type": "serial"}},"ports": {"brushmotorcontroller": {"port": "/dev/ACM0","type": "serial"},"cnc": {"port": "/dev/ACM1","type": "serial"}}})");
    std::string file_name("serial_config.json");
    remove(file_name.c_str());

    // Act
    auto actual_create = PortConfigurationGenerator.CreateConfigurationFile(json_configuration, attached_devices, file_name);
    auto actual = PortConfigurationGenerator.LoadConfiguration(file_name);

    //Assert
    ASSERT_EQ(actual, expected_json);
    ASSERT_EQ(actual_create, true);
}

TEST_F(PortConfigurationGenerator_tests, PortConfigurationGenerator_create_fails_when_cant_write)
{
    // Arrange
    std::string json_configuration;
    std::vector<std::pair<std::string, std::string>> attached_devices;
    PortConfigurationGenerator PortConfigurationGenerator;
    std::string expected_json("{\"ports\": {}}");
    std::string file_name("/impossible/serial_config.json");

    // Act
    auto actual_create = PortConfigurationGenerator.CreateConfigurationFile(json_configuration, attached_devices, file_name);

    //Assert
    ASSERT_NE(actual_create, true);
}

TEST_F(PortConfigurationGenerator_tests, PortConfigurationGenerator_save_configuration_fails_when_cant_write)
{
    // Arrange
    PortConfigurationGenerator PortConfigurationGenerator;
    std::string file_contents("The quick brown fox jumped over the lazy dog.");
    std::string file_name("/impossible/serial_config.json");
    // Act
    auto actual = PortConfigurationGenerator.SaveConfiguration(file_name, file_contents);

    //Assert
    ASSERT_FALSE(actual);
}

TEST_F(PortConfigurationGenerator_tests, PortConfigurationGenerator_save_configuration_succeeds_when_written_can_be_loaded)
{
    // Arrange
    PortConfigurationGenerator PortConfigurationGenerator;
    std::string expected_file_contents("The quick brown fox jumped over the lazy dog.");
    std::string file_name("serial_config.json");

    remove(file_name.c_str());

    // Act
    auto actual = PortConfigurationGenerator.SaveConfiguration(file_name, expected_file_contents);
    std::string actual_file_contents = PortConfigurationGenerator.LoadConfiguration(file_name);

    //Assert
    ASSERT_EQ(actual_file_contents, expected_file_contents);
    ASSERT_TRUE(actual);
}

TEST_F(PortConfigurationGenerator_tests, PortConfigurationGenerator_load_configuration_fails_returns_empty_string)
{
    // Arrange
    PortConfigurationGenerator PortConfigurationGenerator;
    std::string file_name("serial_config_noexist.json");
    remove(file_name.c_str());

    // Act
    auto actual = PortConfigurationGenerator.LoadConfiguration(file_name);

    //Assert
    ASSERT_TRUE(actual.empty());
}
