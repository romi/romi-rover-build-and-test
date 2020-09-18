#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "AnalogButton.h"
#include "ButtonPanel.h"
#include "../mock/mock_arduino.h"
#include "../mock/mock_statemachine.h"

using namespace std;
using namespace testing;

class buttonpanel_tests : public ::testing::Test
{
protected:
	buttonpanel_tests() {
	}

	~buttonpanel_tests() override = default;

	void SetUp() override {
	}

	void TearDown() override {
	}
};

TEST_F(buttonpanel_tests, buttonpanel_test_create_button)
{
        // Arrange
        MockArduino arduino;
        AnalogButton button(&arduino, 0, 1, 2, 3);

        // Act
        //
        
        //Assert
        ASSERT_EQ(button.id(), 1);
        ASSERT_EQ(button.getmin(), 2);
        ASSERT_EQ(button.getmax(), 3);
        ASSERT_EQ(button.state(), IButton::Released);
}

TEST_F(buttonpanel_tests, buttonpanel_test_button_still_released)
{
        // Arrange
        MockArduino arduino;
        AnalogButton button(&arduino, 0, 1, 100, 200);

        EXPECT_CALL(arduino, analogRead)
                .WillOnce(Return(0));

        // Act
        uint8_t r = button.update(0);
        
        //Assert
        ASSERT_EQ(button.state(), IButton::Released);
        ASSERT_EQ(button.timestamp(), 0);
        ASSERT_EQ(r, 0);
}

TEST_F(buttonpanel_tests, buttonpanel_test_button_down)
{
        // Arrange
        MockArduino arduino;
        AnalogButton button(&arduino, 0, 1, 100, 200);

        EXPECT_CALL(arduino, analogRead)
                .WillOnce(Return(150));

        // Act
        uint8_t r = button.update(100);
        
        //Assert
        ASSERT_EQ(button.state(), IButton::Down);
        ASSERT_EQ(button.timestamp(), 100);
        ASSERT_EQ(r, 0);
}

TEST_F(buttonpanel_tests, buttonpanel_test_button_released)
{
        // Arrange
        MockArduino arduino;
        AnalogButton button(&arduino, 0, 1, 100, 200);

        EXPECT_CALL(arduino, analogRead)
                .WillOnce(Return(150))
                .WillOnce(Return(0));

        // Act
        uint8_t r1 = button.update(100);
        uint8_t r2 = button.update(101);
        
        //Assert
        ASSERT_EQ(button.state(), IButton::Released);
        ASSERT_EQ(button.timestamp(), 100);
        ASSERT_EQ(r1, 0);
        ASSERT_EQ(r2, 0);
}

TEST_F(buttonpanel_tests, buttonpanel_test_button_still_dowwn)
{
        // Arrange
        MockArduino arduino;
        AnalogButton button(&arduino, 0, 1, 100, 200);

        EXPECT_CALL(arduino, analogRead)
                .WillOnce(Return(150))
                .WillOnce(Return(150));

        // Act
        uint8_t r1 = button.update(100);
        uint8_t r2 = button.update(10 + MIN_TIME_PRESSED - 1);
        
        //Assert
        ASSERT_EQ(button.state(), IButton::Down);
        ASSERT_EQ(button.timestamp(), 100);
        ASSERT_EQ(r1, 0);
        ASSERT_EQ(r2, 0);
}

TEST_F(buttonpanel_tests, buttonpanel_test_button_pressed_1)
{
        // Arrange
        MockArduino arduino;
        AnalogButton button(&arduino, 0, 1, 100, 200);

        EXPECT_CALL(arduino, analogRead)
                .WillOnce(Return(150))
                .WillOnce(Return(150));

        // Act
        uint8_t r1 = button.update(100);
        uint8_t r2 = button.update(100 + MIN_TIME_PRESSED + 1);
        
        //Assert
        ASSERT_EQ(button.state(), IButton::Pressed);
        ASSERT_EQ(button.timestamp(), 100);
        ASSERT_EQ(r1, 0);
        ASSERT_EQ(r2, BUTTON_PRESSED);
}

TEST_F(buttonpanel_tests, buttonpanel_test_button_pressed_2)
{
        // Arrange
        MockArduino arduino;
        AnalogButton button(&arduino, 0, 1, 100, 200);

        EXPECT_CALL(arduino, analogRead)
                .WillOnce(Return(150))
                .WillOnce(Return(150))
                .WillOnce(Return(0));

        // Act
        uint8_t r1 = button.update(100);
        uint8_t r2 = button.update(100 + MIN_TIME_PRESSED + 1);
        uint8_t r3 = button.update(100 + MIN_TIME_PRESSED + 2);
        
        //Assert
        ASSERT_EQ(button.state(), IButton::Released);
        ASSERT_EQ(button.timestamp(), 100);
        ASSERT_EQ(r1, 0);
        ASSERT_EQ(r2, BUTTON_PRESSED);
        ASSERT_EQ(r3, 0);
}

