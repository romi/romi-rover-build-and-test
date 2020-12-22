#include <linux/joystick.h>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "LinuxJoystick.h"

#include "../mock/Linux-Mock.cpp"

using namespace std;
using namespace testing;
using namespace romi;

class joystick_tests : public ::testing::Test
{
protected:
        rpp::LinuxMock linux;
        
	joystick_tests() {
	}

	~joystick_tests() override = default;

	void SetUp() override {
	}

	void TearDown() override {
	}
};

TEST_F(joystick_tests, throw_exception_when_open_fails)
{
        try {
                EXPECT_CALL(linux, open(_,_))
                        .WillOnce(Return(-1));
                
                LinuxJoystick joystick(linux, "foo");
                FAIL() << "Expected an exception";
                
        } catch (...) {
        }
}

TEST_F(joystick_tests, throw_exception_when_ioctl_fails_1)
{
        try {
                EXPECT_CALL(linux, open(_,_))
                        .WillOnce(Return(1));
                
                EXPECT_CALL(linux, ioctl(1,_,_))
                        .WillOnce(Return(-1));
                
                EXPECT_CALL(linux, close(1))
                        .WillOnce(Return(0))
                        .RetiresOnSaturation();
                
                LinuxJoystick joystick(linux, "foo");
                FAIL() << "Expected an exception";
                
        } catch (...) {
        }
}

ACTION_P(AssignIoctlValue, param) { *static_cast<__u8*>(arg2) = param; }
ACTION_P(AssignReadValue, param) { *static_cast<struct js_event*>(arg1) = param; }
ACTION_P(AssignPollValue, value) { arg0[0].revents = value; }

TEST_F(joystick_tests, throw_exception_when_ioctl_fails_2)
{
        __u8 mock_count = 8;

        try {
                InSequence seq;
                
                EXPECT_CALL(linux, open(_,_))
                        .WillOnce(Return(1))
                        .RetiresOnSaturation();
                
                EXPECT_CALL(linux, ioctl(1,_,_))
                        .WillOnce(DoAll(AssignIoctlValue(mock_count), Return(0)))
                        .RetiresOnSaturation();
                
                EXPECT_CALL(linux, ioctl(1,_,_))
                        .WillOnce(Return(-1))
                        .RetiresOnSaturation();
                
                EXPECT_CALL(linux, close(1))
                        .WillOnce(Return(0))
                        .RetiresOnSaturation();
                
                LinuxJoystick joystick(linux, "foo");
                FAIL() << "Expected an exception";
                
        } catch (...) {
        }
}

TEST_F(joystick_tests, assure_expected_initial_state)
{
        __u8 mock_axis_count = 8;
        __u8 mock_button_count = 12;
                
        try {
                InSequence seq;
                
                EXPECT_CALL(linux, open(_,_))
                        .WillOnce(Return(1))
                        .RetiresOnSaturation();
                
                EXPECT_CALL(linux, ioctl(1,_,_))
                        .WillOnce(DoAll(AssignIoctlValue(mock_axis_count),
                                        Return(0)))
                        .RetiresOnSaturation();
                
                EXPECT_CALL(linux, ioctl(1,_,_))
                        .WillOnce(DoAll(AssignIoctlValue(mock_button_count),
                                        Return(0)))
                        .RetiresOnSaturation();
                
                EXPECT_CALL(linux, close(1))
                        .WillOnce(Return(0))
                        .RetiresOnSaturation();
                
                LinuxJoystick joystick(linux, "foo");

                ASSERT_EQ(joystick.get_num_axes(), 8);
                ASSERT_EQ(joystick.get_num_buttons(), 12);
                
                for (int i = 0; i < joystick.get_num_axes(); i++) {
                        ASSERT_EQ(joystick.get_axis(i), 0.0);
                }
                for (int i = 0; i < joystick.get_num_buttons(); i++) {
                        ASSERT_EQ(joystick.is_button_pressed(i), false);
                }
                
        } catch (...) {
                FAIL() << "Expected success";
        }
}


TEST_F(joystick_tests, successfully_read_a_button_event)
{
        __u8 mock_axis_count = 8;
        __u8 mock_button_count = 12;
        struct js_event linux_event;

        linux_event.type = JS_EVENT_BUTTON;
        linux_event.number = 5;
        linux_event.value = 1;
                
        try {
                InSequence seq;
                
                EXPECT_CALL(linux, open(_,_))
                        .WillOnce(Return(1))
                        .RetiresOnSaturation();
                
                EXPECT_CALL(linux, ioctl(1,_,_))
                        .WillOnce(DoAll(AssignIoctlValue(mock_axis_count), Return(0)))
                        .RetiresOnSaturation();
                
                EXPECT_CALL(linux, ioctl(1,_,_))
                        .WillOnce(DoAll(AssignIoctlValue(mock_button_count), Return(0)))
                        .RetiresOnSaturation();
                
                EXPECT_CALL(linux, poll(_,_,_))
                        .WillOnce(DoAll(AssignPollValue(POLLIN), Return(1)))
                        .RetiresOnSaturation();
                
                EXPECT_CALL(linux, read(1,_,_))
                        .WillOnce(DoAll(AssignReadValue(linux_event),
                                        Return(sizeof(linux_event))))
                        .RetiresOnSaturation();
                
                EXPECT_CALL(linux, close(1))
                        .WillOnce(Return(0))
                        .RetiresOnSaturation();
                
                LinuxJoystick joystick(linux, "foo");
                joystick.set_debug(true);
                
                JoystickEvent &event = joystick.get_next_event();

                ASSERT_EQ(event.type, JoystickEvent::Button);
                ASSERT_EQ(event.number, 5);
                ASSERT_EQ(joystick.is_button_pressed(5), true);
                
        } catch (...) {
                FAIL() << "Expected success";
        }
}

