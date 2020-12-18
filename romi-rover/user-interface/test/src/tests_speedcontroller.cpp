#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../mock/mock_navigation.h"
#include "SpeedController.h"
#include "JSONConfiguration.h"

using namespace std;
using namespace testing;
using namespace romi;

class speedcontroller_tests : public ::testing::Test
{
protected:
        MockNavigation navigation;
        SpeedControllerSettings fast;
        SpeedControllerSettings accurate;
         
	speedcontroller_tests() {
	}

	~speedcontroller_tests() override = default;

	void SetUp() override {
                // default values
                fast.use_speed_curve = true;
                fast.speed_curve_exponent = 2.0;
                fast.use_direction_curve = true;
                fast.direction_curve_exponent = 2.0;
                fast.speed_multiplier = 1.0;
                fast.direction_multiplier = 0.4;
                
                accurate.use_speed_curve = true;
                accurate.speed_curve_exponent = 1.0;
                accurate.use_direction_curve = true;
                accurate.direction_curve_exponent = 1.0;
                accurate.speed_multiplier = 1.0;
                accurate.direction_multiplier = 0.4;
	}

	void TearDown() override {
	}
};

TEST_F(speedcontroller_tests, constructor_throws_error_on_bad_json)
{

        try {
                JsonCpp config = JsonCpp::parse("{}");
                SpeedController controller(navigation, config);
                FAIL() << "Expected JSONError";
                
        } catch (JSONError &e) {
                        // NOP
        } catch (...) {
                FAIL() << "Expected JSONError";
        }
}

TEST_F(speedcontroller_tests, constructor_throws_range_error_on_bad_settings_1)
{
        fast.speed_curve_exponent = 0.0;

        try {
                SpeedController controller(navigation, fast, accurate);
                FAIL() << "Expected std::runtime_error";
        } catch (std::range_error const &e) {
                        // NOP
        } catch (...) {
                FAIL() << "Expected std::runtime_error";
        }
}

TEST_F(speedcontroller_tests, constructor_throws_range_error_on_bad_settings_2)
{
        fast.direction_curve_exponent = 0.0;

        try {
                SpeedController controller(navigation, fast, accurate);
                FAIL() << "Expected std::runtime_error";
        } catch (std::range_error const &e) {
                        // NOP
        } catch (...) {
                FAIL() << "Expected std::runtime_error";
        }
}

TEST_F(speedcontroller_tests, constructor_throws_range_error_on_bad_settings_3)
{
        fast.speed_curve_exponent = 11.0;

        try {
                SpeedController controller(navigation, fast, accurate);
                FAIL() << "Expected std::runtime_error";
        } catch (std::range_error const &e) {
                        // NOP
        } catch (...) {
                FAIL() << "Expected std::runtime_error";
        }
}

TEST_F(speedcontroller_tests, constructor_throws_range_error_on_bad_settings_4)
{
        fast.direction_curve_exponent = 11.0;

        try {
                SpeedController controller(navigation, fast, accurate);
                FAIL() << "Expected std::runtime_error";
        } catch (std::range_error const &e) {
                        // NOP
        } catch (...) {
                FAIL() << "Expected std::runtime_error";
        }
}

TEST_F(speedcontroller_tests, constructor_throws_range_error_on_bad_settings_5)
{
        fast.speed_multiplier = 0.0;

        try {
                SpeedController controller(navigation, fast, accurate);
                FAIL() << "Expected std::runtime_error";
        } catch (std::range_error const &e) {
                        // NOP
        } catch (...) {
                FAIL() << "Expected std::runtime_error";
        }
}

TEST_F(speedcontroller_tests, constructor_throws_range_error_on_bad_settings_6)
{
        fast.speed_multiplier = 2.0;

        try {
                SpeedController controller(navigation, fast, accurate);
                FAIL() << "Expected std::runtime_error";
        } catch (std::range_error const &e) {
                        // NOP
        } catch (...) {
                FAIL() << "Expected std::runtime_error";
        }
}

