#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "Navigation.h"
#include "../mock/mock_motordriver.h"

using namespace std;
using namespace testing;
using namespace romi;

class navigation_tests : public ::testing::Test
{
protected:
        JSON config;
        MockMotorDriver driver;
        
	navigation_tests() {
                const char * config_string = "{"
                        "'wheel_diameter': 1.0,"
                        "'wheel_base': 1.0,"
                        "'encoder_steps': 1000.0,"
                        "'maximum_speed': 3.0 }";
                config = JSON::parse(config_string);
	}

	~navigation_tests() override = default;

	void SetUp() override {
	}

	void TearDown() override {
	}
};

TEST_F(navigation_tests, test_moveat)
{
        RoverConfiguration rover(config);
        Navigation navigation(driver, rover);

        EXPECT_CALL(driver, moveat(0.1, 0.2))
                .WillOnce(Return(true));

        navigation.moveat(0.1, 0.2);
}

TEST_F(navigation_tests, move_fails_with_invalid_speed_1)
{
        RoverConfiguration rover(config);
        Navigation navigation(driver, rover);

        EXPECT_CALL(driver, stop())
                .WillOnce(Return(true));

        bool success = navigation.move(1.0, 2.0);
        ASSERT_EQ(success, false);
}

TEST_F(navigation_tests, move_fails_with_invalid_speed_2)
{
        RoverConfiguration rover(config);
        Navigation navigation(driver, rover);

        EXPECT_CALL(driver, stop())
                .WillOnce(Return(true));

        bool success = navigation.move(1.0, -2.0);
        ASSERT_EQ(success, false);
}

TEST_F(navigation_tests, move_fails_with_invalid_distance_1)
{
        RoverConfiguration rover(config);
        Navigation navigation(driver, rover);

        EXPECT_CALL(driver, stop())
                .WillOnce(Return(true));

        bool success = navigation.move(51.0, 1.0);
        ASSERT_EQ(success, false);
}

TEST_F(navigation_tests, move_fails_with_invalid_distance_2)
{
        RoverConfiguration rover(config);
        Navigation navigation(driver, rover);

        EXPECT_CALL(driver, stop())
                .WillOnce(Return(true));

        bool success = navigation.move(-51.0, 1.0);
        ASSERT_EQ(success, false);
}

TEST_F(navigation_tests, successful_move)
{
        RoverConfiguration rover(config);
        Navigation navigation(driver, rover);

        {
                InSequence seq;
                
                EXPECT_CALL(driver, stop())
                        .WillOnce(Return(true))
                        .RetiresOnSaturation();
                
                EXPECT_CALL(driver, get_encoder_values(_,_,_))
                        .WillOnce(DoAll(SetArgReferee<0>(0.0),
                                        SetArgReferee<1>(0.0),
                                        SetArgReferee<2>(0.0),
                                        Return(true)))
                        .RetiresOnSaturation();
                
                EXPECT_CALL(driver, moveat(1.0, 1.0))
                        .WillOnce(Return(true))
                        .RetiresOnSaturation();
                
                EXPECT_CALL(driver, get_encoder_values(_,_,_))
                        .WillOnce(DoAll(SetArgReferee<0>(3.0 * 1000.0 / M_PI),
                                        SetArgReferee<1>(3.0 * 1000.0 / M_PI),
                                        SetArgReferee<2>(1.0),
                                        Return(true)))
                        .RetiresOnSaturation();

                EXPECT_CALL(driver, moveat(0.0, 0.0))
                        .WillOnce(Return(true))
                        .RetiresOnSaturation();

                EXPECT_CALL(driver, stop())
                        .WillOnce(Return(true))
                        .RetiresOnSaturation();

        }

        // Move 3 meter, at the maximum speed of 3 m/s.
        bool success = navigation.move(3.0, 1.0);
        ASSERT_EQ(success, true);
}

