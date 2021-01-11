#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "SpeedConverter.h"
#include "JsonCpp.h"

using namespace std;
using namespace testing;
using namespace romi;

class speedconverter_tests : public ::testing::Test
{
protected:
	speedconverter_tests() {
	}

	~speedconverter_tests() override = default;

	void SetUp() override {
	}

	void TearDown() override {
	}
};

TEST_F(speedconverter_tests, constructor_successfully_parses_json)
{
        try {
                JsonCpp config = JsonCpp::parse(
                        "{"
                        "    \"use-speed-curve\": true,"
                        "    \"speed-curve-exponent\": 2.0,"
                        "    \"use-direction-curve\": false,"
                        "    \"direction-curve-exponent\": 3.0,"
                        "    \"speed-multiplier\": 1.0,"
                        "    \"direction-multiplier\": 0.4"
                        "}");
                
                SpeedConverter converter;
                converter.parse(config);

                ASSERT_EQ(converter.use_speed_curve, true);
                ASSERT_EQ(converter.speed_curve_exponent, 2.0);
                ASSERT_EQ(converter.use_direction_curve, false); 
                ASSERT_EQ(converter.direction_curve_exponent, 3.0);
                ASSERT_EQ(converter.speed_multiplier, 1.0);
                ASSERT_EQ(converter.direction_multiplier, 0.4);
               
        } catch (JSONError &e) {
                FAIL() << "Expected successfull parsing: " << e.what();
        } catch (...) {
                FAIL() << "Expected successfull parsing";
        }
}

TEST_F(speedconverter_tests, constructor_throws_error_on_bad_json)
{
        try {
                JsonCpp config = JsonCpp::parse("{}");
                SpeedConverter converter;
                converter.parse(config);
                FAIL() << "Expected JSONError";
                
        } catch (JSONError &e) {
                        // NOP
        } catch (...) {
                FAIL() << "Expected JSONError";
        }
}

TEST_F(speedconverter_tests, is_valid_returns_false_on_bad_settings_1)
{
        SpeedConverter converter;
        converter.use_speed_curve = true;
        converter.speed_curve_exponent = 0.0; // Bad
        converter.use_direction_curve = true;
        converter.direction_curve_exponent = 2.0;
        converter.speed_multiplier = 1.0;
        converter.direction_multiplier = 0.4;
                
        ASSERT_EQ(converter.is_valid(), false);
}

TEST_F(speedconverter_tests, is_valid_returns_false_on_bad_settings_2)
{
        SpeedConverter converter;
        converter.use_speed_curve = true;
        converter.speed_curve_exponent = 11.0; // Bad
        converter.use_direction_curve = true;
        converter.direction_curve_exponent = 2.0;
        converter.speed_multiplier = 1.0;
        converter.direction_multiplier = 0.4;
                
        ASSERT_EQ(converter.is_valid(), false);
}

TEST_F(speedconverter_tests, is_valid_returns_false_on_bad_settings_3)
{
        SpeedConverter converter;
        converter.use_speed_curve = true;
        converter.speed_curve_exponent = 1.0;
        converter.use_direction_curve = true;
        converter.direction_curve_exponent = 0.0; // Bad
        converter.speed_multiplier = 1.0;
        converter.direction_multiplier = 0.4;
                
        ASSERT_EQ(converter.is_valid(), false);
}

TEST_F(speedconverter_tests, is_valid_returns_false_on_bad_settings_4)
{
        SpeedConverter converter;
        converter.use_speed_curve = true;
        converter.speed_curve_exponent = 1.0;
        converter.use_direction_curve = true;
        converter.direction_curve_exponent = 11.0; // Bad
        converter.speed_multiplier = 1.0;
        converter.direction_multiplier = 0.4;
                
        ASSERT_EQ(converter.is_valid(), false);
}

TEST_F(speedconverter_tests, is_valid_returns_false_on_bad_settings_5)
{
        SpeedConverter converter;
        converter.use_speed_curve = true;
        converter.speed_curve_exponent = 1.0;
        converter.use_direction_curve = true;
        converter.direction_curve_exponent = 1.0; 
        converter.speed_multiplier = 0.0; // Bad
        converter.direction_multiplier = 0.4;
                
        ASSERT_EQ(converter.is_valid(), false);
}

TEST_F(speedconverter_tests, is_valid_returns_false_on_bad_settings_6)
{
        SpeedConverter converter;
        converter.use_speed_curve = true;
        converter.speed_curve_exponent = 1.0;
        converter.use_direction_curve = true;
        converter.direction_curve_exponent = 1.0; 
        converter.speed_multiplier = 2.0; // Bad
        converter.direction_multiplier = 0.4;
                
        ASSERT_EQ(converter.is_valid(), false);
}

