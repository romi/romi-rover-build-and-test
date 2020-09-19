#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "AnalogButton.h"
#include "ButtonPanel.h"
#include "ControlPanelTransitions.h"
#include "EventTimer.h"
#include "Parser.h"
#include "Menu.h"
#include "ControlPanel.h"
#include "Buttons.h"
#include "ISerial.h"
#include "../mock/mock_arduino.h"
#include "../mock/mock_serial.h"
#include "../mock/mock_display.h"
#include "../mock/mock_relay.h"
#include "../mock/mock_timer.h"
#include "../mock/mock_button.h"

using namespace std;
using namespace testing;

class controlpanel_tests : public ::testing::Test
{
protected:
        MockArduino arduino;
        MockSerial serial;
        ButtonPanel buttonPanel;
        MockButton selectButton;
        MockButton upButton;
        MockButton downButton;
        MockButton menuButton;
        MockButton onoffButton;        
        MockTimer eventTimer;
        MockDisplay display;
        MockRelay relayControlCircuit;
        MockRelay relayPowerCircuit;        
        ControlPanel controlPanel;
        
	controlpanel_tests()
                : controlPanel(&serial, &buttonPanel, &eventTimer, &display,
                               &relayControlCircuit, &relayPowerCircuit) {
                buttonPanel.addButton(&selectButton);
                buttonPanel.addButton(&upButton);
                buttonPanel.addButton(&downButton);
                buttonPanel.addButton(&menuButton);
                buttonPanel.addButton(&onoffButton);
        }
        
	~controlpanel_tests() override = default;

	void SetUp() override {
                EXPECT_CALL(display, clearMenu)
                        .Times(AtLeast(1));

	}

	void TearDown() override {
	}
};

TEST_F(controlpanel_tests, controlpanel_test_init)
{
        // Arrange
        EXPECT_CALL(display, showState(StrEq("Off")))
                .Times(1);
        // EXPECT_CALL(display, clearMenu)
        //         .Times(1);
        
        // Act
        controlPanel.init();
        
        //Assert
        ASSERT_EQ(controlPanel.stateMachine()->getState(), STATE_OFF);
}

TEST_F(controlpanel_tests, controlpanel_test_update_0)
{
        // Arrange
        EXPECT_CALL(display, showState(StrEq("Off")))
                .Times(1);
        // EXPECT_CALL(display, clearMenu)
        //         .Times(1);

        EXPECT_CALL(serial, available)
                .Times(AtLeast(1));
        
        EXPECT_CALL(onoffButton, update)
                .Times(1);
        EXPECT_CALL(menuButton, update)
                .Times(1);
        EXPECT_CALL(downButton, update)
                .Times(1);
        EXPECT_CALL(upButton, update)
                .Times(1);
        EXPECT_CALL(selectButton, update)
                .Times(1);
        
        EXPECT_CALL(eventTimer, update)
                .Times(1);
        
        // Act
        controlPanel.init();
        controlPanel.update(0);
        
        //Assert
        ASSERT_EQ(controlPanel.stateMachine()->getState(), STATE_OFF);
}

TEST_F(controlpanel_tests, controlpanel_test_starting_up)
{
        // Arrange
        {
                InSequence seq;
                EXPECT_CALL(display, showState(StrEq(STATE_OFF_STR)))
                        .Times(1);
                EXPECT_CALL(display, showState(StrEq(STATE_STARTING_UP_STR)))
                        .Times(1);
        }
        
        // EXPECT_CALL(display, clearMenu)
        //         .Times(AtLeast(1));

        EXPECT_CALL(serial, available)
                .Times(AtLeast(1));
        
        EXPECT_CALL(onoffButton, update)
                .WillOnce(Return(BUTTON_HELD));
        EXPECT_CALL(onoffButton, id)
                .Times(1);

        EXPECT_CALL(menuButton, update)
                .Times(AtLeast(1));
        EXPECT_CALL(downButton, update)
                .Times(AtLeast(1));
        EXPECT_CALL(upButton, update)
                .Times(AtLeast(1));
        EXPECT_CALL(selectButton, update)
                .Times(AtLeast(1));
        
        EXPECT_CALL(eventTimer, update)
                .Times(AtLeast(1));
        
        EXPECT_CALL(relayControlCircuit, close)
                .Times(1);
        
        EXPECT_CALL(relayPowerCircuit, open)
                .Times(1);

        
        // Act
        controlPanel.init();
        controlPanel.update(0);
        
        //Assert
        ASSERT_EQ(controlPanel.stateMachine()->getState(), STATE_STARTING_UP);
}

