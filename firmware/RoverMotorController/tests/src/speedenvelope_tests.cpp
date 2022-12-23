#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "SpeedEnvelope.h"

using namespace std;
using namespace testing;

class speedenvelope_tests : public ::testing::Test
{
protected:
        
	speedenvelope_tests() {
        }

	~speedenvelope_tests() override = default;

	void SetUp() override {
        }

	void TearDown() override {}
};

TEST_F(speedenvelope_tests, test_constructor)
{
        // Arrange

        // Act
        SpeedEnvelope envelope;

        // Assert
        ASSERT_EQ(envelope.target_, 0);
        ASSERT_EQ(envelope.current_, 0);
        ASSERT_EQ(envelope.increment_, 0);
}

TEST_F(speedenvelope_tests, test_init_set_correct_increment)
{
        // Arrange
        SpeedEnvelope envelope;

        // Act
        envelope.init(1.0, 0.3, 0.020);

        // Assert
        ASSERT_EQ(envelope.increment_, 6);
}

TEST_F(speedenvelope_tests, test_set_target)
{
        // Arrange
        SpeedEnvelope envelope;
        envelope.init(1.0, 0.3, 0.020);

        // Act
        envelope.set_target(1000);

        // Assert
        ASSERT_EQ(envelope.target_, 1000);
}

TEST_F(speedenvelope_tests, test_update_once_increments_speed_once)
{
        // Arrange
        SpeedEnvelope envelope;
        envelope.init(1.0, 0.3, 0.020);
        envelope.set_target(1000);

        // Act
        int16_t speed = envelope.update();

        // Assert
        ASSERT_EQ(speed, 6);
}

TEST_F(speedenvelope_tests, test_multiple_update_increments_speed)
{
        // Arrange
        SpeedEnvelope envelope;
        envelope.init(1.0, 0.3, 0.020);
        envelope.set_target(1000);

        // Act
        int n = 3;
        int16_t speed;
        for (int i = 0; i < n; i++)
                speed = envelope.update();

        // Assert
        ASSERT_EQ(speed, n * 6);
}

TEST_F(speedenvelope_tests, test_multiple_update_increments_speed_negative_target)
{
        // Arrange
        SpeedEnvelope envelope;
        envelope.init(1.0, 0.3, 0.020);
        envelope.set_target(-1000);

        // Act
        int n = 3;
        int16_t speed;
        for (int i = 0; i < n; i++)
                speed = envelope.update();

        // Assert
        ASSERT_EQ(speed, n * -6);
}

TEST_F(speedenvelope_tests, test_handles_changing_target_speeds)
{
        // Arrange
        SpeedEnvelope envelope;
        envelope.init(1.0, 0.3, 0.020);

        // Act
        int n = 3;
        int16_t speed;
        
        envelope.set_target(1000);
        for (int i = 0; i < n; i++)
                speed = envelope.update();
        
        envelope.set_target(0);
        for (int i = 0; i < n; i++)
                speed = envelope.update();

        // Assert
        ASSERT_EQ(speed, 0);
}

TEST_F(speedenvelope_tests, test_handles_target_speed_limits)
{
        // Arrange
        SpeedEnvelope envelope;
        envelope.init(1.0, 0.3, 0.020);

        // Act
        int n = 3;
        int16_t speed;
        
        envelope.set_target(1);
        for (int i = 0; i < n; i++)
                speed = envelope.update();

        // Assert
        ASSERT_EQ(speed, 1);
}

TEST_F(speedenvelope_tests, test_handles_target_speed_limits_for_negative_values)
{
        // Arrange
        SpeedEnvelope envelope;
        envelope.init(1.0, 0.3, 0.020);

        // Act
        int n = 3;
        int16_t speed;
        
        envelope.set_target(-1);
        for (int i = 0; i < n; i++)
                speed = envelope.update();

        // Assert
        ASSERT_EQ(speed, -1);
}
