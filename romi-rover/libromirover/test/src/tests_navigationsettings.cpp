#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "NavigationSettings.h"

using namespace std;
using namespace testing;
using namespace romi;

class navigationsettings_tests : public ::testing::Test
{
protected:
        JsonCpp config;
        
	navigationsettings_tests() {
                const char * config_string = "{"
                        "'wheel_diameter': 1.0,"
                        "'wheel_base': 2.0,"
                        "'encoder_steps': 1000.0,"
                        "'maximum_speed': 3.0 }";
                config = JsonCpp::parse(config_string);
	}

	~navigationsettings_tests() override = default;

	void SetUp() override {
	}

	void TearDown() override {
	}
};

TEST_F(navigationsettings_tests, parse_config)
{
        NavigationSettings rover(config);
        
        //Assert
        ASSERT_EQ(rover.wheel_diameter, 1.0);
        ASSERT_EQ(rover.wheel_base, 2.0);
        ASSERT_EQ(rover.encoder_steps, 1000.0);
        ASSERT_EQ(rover.wheel_circumference, M_PI);
        ASSERT_EQ(rover.max_revolutions_per_sec, 3.0/M_PI);
}