TEST_F(speedconverter_tests, is_valid_returns_false_on_bad_settings_7)
{
        SpeedConverter converter;
        converter.use_speed_curve = true;
        converter.speed_curve_exponent = 1.0;
        converter.use_direction_curve = true;
        converter.direction_curve_exponent = 1.0; 
        converter.speed_multiplier = 1.0; 
        converter.direction_multiplier = 0.0; // Bad
                
        ASSERT_EQ(converter.is_valid(), false);
}

TEST_F(speedconverter_tests, is_valid_returns_false_on_bad_settings_8)
{
        SpeedConverter converter;
        converter.use_speed_curve = true;
        converter.speed_curve_exponent = 1.0;
        converter.use_direction_curve = true;
        converter.direction_curve_exponent = 1.0; 
        converter.speed_multiplier = 1.0; 
        converter.direction_multiplier = 2.0; // Bad
                
        ASSERT_EQ(converter.is_valid(), false);
}

TEST_F(speedconverter_tests, compute_speeds_generates_expected_speeds_1)
{
        SpeedConverter converter;
        converter.use_speed_curve = false;
        converter.speed_curve_exponent = 1.0;
        converter.use_direction_curve = false;
        converter.direction_curve_exponent = 1.0; 
        converter.speed_multiplier = 0.5; 
        converter.direction_multiplier = 1.0;

        WheelSpeeds speeds;

        speeds = converter.compute_speeds(0.0, 0.0);
        ASSERT_EQ(speeds.left, 0.0);
        ASSERT_EQ(speeds.right, 0.0);
        
        speeds = converter.compute_speeds(0.5, 0.0);
        ASSERT_EQ(speeds.left, 0.25);
        ASSERT_EQ(speeds.right, 0.25);
        
        speeds = converter.compute_speeds(1.0, 0.0);
        ASSERT_EQ(speeds.left, 0.5);
        ASSERT_EQ(speeds.right, 0.5);        
}

TEST_F(speedconverter_tests, compute_speeds_generates_expected_speeds_2)
{
        SpeedConverter converter;
        converter.use_speed_curve = false;
        converter.speed_curve_exponent = 1.0;
        converter.use_direction_curve = false;
        converter.direction_curve_exponent = 1.0; 
        converter.speed_multiplier = 1.0; 
        converter.direction_multiplier = 0.3;

        WheelSpeeds speeds;

        speeds = converter.compute_speeds(0.0, 0.0);
        ASSERT_EQ(speeds.left, 0.0);
        ASSERT_EQ(speeds.right, 0.0);
        
        speeds = converter.compute_speeds(0.0, 0.5);
        ASSERT_EQ(speeds.left, 0.15);
        ASSERT_EQ(speeds.right, -0.15);
        
        speeds = converter.compute_speeds(0.0, 1.0);
        ASSERT_EQ(speeds.left, 0.3);
        ASSERT_EQ(speeds.right, -0.3);        
}

TEST_F(speedconverter_tests, apply_speed_curve_generates_less_than_linear_values)
{
        SpeedConverter converter;
        converter.use_speed_curve = true;
        converter.speed_curve_exponent = 1.0;
        converter.use_direction_curve = false;
        converter.direction_curve_exponent = 1.0; 
        converter.speed_multiplier = 1.0; 
        converter.direction_multiplier = 0.3;
        
        ASSERT_EQ(converter.apply_speed_curve(0.0), 0.0);
        ASSERT_EQ(converter.apply_speed_curve(1.0), 1.0);
        
        for (int i = 1; i < 10; i++) {
                double value = i * 0.1;
                ASSERT_LT(converter.apply_speed_curve(value), value);
        }
}

TEST_F(speedconverter_tests, apply_direction_curve_generates_less_than_linear_values)
{
        SpeedConverter converter;
        converter.use_speed_curve = true;
        converter.speed_curve_exponent = 1.0;
        converter.use_direction_curve = true;
        converter.direction_curve_exponent = 1.0; 
        converter.speed_multiplier = 1.0; 
        converter.direction_multiplier = 0.3;
        
        ASSERT_EQ(converter.apply_direction_curve(0.0), 0.0);
        ASSERT_EQ(converter.apply_direction_curve(1.0), 1.0);
        
        for (int i = 1; i < 10; i++) {
                double value = i * 0.1;
                ASSERT_LT(converter.apply_direction_curve(value), value);
        }
}