TEST_F(joystick_tests, successfully_read_an_axis_event)
{
        __u8 mock_axis_count = 8;
        __u8 mock_button_count = 12;
        struct js_event linux_event;

        linux_event.type = JS_EVENT_AXIS;
        linux_event.number = 3;
        linux_event.value = 32768.0 / 2.0;
                
        try {
                InSequence seq;
                
                EXPECT_CALL(linux, open(_,_))
                        .WillOnce(Return(1))
                        .RetiresOnSaturation();
                
                EXPECT_CALL(linux, ioctl(1,_,_))
                        .WillOnce(DoAll(AssignIoctlValue(mock_axis_count), Return(0)))
                        .RetiresOnSaturation();
                
                EXPECT_CALL(linux, ioctl(1,_,_))
                        .WillOnce(DoAll(AssignIoctlValue(mock_button_count), Return(0)))
                        .RetiresOnSaturation();
                
                EXPECT_CALL(linux, poll(_,_,_))
                        .WillOnce(DoAll(AssignPollValue(POLLIN), Return(1)))
                        .RetiresOnSaturation();
                
                EXPECT_CALL(linux, read(1,_,_))
                        .WillOnce(DoAll(AssignReadValue(linux_event),
                                        Return(sizeof(linux_event))))
                        .RetiresOnSaturation();
                
                EXPECT_CALL(linux, close(1))
                        .WillOnce(Return(0))
                        .RetiresOnSaturation();
                
                LinuxJoystick joystick(linux, "foo");
                joystick.set_debug(true);

                JoystickEvent &event = joystick.get_next_event();

                ASSERT_EQ(event.type, JoystickEvent::Axis);
                ASSERT_EQ(event.number, 3);
                ASSERT_EQ(joystick.get_axis(3), 0.5);
                
        } catch (...) {
                FAIL() << "Expected success";
        }
}

TEST_F(joystick_tests, thows_exception_when_poll_failed)
{
        __u8 mock_axis_count = 8;
        __u8 mock_button_count = 12;
                
        try {
                InSequence seq;
                
                EXPECT_CALL(linux, open(_,_))
                        .WillOnce(Return(1))
                        .RetiresOnSaturation();
                
                EXPECT_CALL(linux, ioctl(1,_,_))
                        .WillOnce(DoAll(AssignIoctlValue(mock_axis_count), Return(0)))
                        .RetiresOnSaturation();
                
                EXPECT_CALL(linux, ioctl(1,_,_))
                        .WillOnce(DoAll(AssignIoctlValue(mock_button_count), Return(0)))
                        .RetiresOnSaturation();
                
                EXPECT_CALL(linux, poll(_,_,_))
                        .WillOnce(Return(-1))
                        .RetiresOnSaturation();
                
                EXPECT_CALL(linux, close(1))
                        .WillOnce(Return(0))
                        .RetiresOnSaturation();
                
                LinuxJoystick joystick(linux, "foo");

                joystick.get_next_event();

                FAIL() << "Expected exception";
                
        } catch (...) {
                // NOP
        }
}

TEST_F(joystick_tests, thows_exception_when_read_failed)
{
        __u8 mock_axis_count = 8;
        __u8 mock_button_count = 12;
                
        try {
                InSequence seq;
                
                EXPECT_CALL(linux, open(_,_))
                        .WillOnce(Return(1))
                        .RetiresOnSaturation();
                
                EXPECT_CALL(linux, ioctl(1,_,_))
                        .WillOnce(DoAll(AssignIoctlValue(mock_axis_count), Return(0)))
                        .RetiresOnSaturation();
                
                EXPECT_CALL(linux, ioctl(1,_,_))
                        .WillOnce(DoAll(AssignIoctlValue(mock_button_count), Return(0)))
                        .RetiresOnSaturation();
                
                EXPECT_CALL(linux, poll(_,_,_))
                        .WillOnce(DoAll(AssignPollValue(POLLIN), Return(1)))
                        .RetiresOnSaturation();
                
                EXPECT_CALL(linux, read(1,_,_))
                        .WillOnce(Return(-1))
                        .RetiresOnSaturation();
                
                EXPECT_CALL(linux, close(1))
                        .WillOnce(Return(0))
                        .RetiresOnSaturation();
                
                LinuxJoystick joystick(linux, "foo");

                joystick.get_next_event();

                FAIL() << "Expected exception";
                
        } catch (...) {
                // NOP
        }
}