TEST_F(speedcontroller_tests, constructor_throws_range_error_on_bad_settings_7)
{
        fast.direction_multiplier = 0.0;

        try {
                SpeedController controller(navigation, fast, accurate);
                FAIL() << "Expected std::runtime_error";
        } catch (std::range_error const &e) {
                        // NOP
        } catch (...) {
                FAIL() << "Expected std::runtime_error";
        }
}

TEST_F(speedcontroller_tests, constructor_throws_range_error_on_bad_settings_8)
{
        fast.direction_multiplier = 2.0;

        try {
                SpeedController controller(navigation, fast, accurate);
                FAIL() << "Expected std::runtime_error";
        } catch (std::range_error const &e) {
                        // NOP
        } catch (...) {
                FAIL() << "Expected std::runtime_error";
        }
}

TEST_F(speedcontroller_tests, stop_calls_navigation_stop)
{
        SpeedController controller(navigation, fast, accurate);
        
        EXPECT_CALL(navigation, stop())
                .WillOnce(Return(true));
        
        bool success = controller.stop();

        ASSERT_EQ(success, true);
}

TEST_F(speedcontroller_tests, stop_returns_false_when_navigation_stop_fails)
{
        SpeedController controller(navigation, fast, accurate);
        
        EXPECT_CALL(navigation, stop())
                .WillOnce(Return(false));
        
        bool success = controller.stop();

        ASSERT_EQ(success, false);
}

TEST_F(speedcontroller_tests, moveat_generates_expected_speeds_1)
{
        fast.use_speed_curve = false;
        fast.use_direction_curve = false;
        fast.speed_multiplier = 1.0;
        fast.direction_multiplier = 1.0;
        
        SpeedController controller(navigation, fast, accurate);
        
        // Speed 1 and direction 0: straight forward: left wheel (1)
        // and right wheel (1).
        EXPECT_CALL(navigation, moveat(1.0, 1.0))
                .WillOnce(Return(true));
        
        bool success = controller.drive_at(1.0, 0.0);

        ASSERT_EQ(success, true);
}

TEST_F(speedcontroller_tests, moveat_generates_expected_speeds_2)
{
        fast.use_speed_curve = false;
        fast.use_direction_curve = false;
        fast.speed_multiplier = 1.0;
        fast.direction_multiplier = 1.0;
        
        SpeedController controller(navigation, fast, accurate);
        
        // Speed 1 and direction 1: max turning right: left wheel (1)
        // and right wheel (0).
        EXPECT_CALL(navigation, moveat(1.0, 0.0))
                .WillOnce(Return(true));
        
        bool success = controller.drive_at(1.0, 1.0);

        ASSERT_EQ(success, true);
}

TEST_F(speedcontroller_tests, moveat_generates_expected_speeds_3)
{
        fast.use_speed_curve = false;
        fast.use_direction_curve = false;
        fast.speed_multiplier = 1.0;
        fast.direction_multiplier = 1.0;
        
        SpeedController controller(navigation, fast, accurate);

        // Speed 1 and direction -1: max turning left: left wheel
        // stops (0) and right wheel full speed (1.0).
        
        EXPECT_CALL(navigation, moveat(0.0, 1.0))
                .WillOnce(Return(true));
        
        bool success = controller.drive_at(1.0, -1.0);

        ASSERT_EQ(success, true);
}

TEST_F(speedcontroller_tests, moveat_generates_expected_speeds_4)
{
        fast.use_speed_curve = false;
        fast.use_direction_curve = false;
        fast.speed_multiplier = 1.0;
        fast.direction_multiplier = 1.0;
        
        SpeedController controller(navigation, fast, accurate);
        
        // Speed 0 and direction -1: spin: left wheel (-1) and right
        // wheel (1).
        EXPECT_CALL(navigation, moveat(-1.0, 1.0))
                .WillOnce(Return(true));

        bool success = controller.drive_at(0.0, -1.0);

        ASSERT_EQ(success, true);
}

