#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "WheelOdometry.h"

using namespace std;
using namespace testing;
using namespace romi;

class wheelodometry_tests : public ::testing::Test
{
protected:
        JsonCpp config;
        double epsilon;
        
	wheelodometry_tests() {
                const char * config_string = "{"
                        "'wheel_diameter': 1.0,"
                        "'wheel_base': 1.0,"
                        "'encoder_steps': 1000.0,"
                        "'maximum_speed': 3.0 }";
                config = JsonCpp::parse(config_string);
                epsilon = 0.000001;
	}

	~wheelodometry_tests() override = default;

	void SetUp() override {
	}

	void TearDown() override {
	}
};

TEST_F(wheelodometry_tests, test_initialisation)
{
        NavigationSettings rover(config);

        WheelOdometry odometry(rover, 1000.0, 1000.0, 0.0);
        double x, y, orientation;
        
        //Assert
        odometry.get_location(x, y);
        orientation = odometry.get_orientation();
                
        ASSERT_NEAR(x, 0.0, epsilon);
        ASSERT_NEAR(y, 0.0, epsilon);
        ASSERT_NEAR(orientation, 0.0, epsilon);
}

TEST_F(wheelodometry_tests, test_update_encoders_displacement)
{
        NavigationSettings rover(config);

        WheelOdometry odometry(rover, 1000.0, 1000.0, 0.0);
        double x, y, vx, vy, orientation;

        // 1000 steps = 1 revolution = 3.14 meters
        odometry.set_encoders(2000.0, 2000.0, 1.0);
        
        //Assert
        odometry.get_location(x, y);
        odometry.get_speed(vx, vy);
        orientation = odometry.get_orientation();
                
        ASSERT_NEAR(x, M_PI, epsilon);
        ASSERT_NEAR(y, 0.0, epsilon);
        ASSERT_NEAR(vx, 0.2 * M_PI, epsilon);
        ASSERT_NEAR(vy, 0.0, epsilon);
        ASSERT_NEAR(orientation, 0.0, epsilon);
}

TEST_F(wheelodometry_tests, test_update_encoders_orientation_360degrees)
{
        NavigationSettings rover(config);

        WheelOdometry odometry(rover, 0.0, 0.0, 0.0);
        double x, y, orientation;

        // left: 1000 steps = 1 revolution = 3.14 meters
        // right: 1000 steps = 1 revolution = -3.14 meters
        // -> full rotation in the same spot (wheel base = 1m)
        odometry.set_encoders(1000.0, -1000.0, 1.0);
        
        //Assert
        odometry.get_location(x, y);
        orientation = odometry.get_orientation();
                
        ASSERT_NEAR(x, 0, epsilon);
        ASSERT_NEAR(y, 0.0, epsilon);
        ASSERT_NEAR(orientation, -2.0 * M_PI, epsilon);
}

TEST_F(wheelodometry_tests, test_update_encoders_orientation_90degrees)
{
        NavigationSettings rover(config);

        WheelOdometry odometry(rover, 0.0, 0.0, 0.0);
        double x, y, orientation;

        // 90° rotation
        odometry.set_encoders(-250.0, 250.0, 1.0);
        
        //Assert
        odometry.get_location(x, y);
        orientation = odometry.get_orientation();
                
        ASSERT_NEAR(x, 0, epsilon);
        ASSERT_NEAR(y, 0.0, epsilon);
        ASSERT_NEAR(orientation, M_PI / 2.0, epsilon);
}

TEST_F(wheelodometry_tests, test_update_encoders_move_and_turn_90degrees)
{
        NavigationSettings rover(config);

        WheelOdometry odometry(rover, 0.0, 0.0, 0.0);
        double x, y, orientation;

        odometry.set_encoders(1000.0, 1000.0, 1.0);
        odometry.set_encoders(1000.0 - 250.0, 1000.0 + 250.0, 1.0);
        
        //Assert
        odometry.get_location(x, y);
        orientation = odometry.get_orientation();
                
        ASSERT_NEAR(x, M_PI, epsilon);
        ASSERT_NEAR(y, 0.0, epsilon);
        ASSERT_NEAR(orientation, M_PI / 2.0, epsilon);
}

TEST_F(wheelodometry_tests, test_update_encoders_2x90degrees)
{
        NavigationSettings rover(config);

        WheelOdometry odometry(rover, 0.0, 0.0, 0.0);
        double x, y, orientation;

        double wheel_base = 1.0;
        double wheel_circumference = M_PI;

        // 90° rotation around the left wheel
        odometry.set_encoders(0.0,
                              M_PI * 1000.0 / 2.0 / wheel_circumference,
                              1.0);
        
        //Assert
        odometry.get_location(x, y);
        orientation = odometry.get_orientation();
        
        ASSERT_NEAR(x, wheel_base/2.0, epsilon);
        ASSERT_NEAR(y, wheel_base/2.0, epsilon);
        ASSERT_NEAR(orientation, M_PI/2.0, epsilon);
        
        // 90° rotation around the right wheel
        odometry.set_encoders(M_PI * 1000.0 / 2.0 / wheel_circumference,
                              M_PI * 1000.0 / 2.0 / wheel_circumference,
                              2.0);
        
        //Assert
        odometry.get_location(x, y);
        orientation = odometry.get_orientation();
        
        ASSERT_NEAR(x, wheel_base, epsilon);
        ASSERT_NEAR(y, wheel_base, epsilon);
        ASSERT_NEAR(orientation, 0.0, epsilon);
}
