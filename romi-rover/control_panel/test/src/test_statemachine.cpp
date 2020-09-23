#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "StateMachine.h"
#include "../mock/mock_statetransition.h"

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

        void initMockStateTransition(MockStateTransition &transition,
                                     int8_t from, int16_t evt, int8_t to) {                
                EXPECT_CALL(transition, state)
                        .WillRepeatedly(Return(from));

                EXPECT_CALL(transition, event)
                        .WillRepeatedly(Return(evt));

                EXPECT_CALL(transition, nextState)
                        .WillRepeatedly(Return(to));
	}

};

TEST_F(statemachine_tests, statemachine_initial_state_is_start)
{
        // Arrange
        StateMachine statemachine;

        // Act
        //
        
        //Assert
        ASSERT_EQ(statemachine.getState(), STATE_START);
}

TEST_F(statemachine_tests, statemachine_returns_ignore)
{
        // Arrange
        StateMachine statemachine;

        // Act
        int r = statemachine.handleEvent(1, 0);
        
        //Assert
        ASSERT_EQ(statemachine.getState(), STATE_START);
        ASSERT_EQ(r, IStateMachine::Ignored);
}

TEST_F(statemachine_tests, statemachine_handle_single_event)
{
        // Arrange
        StateMachine statemachine;
        MockStateTransition transition1;
        statemachine.add(&transition1);

        EXPECT_CALL(transition1, state)
                .WillRepeatedly(Return(STATE_START));

        EXPECT_CALL(transition1, event)
                .WillRepeatedly(Return(1));

        EXPECT_CALL(transition1, nextState)
                .WillRepeatedly(Return(STATE_START+1));
                
        EXPECT_CALL(transition1, doTransition)
                .Times(1);        
        
        // Act
        int r = statemachine.handleEvent(1, 0);
        
        //Assert
        ASSERT_EQ(r, IStateMachine::OK);
        ASSERT_EQ(statemachine.getState(), STATE_START+1);
}

TEST_F(statemachine_tests, statemachine_handle_two_events)
{
        // Arrange
        StateMachine statemachine;
        MockStateTransition transition1;
        MockStateTransition transition2;
        statemachine.add(&transition1);
        statemachine.add(&transition2);

        initMockStateTransition(transition1, STATE_START, 1, STATE_START+1);
        initMockStateTransition(transition2, STATE_START+1, 2, STATE_START+2);

        {
                InSequence seq;
                EXPECT_CALL(transition1, doTransition)
                        .Times(1);        
                EXPECT_CALL(transition2, doTransition)
                        .Times(1);
        }
        
        // Act
        vector<int> r;
        r.push_back(statemachine.handleEvent(1, 0));
        r.push_back(statemachine.handleEvent(2, 0));
        
        //Assert
        ASSERT_THAT(r, ElementsAre(IStateMachine::OK, IStateMachine::OK));
        ASSERT_EQ(statemachine.getState(), STATE_START+2);
}