TEST_F(navigation_tests, successfully_move_distance_with_negative_speed)
{
        RoverConfiguration rover(config);
        Navigation navigation(driver, rover);

        {
                InSequence seq;
                
                EXPECT_CALL(driver, stop())
                        .WillOnce(Return(true))
                        .RetiresOnSaturation();
                
                EXPECT_CALL(driver, get_encoder_values(_,_,_))
                        .WillOnce(DoAll(SetArgReferee<0>(0.0),
                                        SetArgReferee<1>(0.0),
                                        SetArgReferee<2>(0.0),
                                        Return(true)))
                        .RetiresOnSaturation();
                
                EXPECT_CALL(driver, moveat(-1.0, -1.0))
                        .WillOnce(Return(true))
                        .RetiresOnSaturation();
                
                EXPECT_CALL(driver, get_encoder_values(_,_,_))
                        .WillOnce(DoAll(SetArgReferee<0>(-3.0 * 1000.0 / M_PI),
                                        SetArgReferee<1>(-3.0 * 1000.0 / M_PI),
                                        SetArgReferee<2>(1.0),
                                        Return(true)))
                        .RetiresOnSaturation();

                EXPECT_CALL(driver, moveat(0.0, 0.0))
                        .WillOnce(Return(true))
                        .RetiresOnSaturation();

                EXPECT_CALL(driver, stop())
                        .WillOnce(Return(true))
                        .RetiresOnSaturation();

        }

        // Move 3 meter, at the maximum speed of 3 m/s.
        bool success = navigation.move(3.0, -1.0);
        ASSERT_EQ(success, true);
}

TEST_F(navigation_tests, successfully_move_negative_distance_with_positive_speed)
{
        RoverConfiguration rover(config);
        Navigation navigation(driver, rover);

        {
                InSequence seq;
                
                EXPECT_CALL(driver, stop())
                        .WillOnce(Return(true))
                        .RetiresOnSaturation();
                
                EXPECT_CALL(driver, get_encoder_values(_,_,_))
                        .WillOnce(DoAll(SetArgReferee<0>(0.0),
                                        SetArgReferee<1>(0.0),
                                        SetArgReferee<2>(0.0),
                                        Return(true)))
                        .RetiresOnSaturation();
                
                EXPECT_CALL(driver, moveat(-1.0, -1.0))
                        .WillOnce(Return(true))
                        .RetiresOnSaturation();
                
                EXPECT_CALL(driver, get_encoder_values(_,_,_))
                        .WillOnce(DoAll(SetArgReferee<0>(-3.0 * 1000.0 / M_PI),
                                        SetArgReferee<1>(-3.0 * 1000.0 / M_PI),
                                        SetArgReferee<2>(1.0),
                                        Return(true)))
                        .RetiresOnSaturation();

                EXPECT_CALL(driver, moveat(0.0, 0.0))
                        .WillOnce(Return(true))
                        .RetiresOnSaturation();

                EXPECT_CALL(driver, stop())
                        .WillOnce(Return(true))
                        .RetiresOnSaturation();

        }

        // Move 3 meter, at the maximum speed of 3 m/s.
        bool success = navigation.move(-3.0, 1.0);
        ASSERT_EQ(success, true);
}

