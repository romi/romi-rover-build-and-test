#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "Script.h"

using namespace std;
using namespace testing;

class script_tests : public ::testing::Test
{
protected:
	script_tests() {
	}

	~script_tests() override = default;

	void SetUp() override {
	}

	void TearDown() override {
	}
};

TEST_F(script_tests, add_moveto_action)
{
        // Arrange
        oquam::Script script;

        // Act
        script.moveto(1.0, 2.0, 3.0, 4.0, 5);
        
        //Assert
        ASSERT_EQ(script.length(), 1);

        oquam::V3 p(1.0, 2.0, 3.0);
        oquam::Action action = script.get(0);
        ASSERT_EQ(action.type, oquam::Action::MOVETO);
        ASSERT_EQ(action.id, 5);
        ASSERT_EQ(action.destination == p, true);
        ASSERT_EQ(action.speed, 4.0);
}

TEST_F(script_tests, add_delay_action)
{
        // Arrange
        oquam::Script script;

        // Act
        script.delay(1.0, 2);
        
        //Assert
        ASSERT_EQ(script.length(), 1);

        oquam::Action action = script.get(0);
        ASSERT_EQ(action.type, oquam::Action::DELAY);
        ASSERT_EQ(action.id, 2);
        ASSERT_EQ(action.delay, 1.0);
}

TEST_F(script_tests, add_trigger_action)
{
        // Arrange
        oquam::Script script;
        void *p = (void *) 0x01;

        MockFunction<void(void*, int16_t)> mockCallback;
        EXPECT_CALL(mockCallback, Call((void*)0x01, 2))
                .Times(1);
        
        // Act
        script.trigger(mockCallback.AsStdFunction(), p, 2, 3.0, 4);
        oquam::Action action = script.get(0);
        action.invoke();
        
        //Assert
        ASSERT_EQ(script.length(), 1);
        ASSERT_EQ(action.type, oquam::Action::TRIGGER);
        ASSERT_EQ(action.id, 4);
        ASSERT_EQ(action.delay, 3.0);
}

TEST_F(script_tests, add_two_moveto_actions)
{
        // Arrange
        oquam::Script script;
        
        // Act
        script.moveto(1.0, 2.0, 3.0, 4.0, 5);
        script.moveto(6.0, 7.0, 8.0, 9.0, 10);
        
        //Assert
        ASSERT_EQ(script.length(), 2);

        oquam::V3 p1(1.0, 2.0, 3.0);
        oquam::Action action = script.get(0);
        ASSERT_EQ(action.type, oquam::Action::MOVETO);
        ASSERT_EQ(action.id, 5);
        ASSERT_EQ(action.destination == p1, true);
        ASSERT_EQ(action.speed, 4.0);

        oquam::V3 p2(6.0, 7.0, 8.0);
        action = script.get(1);
        ASSERT_EQ(action.type, oquam::Action::MOVETO);
        ASSERT_EQ(action.id, 10);
        ASSERT_EQ(action.destination == p2, true);
        ASSERT_EQ(action.speed, 9.0);
}
