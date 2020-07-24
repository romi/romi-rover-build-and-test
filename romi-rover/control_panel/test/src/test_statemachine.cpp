#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "StateMachine.h"

using namespace std;
using namespace testing;

class statemachine_tests : public ::testing::Test
{
protected:
	statemachine_tests() {
	}

	~statemachine_tests() override = default;

	void SetUp() override {
	}

	void TearDown() override {
	}
};

TEST_F(statemachine_tests, statemachine_initial_state_is_zero)
{
        // Arrange
        StateMachine statemachine;

        // Act
        //
        
        //Assert
        ASSERT_EQ(statemachine.getState(), 0);
}