TEST_F(navigation_tests, successfully_move_negative_distance_with_negative_speed)
{
        RoverConfiguration rover(config);
        Navigation navigation(driver, rover);

        {
                InSequence seq;
                
                EXPECT_CALL(driver, stop())
                        .WillOnce(Return(true))
                        .RetiresOnSaturation();
                
                EXPECT_CALL(driver, get_encoder_values(_,_,_))
                        .WillOnce(DoAll(SetArgReferee<0>(0.0),
                                        SetArgReferee<1>(0.0),
                                        SetArgReferee<2>(0.0),
                                        Return(true)))
                        .RetiresOnSaturation();
                
                EXPECT_CALL(driver, moveat(-1.0, -1.0))
                        .WillOnce(Return(true))
                        .RetiresOnSaturation();
                
                EXPECT_CALL(driver, get_encoder_values(_,_,_))
                        .WillOnce(DoAll(SetArgReferee<0>(-3.0 * 1000.0 / M_PI),
                                        SetArgReferee<1>(-3.0 * 1000.0 / M_PI),
                                        SetArgReferee<2>(1.0),
                                        Return(true)))
                        .RetiresOnSaturation();

                EXPECT_CALL(driver, moveat(0.0, 0.0))
                        .WillOnce(Return(true))
                        .RetiresOnSaturation();

                EXPECT_CALL(driver, stop())
                        .WillOnce(Return(true))
                        .RetiresOnSaturation();

        }

        // Move 3 meter, at the maximum speed of 3 m/s.
        bool success = navigation.move(-3.0, -1.0);
        ASSERT_EQ(success, true);
}

TEST_F(navigation_tests, move_fails_on_zero_speed)
{
        RoverConfiguration rover(config);
        Navigation navigation(driver, rover);
        
        EXPECT_CALL(driver, stop())
                .WillOnce(Return(true))
                .RetiresOnSaturation();
        
        bool success = navigation.move(3.0, 0.0);
        ASSERT_EQ(success, false);
}

TEST_F(navigation_tests, move_returns_true_on_zero_distance)
{
        RoverConfiguration rover(config);
        Navigation navigation(driver, rover);
        
        EXPECT_CALL(driver, stop())
                .WillOnce(Return(true))
                .RetiresOnSaturation();
        
        bool success = navigation.move(0.0, 1.0);
        ASSERT_EQ(success, true);
}

TEST_F(navigation_tests, move_fails_on_bad_speed_1)
{
        RoverConfiguration rover(config);
        Navigation navigation(driver, rover);
        
        EXPECT_CALL(driver, stop())
                .WillOnce(Return(true))
                .RetiresOnSaturation();
        
        bool success = navigation.move(3.0, 2.0);
        ASSERT_EQ(success, false);
}

TEST_F(navigation_tests, move_fails_on_bad_speed_2)
{
        RoverConfiguration rover(config);
        Navigation navigation(driver, rover);
        
        EXPECT_CALL(driver, stop())
                .WillOnce(Return(true))
                .RetiresOnSaturation();
        
        bool success = navigation.move(3.0, -1.1);
        ASSERT_EQ(success, false);
}

TEST_F(navigation_tests, move_fails_on_bad_speed_3)
{
        RoverConfiguration rover(config);
        Navigation navigation(driver, rover);
        
        EXPECT_CALL(driver, stop())
                .WillOnce(Return(true))
                .RetiresOnSaturation();
        
        bool success = navigation.move(3.0, 0);
        ASSERT_EQ(success, false);
}

TEST_F(navigation_tests, move_return_false_on_failing_stop)
{
        RoverConfiguration rover(config);
        Navigation navigation(driver, rover);
        
        EXPECT_CALL(driver, stop())
                .WillOnce(Return(false))
                .RetiresOnSaturation();
        
        EXPECT_CALL(driver, stop())
                .WillOnce(Return(false))
                .RetiresOnSaturation();
        
        bool success = navigation.move(3.0, 0.5);
        ASSERT_EQ(success, false);
}

TEST_F(navigation_tests, move_return_false_on_failing_get_encoders)
{
        RoverConfiguration rover(config);
        Navigation navigation(driver, rover);
        
        EXPECT_CALL(driver, stop())
                .WillOnce(Return(true))
                .RetiresOnSaturation();
                
        EXPECT_CALL(driver, get_encoder_values(_,_,_))
                .WillOnce(Return(false))
                .RetiresOnSaturation();
        
        EXPECT_CALL(driver, stop())
                .WillOnce(Return(true))
                .RetiresOnSaturation();
        
        bool success = navigation.move(3.0, 0.5);
        ASSERT_EQ(success, false);
}