TEST_F(buttonpanel_tests, buttonpanel_test_button_pressed_3)
{
        // Arrange
        MockArduino arduino;
        AnalogButton button(&arduino, 0, 1, 100, 200);

        EXPECT_CALL(arduino, analogRead)
                .WillOnce(Return(150))
                .WillOnce(Return(150))
                .WillOnce(Return(150));

        // Act
        uint8_t r1 = button.update(100);
        uint8_t r2 = button.update(100 + MIN_TIME_PRESSED + 1);
        uint8_t r3 = button.update(100 + MIN_TIME_PRESSED + 2);
        
        //Assert
        ASSERT_EQ(button.state(), IButton::Pressed);
        ASSERT_EQ(button.timestamp(), 100);
        ASSERT_EQ(r1, 0);
        ASSERT_EQ(r2, BUTTON_PRESSED);
        ASSERT_EQ(r3, 0);
}

TEST_F(buttonpanel_tests, buttonpanel_test_button_pressed_4)
{
        // Arrange
        MockArduino arduino;
        AnalogButton button(&arduino, 0, 1, 100, 200);

        EXPECT_CALL(arduino, analogRead)
                .WillOnce(Return(150))
                .WillOnce(Return(150))
                .WillOnce(Return(150))
                .WillOnce(Return(0));

        // Act
        uint8_t r1 = button.update(100);
        uint8_t r2 = button.update(100 + MIN_TIME_PRESSED + 1);
        uint8_t r3 = button.update(100 + MIN_TIME_PRESSED + 2);
        uint8_t r4 = button.update(100 + MIN_TIME_PRESSED + 3);
        
        //Assert
        ASSERT_EQ(button.state(), IButton::Released);
        ASSERT_EQ(button.timestamp(), 100);
        ASSERT_EQ(r1, 0);
        ASSERT_EQ(r2, BUTTON_PRESSED);
        ASSERT_EQ(r3, 0);
        ASSERT_EQ(r4, 0);
}

TEST_F(buttonpanel_tests, buttonpanel_test_button_pressed_5)
{
        // Arrange
        MockArduino arduino;
        AnalogButton button(&arduino, 0, 1, 100, 200);

        EXPECT_CALL(arduino, analogRead)
                .WillOnce(Return(150))
                .WillOnce(Return(150))
                .WillOnce(Return(150))
                .WillOnce(Return(0))
                .WillOnce(Return(150));

        // Act
        uint8_t r1 = button.update(100);
        uint8_t r2 = button.update(100 + MIN_TIME_PRESSED + 1);
        uint8_t r3 = button.update(100 + MIN_TIME_PRESSED + 2);
        uint8_t r4 = button.update(100 + MIN_TIME_PRESSED + 3);
        uint8_t r5 = button.update(100 + MIN_TIME_PRESSED + 100);
        
        //Assert
        ASSERT_EQ(button.state(), IButton::Down);
        ASSERT_EQ(button.timestamp(), 100 + MIN_TIME_PRESSED + 100);
        ASSERT_EQ(r1, 0);
        ASSERT_EQ(r2, BUTTON_PRESSED);
        ASSERT_EQ(r3, 0);
        ASSERT_EQ(r4, 0);
        ASSERT_EQ(r5, 0);
}

TEST_F(buttonpanel_tests, buttonpanel_test_button_held_1)
{
        // Arrange
        MockArduino arduino;
        AnalogButton button(&arduino, 0, 1, 100, 200);

        EXPECT_CALL(arduino, analogRead)
                .WillOnce(Return(150))
                .WillOnce(Return(150));

        // Act
        uint8_t r1 = button.update(100);
        uint8_t r2 = button.update(100 + MIN_TIME_HELD + 1);
        
        //Assert
        ASSERT_EQ(button.state(), IButton::Held);
        ASSERT_EQ(button.timestamp(), 100);
        ASSERT_EQ(r1, 0);
        ASSERT_EQ(r2, BUTTON_HELD);
}

TEST_F(buttonpanel_tests, buttonpanel_test_button_held_2)
{
        // Arrange
        MockArduino arduino;
        AnalogButton button(&arduino, 0, 1, 100, 200);

        EXPECT_CALL(arduino, analogRead)
                .WillOnce(Return(150))
                .WillOnce(Return(150))
                .WillOnce(Return(0));
        
        // Act
        uint8_t r1 = button.update(100);
        uint8_t r2 = button.update(100 + MIN_TIME_HELD + 1);
        uint8_t r3 = button.update(100 + MIN_TIME_HELD + 2);
        
        //Assert
        ASSERT_EQ(button.state(), IButton::Released);
        ASSERT_EQ(button.timestamp(), 100);
        ASSERT_EQ(r1, 0);
        ASSERT_EQ(r2, BUTTON_HELD);
        ASSERT_EQ(r3, 0);
}

TEST_F(buttonpanel_tests, buttonpanel_test_buttonpanel_button_pressed)
{
        // Arrange
        MockArduino arduino;
        MockStateMachine stateMachine;
        AnalogButton button(&arduino, 0, 1, 100, 200);

        EXPECT_CALL(arduino, analogRead)
                .WillOnce(Return(150))
                .WillOnce(Return(150));
        
        EXPECT_CALL(arduino, millis)
                .WillOnce(Return(100))
                .WillOnce(Return(100 + MIN_TIME_PRESSED + 1));
        
        EXPECT_CALL(stateMachine, handleEvent(ButtonEvent(1, BUTTON_PRESSED)))
                .Times(1);        
        
        // Act
        ButtonPanel buttonpannel(&arduino);
        buttonpannel.addButton(&button);
        buttonpannel.update(&stateMachine);
        buttonpannel.update(&stateMachine);

        //Assert
        ASSERT_EQ(button.state(), IButton::Pressed);
}
