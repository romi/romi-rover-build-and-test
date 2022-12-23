#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "PIController.h"
#include "../mocks/mock_encoder.h"
#include "../mocks/mock_pwm.h"

using namespace std;
using namespace testing;

class picontroller_tests : public ::testing::Test
{
protected:
        MockEncoder encoder_;
        MockPWM pwm_;
        
	picontroller_tests() : encoder_(), pwm_() {
        }

	~picontroller_tests() override = default;

	void SetUp() override {
        }

	void TearDown() override {}
};

TEST_F(picontroller_tests, test_constructor)
{
        // Arrange
        // pass
        
        // Act
        PIController controller(encoder_, pwm_);

        // Assert
        ASSERT_EQ(controller.last_position_, 0);
        ASSERT_EQ(controller.delta_, 0);
        ASSERT_EQ(controller.delta_target_, 0);
        ASSERT_EQ(controller.error_, 0);
        ASSERT_EQ(controller.sum_, 0);
        ASSERT_EQ(controller.p_, 0);
        ASSERT_EQ(controller.i_, 0);
        ASSERT_EQ(controller.out_, 0);
        ASSERT_EQ(controller.pulsewidth_, 0);
        // expected: speed (1 rev/s) x dt (0.020 s) x ppr (pulse/rev)
        //ASSERT_EQ(controller.speed_to_delta_, 1.0 * 0.020 * 10000.0);
}

TEST_F(picontroller_tests, test_update_encoder_values)
{
        // Arrange
        EXPECT_CALL(encoder_, get_position())
                .WillOnce(Return(0));
        EXPECT_CALL(encoder_, positions_per_revolution())
                .WillOnce(Return(10000));
        PIController controller(encoder_, pwm_);

        // Act
        controller.update_encoder_values(0.020);

        // Assert
        // expected: speed (1 rev/s) x dt (0.020 s) x ppr (pulse/rev)
        ASSERT_EQ(controller.speed_to_delta_, 1.0 * 0.020 * 10000.0);
}

TEST_F(picontroller_tests, test_update_1)
{
        // Arrange
        EXPECT_CALL(encoder_, get_position())
                .WillOnce(Return(0))
                .WillOnce(Return(0));
        EXPECT_CALL(encoder_, positions_per_revolution())
                .WillOnce(Return(10000));
        EXPECT_CALL(pwm_, amplitude())
                .WillRepeatedly(Return(1000));
        EXPECT_CALL(pwm_, center())
                .WillRepeatedly(Return(1000));
        EXPECT_CALL(pwm_, set(1040));

        PIController controller(encoder_, pwm_);
        controller.update_encoder_values(0.020);
        controller.init(1, 1, 1, 1, 0);
        
        // Act
        controller.update(100);
        
        // Assert
        ASSERT_EQ(controller.last_position_, 0);
        ASSERT_EQ(controller.delta_, 0);
        ASSERT_EQ(controller.delta_target_, 20);
        ASSERT_EQ(controller.error_, 20);
        ASSERT_EQ(controller.sum_, 20);
        ASSERT_EQ(controller.p_, 20);
        ASSERT_EQ(controller.i_, 20);
        ASSERT_EQ(controller.out_, 40);
        ASSERT_EQ(controller.pulsewidth_, 1040);
}

TEST_F(picontroller_tests, test_update_2)
{
        // Arrange
        EXPECT_CALL(encoder_, get_position())
                .WillOnce(Return(0))
                .WillOnce(Return(0))
                .WillOnce(Return(20));
        EXPECT_CALL(encoder_, positions_per_revolution())
                .WillOnce(Return(10000));
        EXPECT_CALL(pwm_, amplitude())
                .WillRepeatedly(Return(1000));
        EXPECT_CALL(pwm_, center())
                .WillRepeatedly(Return(1000));
        
        EXPECT_CALL(pwm_, set(1040));
        EXPECT_CALL(pwm_, set(1020));

        PIController controller(encoder_, pwm_);
        controller.update_encoder_values(0.020);
        controller.init(1, 1, 1, 1, 0);
        
        // Act
        controller.update(100);
        controller.update(100);
        
        // Assert
        ASSERT_EQ(controller.last_position_, 20);
        ASSERT_EQ(controller.delta_, 20);
        ASSERT_EQ(controller.delta_target_, 20);
        ASSERT_EQ(controller.error_, 0);
        ASSERT_EQ(controller.sum_, 20);
        ASSERT_EQ(controller.p_, 0);
        ASSERT_EQ(controller.i_, 20);
        ASSERT_EQ(controller.out_, 20);
        ASSERT_EQ(controller.pulsewidth_, 1020);
}