TEST_F(controlpanel_tests, controlpanel_test_on)
{
        // Arrange
        {
                InSequence seq;
                EXPECT_CALL(display, showState(StrEq(STATE_OFF_STR)))
                        .Times(1);
                EXPECT_CALL(display, showState(StrEq(STATE_STARTING_UP_STR)))
                        .Times(1);
                EXPECT_CALL(display, showState(StrEq(STATE_ON_STR)))
                        .Times(1);
        }
        
        // EXPECT_CALL(display, clearMenu)
        //         .Times(AtLeast(1));

        EXPECT_CALL(serial, available)
                .WillOnce(Return(0))
                .WillOnce(Return(2))
                .WillOnce(Return(1))
                .WillOnce(Return(0));
        EXPECT_CALL(serial, read)
                .WillOnce(Return('1'))
                .WillOnce(Return('\r'));
        EXPECT_CALL(serial, println(StrEq("OK")))
                .Times(1);
        
        EXPECT_CALL(onoffButton, update)
                .WillOnce(Return(BUTTON_HELD))
                .WillOnce(Return(0));
        EXPECT_CALL(onoffButton, id)
                .Times(1);

        EXPECT_CALL(menuButton, update)
                .Times(AtLeast(1));
        EXPECT_CALL(downButton, update)
                .Times(AtLeast(1));
        EXPECT_CALL(upButton, update)
                .Times(AtLeast(1));
        EXPECT_CALL(selectButton, update)
                .Times(AtLeast(1));
        
        EXPECT_CALL(eventTimer, update)
                .Times(AtLeast(1));
        
        {
                InSequence seq;
                EXPECT_CALL(relayControlCircuit, close)
                        .Times(1);
                EXPECT_CALL(relayControlCircuit, close)
                        .Times(1);
        }
        
        {
                InSequence seq;
                EXPECT_CALL(relayPowerCircuit, open)
                        .Times(1);
                EXPECT_CALL(relayPowerCircuit, close)
                        .Times(1);
        }
        
        // Act
        controlPanel.init();
        controlPanel.update(0);
        controlPanel.update(1);
        
        //Assert
        ASSERT_EQ(controlPanel.stateMachine()->getState(), STATE_ON);
}

#if 0
TEST_F(controlpanel_tests, controlpanel_test_on_state)
{
        // Arrange
        EXPECT_CALL(serial, init)
                .Times(1);
        EXPECT_CALL(display, clear)
                .Times(AtLeast(2));
        EXPECT_CALL(display, setCursor)
                .Times(AtLeast(2));
        EXPECT_CALL(display, print)
                .Times(AtLeast(2));
        
        EXPECT_CALL(relayControlCircuit, close)
                .Times(2);
        {
                InSequence seq;
                EXPECT_CALL(relayPowerCircuit, open)
                        .Times(1);
                EXPECT_CALL(relayPowerCircuit, close)
                        .Times(1);
        }
        
        // Act
        controlPanel.init();
        stateMachine.handleEvent(EVENT_READY);
        stateMachine.handleEvent(EVENT_ONOFF_HELD);
        stateMachine.handleEvent(EVENT_POWERUP);
        
        //Assert
        ASSERT_EQ(controlPanel.stateMachine()->getState(), STATE_ON);
}

TEST_F(controlpanel_tests, controlpanel_test_shuttingdown_state)
{
        // Arrange
        EXPECT_CALL(serial, init)
                .Times(1);
        EXPECT_CALL(display, clear)
                .Times(AtLeast(2));
        EXPECT_CALL(display, setCursor)
                .Times(AtLeast(2));
        EXPECT_CALL(display, print)
                .Times(AtLeast(2));
        
        EXPECT_CALL(relayControlCircuit, open)
                .Times(3);

        {
                InSequence seq;
                EXPECT_CALL(relayPowerCircuit, open)
                        .Times(1);
                EXPECT_CALL(relayPowerCircuit, close)
                        .Times(1);
                EXPECT_CALL(relayPowerCircuit, open)
                        .Times(1);
        }
        
        // Act
        controlPanel.init();
        stateMachine.handleEvent(EVENT_READY);
        stateMachine.handleEvent(EVENT_ONOFF_HELD);
        stateMachine.handleEvent(EVENT_POWERUP);
        stateMachine.handleEvent(EVENT_ONOFF_HELD);
        
        //Assert
        ASSERT_EQ(stateMachine.getState(), STATE_SHUTTING_DOWN);
}
#endif