TEST_F(speedcontroller_tests, moveat_generates_linear_speeds_without_curve)
{
        fast.use_speed_curve = false;
        fast.use_direction_curve = false;
        fast.speed_multiplier = 1.0;
        fast.direction_multiplier = 1.0;
        
        SpeedController controller(navigation, fast, accurate);

        InSequence seq;
        for (int i = 0; i <= 10; i++) {
                double speed = i * 0.1;
                EXPECT_CALL(navigation, moveat(speed, speed))
                        .WillOnce(Return(true))
                        .RetiresOnSaturation();
        }
        
        for (int i = 0; i <= 10; i++) {
                double speed = i * 0.1;
                controller.drive_at(speed, 0.0);
        }
}

TEST_F(speedcontroller_tests, moveat_generates_less_than_linear_speeds_with_curve)
{
        fast.use_speed_curve = true;
        fast.speed_curve_exponent = 2.0;
        fast.speed_multiplier = 1.0;
        
        SpeedController controller(navigation, fast, accurate);

        InSequence seq;
        EXPECT_CALL(navigation, moveat(0.0, 0.0))
                .WillOnce(Return(true))
                .RetiresOnSaturation();
        for (int i = 1; i < 10; i++) {
                double speed = i * 0.1;
                EXPECT_CALL(navigation, moveat(Lt(speed), Lt(speed)))
                        .WillOnce(Return(true))
                        .RetiresOnSaturation();
        }
        EXPECT_CALL(navigation, moveat(1.0, 1.0))
                .WillOnce(Return(true))
                .RetiresOnSaturation();
        
        for (int i = 0; i <= 10; i++) {
                double speed = i * 0.1;
                controller.drive_at(speed, 0.0);
        }
}

TEST_F(speedcontroller_tests, moveat_generates_linear_directions_without_curve)
{
        fast.use_speed_curve = false;
        fast.use_direction_curve = false;
        fast.speed_multiplier = 1.0;
        fast.direction_multiplier = 1.0;
        
        SpeedController controller(navigation, fast, accurate);

        InSequence seq;
        for (int i = 0; i <= 10; i++) {
                double speed = i * 0.1;
                EXPECT_CALL(navigation, moveat(speed, -speed))
                        .WillOnce(Return(true))
                        .RetiresOnSaturation();
        }
        
        for (int i = 0; i <= 10; i++) {
                double direction = i * 0.1;
                controller.drive_at(0.0, direction);
        }
}

TEST_F(speedcontroller_tests, moveat_generates_less_than_linear_directions_with_curve)
{
        fast.use_speed_curve = false;
        fast.use_direction_curve = true;
        fast.direction_curve_exponent = 2.0;
        fast.direction_multiplier = 1.0;
        
        SpeedController controller(navigation, fast, accurate);

        InSequence seq;
        EXPECT_CALL(navigation, moveat(0.0, 0.0))
                .WillOnce(Return(true))
                .RetiresOnSaturation();
        for (int i = 1; i < 10; i++) {
                double speed = i * 0.1;
                EXPECT_CALL(navigation, moveat(Lt(speed), Gt(-speed)))
                        .WillOnce(Return(true))
                        .RetiresOnSaturation();
        }
        EXPECT_CALL(navigation, moveat(1.0, -1.0))
                .WillOnce(Return(true))
                .RetiresOnSaturation();
        
        for (int i = 0; i <= 10; i++) {
                double direction = i * 0.1;
                controller.drive_at(0.0, direction);
        }
}

TEST_F(speedcontroller_tests, spin_generates_inverse_speeds)
{
        accurate.use_speed_curve = false;
        accurate.use_direction_curve = false;
        accurate.speed_multiplier = 1.0;
        accurate.direction_multiplier = 1.0;
        
        SpeedController controller(navigation, fast, accurate);
        
        EXPECT_CALL(navigation, moveat(1.0, -1.0))
                .WillOnce(Return(true));
        
        bool success = controller.spin(1.0);

        ASSERT_EQ(success, true);
}
