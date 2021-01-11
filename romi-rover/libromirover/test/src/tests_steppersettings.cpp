#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "oquam/StepperSettings.h"

using namespace std;
using namespace testing;
using namespace romi;

class steppersettings_tests : public ::testing::Test
{
protected:
        
	steppersettings_tests() {}

	~steppersettings_tests() override = default;

	void SetUp() override {}

	void TearDown() override {}
};

TEST_F(steppersettings_tests, test_constructor)
{
        JsonCpp json = JsonCpp::parse("{"
                                      "'steps-per-revolution': [1, 2, 3],"
                                      "'microsteps': [4, 5, 6],"
                                      "'gears-ratio': [7, 8, 9 ],"
                                      "'maximum-rpm': [10, 11, 12],"
                                      "'displacement-per-revolution': [13, 14, 15],"
                                      "'maximum-acceleration': [16, 17, 18]"
                                      "}");
        StepperSettings settings(json);

        ASSERT_EQ(settings.steps_per_revolution[0], 1);
        ASSERT_EQ(settings.steps_per_revolution[1], 2);
        ASSERT_EQ(settings.steps_per_revolution[2], 3);

        ASSERT_EQ(settings.microsteps[0], 4);
        ASSERT_EQ(settings.microsteps[1], 5);
        ASSERT_EQ(settings.microsteps[2], 6);

        ASSERT_EQ(settings.gears_ratio[0], 7);
        ASSERT_EQ(settings.gears_ratio[1], 8);
        ASSERT_EQ(settings.gears_ratio[2], 9);

        ASSERT_EQ(settings.maximum_rpm[0], 10);
        ASSERT_EQ(settings.maximum_rpm[1], 11);
        ASSERT_EQ(settings.maximum_rpm[2], 12);

        ASSERT_EQ(settings.displacement_per_revolution[0], 13);
        ASSERT_EQ(settings.displacement_per_revolution[1], 14);
        ASSERT_EQ(settings.displacement_per_revolution[2], 15);

        ASSERT_EQ(settings.maximum_acceleration[0], 16);
        ASSERT_EQ(settings.maximum_acceleration[1], 17);
        ASSERT_EQ(settings.maximum_acceleration[2], 18);
}

TEST_F(steppersettings_tests, throws_exception_on_missing_values_1)
{
        JsonCpp json = JsonCpp::parse("{"
                                      "'microsteps': [4, 5, 6],"
                                      "'gears-ratio': [7, 8, 9 ],"
                                      "'maximum-rpm': [10, 11, 12],"
                                      "'displacement-per-revolution': [13, 14, 15],"
                                      "'maximum-acceleration': [16, 17, 18]"
                                      "}");
        try {
                StepperSettings settings(json);
                FAIL() << "Expected a runtime_error";
        } catch (std::runtime_error& e) { }
}

TEST_F(steppersettings_tests, throws_exception_on_missing_values_2)
{
        JsonCpp json = JsonCpp::parse("{"
                                      "'steps-per-revolution': [1, 2, 3],"
                                      "'gears-ratio': [7, 8, 9 ],"
                                      "'maximum-rpm': [10, 11, 12],"
                                      "'displacement-per-revolution': [13, 14, 15],"
                                      "'maximum-acceleration': [16, 17, 18]"
                                      "}");
        try {
                StepperSettings settings(json);
                FAIL() << "Expected a runtime_error";
        } catch (std::runtime_error& e) { }
}

TEST_F(steppersettings_tests, throws_exception_on_missing_values_3)
{
        JsonCpp json = JsonCpp::parse("{"
                                      "'steps-per-revolution': [1, 2, 3],"
                                      "'microsteps': [4, 5, 6],"
                                      "'maximum-rpm': [10, 11, 12],"
                                      "'displacement-per-revolution': [13, 14, 15],"
                                      "'maximum-acceleration': [16, 17, 18]"
                                      "}");
        try {
                StepperSettings settings(json);
                FAIL() << "Expected a runtime_error";
        } catch (std::runtime_error& e) { }
}

TEST_F(steppersettings_tests, throws_exception_on_missing_values_4)
{
        JsonCpp json = JsonCpp::parse("{"
                                      "'steps-per-revolution': [1, 2, 3],"
                                      "'microsteps': [4, 5, 6],"
                                      "'gears-ratio': [7, 8, 9 ],"
                                      "'displacement-per-revolution': [13, 14, 15],"
                                      "'maximum-acceleration': [16, 17, 18]"
                                      "}");
        try {
                StepperSettings settings(json);
                FAIL() << "Expected a runtime_error";
        } catch (std::runtime_error& e) { }
}

TEST_F(steppersettings_tests, throws_exception_on_missing_values_5)
{
        JsonCpp json = JsonCpp::parse("{"
                                      "'steps-per-revolution': [1, 2, 3],"
                                      "'microsteps': [4, 5, 6],"
                                      "'gears-ratio': [7, 8, 9 ],"
                                      "'maximum-rpm': [10, 11, 12],"
                                      "'maximum-acceleration': [16, 17, 18]"
                                      "}");
        try {
                StepperSettings settings(json);
                FAIL() << "Expected a runtime_error";
        } catch (std::runtime_error& e) { }
}

TEST_F(steppersettings_tests, throws_exception_on_missing_values_6)
{
        JsonCpp json = JsonCpp::parse("{"
                                      "'steps-per-revolution': [1, 2, 3],"
                                      "'microsteps': [4, 5, 6],"
                                      "'gears-ratio': [7, 8, 9 ],"
                                      "'maximum-rpm': [10, 11, 12],"
                                      "'displacement-per-revolution': [13, 14, 15]"
                                      "}");
        try {
                StepperSettings settings(json);
                FAIL() << "Expected a runtime_error";
        } catch (std::runtime_error& e) { }
}
