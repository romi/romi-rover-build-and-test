#include <linux/joystick.h>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../mock/mock_joystick.h"
#include "../mock/mock_eventmapper.h"
#include "JoystickInputDevice.h"
#include "rover/EventsAndStates.h"

using namespace std;
using namespace testing;
using namespace romi;

class joystickinputdevice_tests : public ::testing::Test
{
protected:
        MockJoystick joystick;
        MockEventMapper event_mapper;
        
	joystickinputdevice_tests() {
	}

	~joystickinputdevice_tests() override = default;

	void SetUp() override {
	}

	void TearDown() override {
	}
};

TEST_F(joystickinputdevice_tests, throw_exception_when_too_few_axes)
{
        try {
                EXPECT_CALL(joystick, get_num_axes())
                        .WillOnce(Return(1));
        
                JoystickInputDevice input(joystick, event_mapper);
                FAIL() << "Expected an exception";
                
        } catch (std::exception& e) {
                // NOP
        }
}

TEST_F(joystickinputdevice_tests, throw_exception_when_too_few_buttons)
{
        try {
                EXPECT_CALL(joystick, get_num_axes())
                        .WillOnce(Return(8));
                EXPECT_CALL(joystick, get_num_buttons())
                        .WillOnce(Return(1));
        
                JoystickInputDevice input(joystick, event_mapper);
                FAIL() << "Expected an exception";
                
        } catch (std::exception& e) {
                // NOP
        }
}

TEST_F(joystickinputdevice_tests, successfull_constructor)
{
        try {
                EXPECT_CALL(joystick, get_num_axes())
                        .WillOnce(Return(8));
                EXPECT_CALL(joystick, get_num_buttons())
                        .WillOnce(Return(12));
        
                JoystickInputDevice input(joystick, event_mapper);
                
        } catch (std::exception& e) {
                FAIL() << "Expected succesfull creation";
        }
}

TEST_F(joystickinputdevice_tests, successfully_gets_forward_speed)
{
        try {
                EXPECT_CALL(joystick, get_num_axes())
                        .WillOnce(Return(8));
                EXPECT_CALL(joystick, get_num_buttons())
                        .WillOnce(Return(12));
                
                EXPECT_CALL(joystick, get_axis(axis_forward_speed))
                        .WillOnce(Return(0.5));
        
                JoystickInputDevice input(joystick, event_mapper);
                double speed = input.get_forward_speed();

                ASSERT_EQ(speed, 0.5);
                
        } catch (std::exception& e) {
                FAIL() << "Expected succesfull execution";
        }
}

TEST_F(joystickinputdevice_tests, successfully_gets_backward_speed)
{
        try {
                EXPECT_CALL(joystick, get_num_axes())
                        .WillOnce(Return(8));
                EXPECT_CALL(joystick, get_num_buttons())
                        .WillOnce(Return(12));
                
                EXPECT_CALL(joystick, get_axis(axis_backward_speed))
                        .WillOnce(Return(0.7));
        
                JoystickInputDevice input(joystick, event_mapper);
                double speed = input.get_backward_speed();

                ASSERT_EQ(speed, 0.7);
                
        } catch (std::exception& e) {
                FAIL() << "Expected succesfull execution";
        }
}

TEST_F(joystickinputdevice_tests, successfully_gets_direction)
{
        try {
                EXPECT_CALL(joystick, get_num_axes())
                        .WillOnce(Return(8));
                EXPECT_CALL(joystick, get_num_buttons())
                        .WillOnce(Return(12));
                
                EXPECT_CALL(joystick, get_axis(axis_direction))
                        .WillOnce(Return(0.3));
        
                JoystickInputDevice input(joystick, event_mapper);
                double direction = input.get_direction();

                ASSERT_EQ(direction, 0.3);
                
        } catch (std::exception& e) {
                FAIL() << "Expected succesfull execution";
        }
}

TEST_F(joystickinputdevice_tests, returns_event_none)
{
        try {
                JoystickEvent joystick_event;
                        
                EXPECT_CALL(joystick, get_num_axes())
                        .WillOnce(Return(8));
                EXPECT_CALL(joystick, get_num_buttons())
                        .WillOnce(Return(12));
                
                EXPECT_CALL(joystick, get_next_event())
                        .WillOnce(ReturnRef(joystick_event));
        
                joystick_event.type = JoystickEvent::None;
                
                JoystickInputDevice input(joystick, event_mapper);
                int event = input.get_next_event();

                ASSERT_EQ(event, event_none);
                
        } catch (std::exception& e) {
                FAIL() << "Expected succesfull execution";
        }
}

TEST_F(joystickinputdevice_tests, returns_mapped_event)
{
        try {
                JoystickEvent joystick_event;
                        
                EXPECT_CALL(joystick, get_num_axes())
                        .WillOnce(Return(8));
                EXPECT_CALL(joystick, get_num_buttons())
                        .WillOnce(Return(12));
                
                EXPECT_CALL(joystick, get_next_event())
                        .WillOnce(ReturnRef(joystick_event));
                
                EXPECT_CALL(event_mapper, map(_,_))
                        .WillOnce(Return(2));
        
                joystick_event.type = JoystickEvent::Button;
                joystick_event.number = 1; // Not used
                
                JoystickInputDevice input(joystick, event_mapper);
                int event = input.get_next_event();

                ASSERT_EQ(event, 2);
                
        } catch (std::exception& e) {
                FAIL() << "Expected succesfull execution";
        }
}
