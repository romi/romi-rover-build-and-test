#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "StateMachine.h"

using namespace std;
using namespace testing;
using namespace romi;

class Foo
{
public:
        virtual bool do_transition_1() = 0;
        virtual bool do_transition_2() = 0;
        virtual bool do_transition_3() = 0;
};
        
class MockFoo : public Foo
{
public:
        MOCK_METHOD(bool, do_transition_1, (), (override));
        MOCK_METHOD(bool, do_transition_2, (), (override));
        MOCK_METHOD(bool, do_transition_3, (), (override));
};

bool state_transition_1(Foo& target)
{
        target.do_transition_1();
        return true;
}

bool state_transition_2(Foo& target)
{
        target.do_transition_2();
        return true;
}

bool state_transition_3(Foo& target)
{
        target.do_transition_3();
        return true;
}

////

class statemachine_tests : public ::testing::Test
{
protected:
        MockFoo foo;
        
	statemachine_tests() {
	}

	~statemachine_tests() override = default;

	void SetUp() override {
	}

	void TearDown() override {
	}
};

TEST_F(statemachine_tests, calls_state_transition)
{
        StateMachine<Foo> state_machine(foo);
        int event_1 = 1;
        int state_1 = 101;

        EXPECT_CALL(foo, do_transition_1())
                .WillOnce(Return(true));
        
        state_machine.add(STATE_START, event_1, state_transition_1, state_1);
        state_machine.handle_event(event_1);
        
        ASSERT_EQ(state_machine.get_state(), state_1);
}

TEST_F(statemachine_tests, calls_normal_state_transition_before_catchall)
{
        StateMachine<Foo> state_machine(foo);

        EXPECT_CALL(foo, do_transition_1())
                .WillOnce(Return(true));
        EXPECT_CALL(foo, do_transition_2())
                .WillOnce(Return(true));

        int event_1 = 1;
        int event_2 = 2;
        int state_1 = 101;
        int state_2 = 102;
        int state_3 = 103;
        
        state_machine.add(STATE_START, event_1, state_transition_1, state_1);        
        state_machine.add(state_1, event_2, state_transition_2, state_2);
        state_machine.add(ALL_STATES, event_2, state_transition_3, state_3);
        
        state_machine.handle_event(event_1);
        state_machine.handle_event(event_2);
        
        ASSERT_EQ(state_machine.get_state(), state_2);
}

TEST_F(statemachine_tests, calls_catchall_state_transition)
{
        StateMachine<Foo> state_machine(foo);

        EXPECT_CALL(foo, do_transition_1())
                .WillOnce(Return(true));
        EXPECT_CALL(foo, do_transition_3())
                .WillOnce(Return(true));

        int event_1 = 1;
        int event_2 = 2;
        int state_1 = 101;
        int state_3 = 103;
        
        state_machine.add(STATE_START, event_1, state_transition_1, state_1);        
        state_machine.add(ALL_STATES, event_2, state_transition_3, state_3);
        
        state_machine.handle_event(event_1);
        state_machine.handle_event(event_2);
        
        ASSERT_EQ(state_machine.get_state(), state_3);
}
