#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "AnalogButton.h"
#include "ButtonPanel.h"
#include "ControlPanelTransitions.h"
#include "StateMachineTimer.h"
#include "Parser.h"
#include "Menu.h"
#include "ControlPanel.h"
#include "Buttons.h"
#include "ISerial.h"
#include "../mock/mock_arduino.h"
#include "../mock/mock_serial.h"
#include "../mock/mock_display.h"
#include "../mock/mock_relay.h"

using namespace std;
using namespace testing;

class controlpanel_tests : public ::testing::Test
{
protected:
        MockArduino arduino;
        MockSerial serial;
        MockDisplay display;
        MockRelay relayControlCircuit;
        MockRelay relayPowerCircuit;

        ButtonPanel buttonPanel;
        Parser parser;
        Menu menu;
        StateMachine stateMachine;
        StateMachineTimer stateMachineTimer;
        ControlPanel controlPanel;

        Ready ready;
        StartUp startUp;
        PowerUp powerUp;
        IgnorePowerUpWhenOn ignorePowerUp;
        Shutdown shutdown;
        SoftPowerDown softPowerDown;
        HardPowerDown hardPowerDownWhenOn;
        HardPowerDown hardPowerDownWhenStarting;
        HardPowerDown hardPowerDownWhenShuttingDown;
        ShowMenu showMenu;
        HideMenu hideMenu;
        NextMenuItem nextMenuItem;
        PreviousMenuItem previousMenuItem;
        SelectMenuItem selectMenuItem;
        SendingMenuItem sendingMenuItem;
        CancelMenuItem cancelMenuItem;
        SentMenuItem sentMenuItem;
        TimeoutMenuItem timeoutMenuItem;

        AnalogButton selectButton;
        AnalogButton upButton;
        AnalogButton downButton;
        AnalogButton menuButton;
        AnalogButton onoffButton;
        
	controlpanel_tests()
                : buttonPanel(&arduino),
                  parser("M", "01S?"),
                  stateMachineTimer(&stateMachine, &arduino),
                  controlPanel(&arduino, &serial, &display, &buttonPanel, &parser,
                               &menu, &stateMachine, &stateMachineTimer),
                  ready(&display),
                  startUp(&display, &relayControlCircuit, &relayPowerCircuit),
                  powerUp(&display, &relayControlCircuit, &relayPowerCircuit),
                  shutdown(&display, &relayControlCircuit,
                           &relayPowerCircuit, &stateMachineTimer),
                  softPowerDown(&display, &relayControlCircuit, &relayPowerCircuit),
                  hardPowerDownWhenOn(&display, &relayControlCircuit,
                                      &relayPowerCircuit, STATE_ON),
                  hardPowerDownWhenStarting(&display, &relayControlCircuit,
                                            &relayPowerCircuit, STATE_STARTING_UP),
                  hardPowerDownWhenShuttingDown(&display, &relayControlCircuit,
                                                &relayPowerCircuit, STATE_SHUTTING_DOWN),
                  showMenu(&display, &menu),
                  hideMenu(&display),
                  nextMenuItem(&display, &menu),
                  previousMenuItem(&display, &menu),
                  selectMenuItem(&display, &menu),
                  sendingMenuItem(&display, &stateMachineTimer),
                  cancelMenuItem(&display, &menu),
                  sentMenuItem(&display),
                  timeoutMenuItem(&display),
                  selectButton(&arduino, 0, BUTTON_SELECT, 0, 50),
                  upButton(&arduino, 0, BUTTON_UP, 50, 175),
                  downButton(&arduino, 0, BUTTON_DOWN, 175, 325),
                  menuButton(&arduino, 0, BUTTON_MENU, 325, 520),
                  onoffButton(&arduino, 0, BUTTON_ONOFF, 520, 850)
                {
                }

	~controlpanel_tests() override = default;

        void init_state_machine() {
                stateMachine.add(&ready);
                stateMachine.add(&startUp);
                stateMachine.add(&powerUp);
                stateMachine.add(&ignorePowerUp);
                stateMachine.add(&shutdown);
                stateMachine.add(&softPowerDown);
                stateMachine.add(&hardPowerDownWhenOn);
                stateMachine.add(&hardPowerDownWhenStarting);
                stateMachine.add(&hardPowerDownWhenShuttingDown);
                stateMachine.add(&showMenu);
                stateMachine.add(&hideMenu);
                stateMachine.add(&nextMenuItem);
                stateMachine.add(&previousMenuItem);        
                stateMachine.add(&selectMenuItem);
                stateMachine.add(&sendingMenuItem);
                stateMachine.add(&cancelMenuItem);
                stateMachine.add(&sentMenuItem);
                stateMachine.add(&timeoutMenuItem);
        }

        void init_button_panel() {
                buttonPanel.addButton(&selectButton);
                buttonPanel.addButton(&upButton);
                buttonPanel.addButton(&downButton);
                buttonPanel.addButton(&menuButton);
                buttonPanel.addButton(&onoffButton);
        }

	void SetUp() override {
                init_state_machine();
                init_button_panel();
	}

	void TearDown() override {
	}
};

TEST_F(controlpanel_tests, controlpanel_test_init)
{
        // Arrange
        EXPECT_CALL(serial, init)
                .Times(1);
        
        // Act
        controlPanel.init();
        
        //Assert
}

TEST_F(controlpanel_tests, controlpanel_test_ready_state)
{
        // Arrange
        EXPECT_CALL(serial, init)
                .Times(1);
        EXPECT_CALL(display, clear)
                .Times(1);
        EXPECT_CALL(display, setCursor)
                .Times(1);
        EXPECT_CALL(display, print)
                .Times(1);
        
        // Act
        controlPanel.init();
        stateMachine.handleEvent(EVENT_READY);
        
        //Assert
        ASSERT_EQ(stateMachine.getState(), STATE_OFF);
}

TEST_F(controlpanel_tests, controlpanel_test_startingup_state)
{
        // Arrange
        EXPECT_CALL(serial, init)
                .Times(1);
        EXPECT_CALL(display, clear)
                .Times(2);
        EXPECT_CALL(display, setCursor)
                .Times(2);
        EXPECT_CALL(display, print)
                .Times(2);
        
        EXPECT_CALL(relayControlCircuit, close)
                .Times(1);
        EXPECT_CALL(relayPowerCircuit, open)
                .Times(1);
        
        // Act
        controlPanel.init();
        stateMachine.handleEvent(EVENT_READY);
        stateMachine.handleEvent(EVENT_ONOFF_HELD);
        
        //Assert
        ASSERT_EQ(stateMachine.getState(), STATE_STARTING_UP);
}

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
        ASSERT_EQ(stateMachine.getState(), STATE_ON);
}

#if 0
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
