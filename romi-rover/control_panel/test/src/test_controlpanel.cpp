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
#include "../mock/mock_arduino.h"
#include "../mock/mock_inputstream.h"
#include "../mock/mock_outputstream.h"
#include "../mock/mock_display.h"
#include "../mock/mock_relay.h"
#include "../mock/mock_timer.h"
#include "../mock/mock_button.h"

using namespace std;
using namespace testing;

class inputs
{
public:
        string serial;
        int onoff;
        int menu;
        int up;
        int down;
        int select;
        int timer;
        
        inputs() : serial(""), onoff(0), menu(0), up(0),
                   down(0), select(0), timer(0) {}
};

class outputs
{
public:
        enum { RelayNotSet, RelayOpen, RelayClose };
        string serial;
        int controlRelay;
        int powerRelay;
        string displayState;
        string displayMenu;
        int state;
        
        outputs() : serial(""), controlRelay(RelayNotSet), powerRelay(RelayNotSet),
                    displayState("?"), displayMenu(""), state(-1) {}
};

class controlpanel_tests : public ::testing::Test
{
protected:
        MockArduino arduino;
        MockInputStream in;
        MockOutputStream out;
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
                : controlPanel(&in, &buttonPanel, &eventTimer, &out, &display,
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

        void initSerialRead(inputs input[], int num_updates) {
                InSequence seq;
                for (int i = 0; i < num_updates; i++) {
                        int len = input[i].serial.length();
                        for (int j = 0; j < len; j++) {
                                EXPECT_CALL(in, read)
                                        .WillOnce(Return(input[i].serial.at(j)))
                                        .RetiresOnSaturation();;
                        }
                }
        }
        
        void initSerialAvailable(inputs input[], int num_updates) {
                InSequence seq;
                for (int i = 0; i < num_updates; i++) {
                        int len = input[i].serial.length();
                        for (int j = 0; j <= len; j++) {
                                EXPECT_CALL(in, available)
                                        .WillOnce(Return(len - j))
                                        .RetiresOnSaturation();
                        }
                }
        }
        
        void initOnOffButton(inputs input[], int num_updates) {
                InSequence seq;
                for (int i = 0; i < num_updates; i++) {
                        EXPECT_CALL(onoffButton, update)
                                .WillOnce(Return(input[i].onoff))
                                .RetiresOnSaturation();
                        if (input[i].onoff) {
                                EXPECT_CALL(onoffButton, id)
                                        .WillOnce(Return(BUTTON_ONOFF))
                                        .RetiresOnSaturation();
                        }
                }
        }
        
        void initMenuButton(inputs input[], int num_updates) {
                InSequence seq;
                for (int i = 0; i < num_updates; i++) {
                        EXPECT_CALL(menuButton, update)
                                .WillOnce(Return(input[i].menu))
                                .RetiresOnSaturation();
                        if (input[i].menu) {
                                EXPECT_CALL(menuButton, id)
                                        .WillOnce(Return(BUTTON_MENU))
                                        .RetiresOnSaturation();
                        }
                }
        }

        void initUpButton(inputs input[], int num_updates) {
                InSequence seq;
                for (int i = 0; i < num_updates; i++) {
                        EXPECT_CALL(upButton, update)
                                .WillOnce(Return(input[i].up))
                                .RetiresOnSaturation();
                        if (input[i].up) {
                                EXPECT_CALL(upButton, id)
                                        .WillOnce(Return(BUTTON_UP))
                                        .RetiresOnSaturation();
                        }
                }
        }

        void initDownButton(inputs input[], int num_updates) {
                InSequence seq;
                for (int i = 0; i < num_updates; i++) {
                        EXPECT_CALL(downButton, update)
                                .WillOnce(Return(input[i].down))
                                .RetiresOnSaturation();
                        if (input[i].down) {
                                EXPECT_CALL(downButton, id)
                                        .WillOnce(Return(BUTTON_DOWN))
                                        .RetiresOnSaturation();
                        }
                }
        }

        void initSelectButton(inputs input[], int num_updates) {
                InSequence seq;
                for (int i = 0; i < num_updates; i++) {
                        EXPECT_CALL(selectButton, update)
                                .WillOnce(Return(input[i].select))
                                .RetiresOnSaturation();
                        if (input[i].select) {
                                EXPECT_CALL(selectButton, id)
                                        .WillOnce(Return(BUTTON_SELECT))
                                        .RetiresOnSaturation();
                        }
                }
        }

        void initEventTimer(inputs input[], int num_updates) {
                InSequence seq;
                for (int i = 0; i < num_updates; i++) {
                        EXPECT_CALL(eventTimer, update)
                                .WillOnce(Return(input[i].timer))
                                .RetiresOnSaturation();
                }
        }
        
        void initInputs(inputs input[], int num_updates) {
                initSerialAvailable(input, num_updates);
                initSerialRead(input, num_updates);
                initOnOffButton(input, num_updates);
                initMenuButton(input, num_updates);
                initUpButton(input, num_updates);
                initDownButton(input, num_updates);
                initSelectButton(input, num_updates);
                initEventTimer(input, num_updates);
        }

        void initRelayChanges(outputs output[], int num_updates) {
                InSequence seq;
                for (int i = 0; i < num_updates; i++) {
                        if (output[i].controlRelay == outputs::RelayOpen) {
                                EXPECT_CALL(relayControlCircuit, open)
                                        .Times(1)
                                        .RetiresOnSaturation();
                        } else if (output[i].controlRelay == outputs::RelayClose) {
                                EXPECT_CALL(relayControlCircuit, close)
                                        .Times(1)
                                        .RetiresOnSaturation();
                        }
                        if (output[i].powerRelay == outputs::RelayOpen) {
                                EXPECT_CALL(relayPowerCircuit, open)
                                        .Times(1)
                                        .RetiresOnSaturation();
                        } else if (output[i].powerRelay == outputs::RelayClose) {
                                EXPECT_CALL(relayPowerCircuit, close)
                                        .Times(1)
                                        .RetiresOnSaturation();
                        }
                }
        }

        void initSerialOutputs(outputs output[], int num_updates) {
                InSequence seq;
                for (int i = 0; i < num_updates; i++) {
                        int len = output[i].serial.length();
                        if (len) {
                                EXPECT_CALL(out, println(StrEq(output[i].serial)))
                                        .Times(1)
                                        .RetiresOnSaturation();
                        }
                }
        }

        void initStateDisplayChanges(outputs output[], int num_updates) {
                InSequence seq;

                // controlPanel.init calls showState('Off') once.
                EXPECT_CALL(display, showState(StrEq(STATE_OFF_STR)))
                        .Times(1);
                
                for (int i = 0; i < num_updates; i++) {
                        EXPECT_CALL(display, showState(StrEq(output[i].displayState)))
                                .Times(1)
                                .RetiresOnSaturation();
                }
        }
        
        void initMenuDisplayChanges(outputs output[], int num_updates) {
                InSequence seq;
                for (int i = 0; i < num_updates; i++) {
                        if (output[i].displayMenu.length() > 0) 
                                EXPECT_CALL(display, showMenu(StrEq(output[i].displayMenu)))
                                        .Times(1)
                                        .RetiresOnSaturation();
                }
        }
        
        void initOutputs(outputs output[], int num_updates) {
                initRelayChanges(output, num_updates);
                initSerialOutputs(output, num_updates);
                initStateDisplayChanges(output, num_updates);
                initMenuDisplayChanges(output, num_updates);
        }
        
	void TearDown() override {
	}
};

TEST_F(controlpanel_tests, controlpanel_test_init)
{
        // Arrange
        EXPECT_CALL(display, showState(StrEq("Off")))
                .Times(1);
        
        // Act
        controlPanel.init();
        
        //Assert
        ASSERT_EQ(controlPanel.stateMachine()->getState(), STATE_OFF);
}

TEST_F(controlpanel_tests, controlpanel_test_starting_up)
{
        // Arrange
        const int num_updates = 1;
        inputs input[num_updates];
        outputs output[num_updates];

        // Inputs
        input[0].onoff = BUTTON_HELD;
        
        // Expected outputs
        output[0].displayState = STATE_STARTING_UP_STR;
        output[0].controlRelay = outputs::RelayClose;
        output[0].powerRelay = outputs::RelayOpen;
        output[0].state = STATE_STARTING_UP;
        
        initInputs(input, num_updates);
        initOutputs(output, num_updates);
        
        // Act
        controlPanel.init();
        for (int i = 0; i < num_updates; i++)
                controlPanel.update(i);
        
        //Assert
        ASSERT_EQ(controlPanel.stateMachine()->getState(), output[num_updates-1].state);
}

TEST_F(controlpanel_tests, controlpanel_test_on)
{
        // Arrange
        const int num_updates = 2;
        inputs input[num_updates];
        outputs output[num_updates];

        // Inputs
        input[0].onoff = BUTTON_HELD;
        input[1].serial = "1\n";
        
        // Expected outputs
        output[0].displayState = STATE_STARTING_UP_STR;
        output[0].controlRelay = outputs::RelayClose;
        output[0].powerRelay = outputs::RelayOpen;
        output[0].state = STATE_STARTING_UP;
        
        output[1].serial = "OK";
        output[1].displayState = STATE_ON_STR; 
        output[1].controlRelay = outputs::RelayClose;
        output[1].powerRelay = outputs::RelayClose;
        output[1].state = STATE_ON;

        
        initInputs(input, num_updates);
        initOutputs(output, num_updates);
        
        // Act
        controlPanel.init();
        for (int i = 0; i < num_updates; i++)
                controlPanel.update(i);
        
        //Assert
        ASSERT_EQ(controlPanel.stateMachine()->getState(), output[num_updates-1].state);
}

TEST_F(controlpanel_tests, controlpanel_test_menu)
{
        // Arrange
        const int num_updates = 3;
        inputs input[num_updates];
        outputs output[num_updates];

        controlPanel.menu()->setMenuItem("menu1", 0);
        
        // Inputs
        input[0].onoff = BUTTON_HELD;
        input[1].serial = "1\n";
        input[2].menu = BUTTON_HELD;
        
        // Expected outputs
        output[0].displayState = STATE_STARTING_UP_STR;
        output[0].controlRelay = outputs::RelayClose;
        output[0].powerRelay = outputs::RelayOpen;
        output[0].state = STATE_STARTING_UP;
        
        output[1].serial = "OK";
        output[1].displayState = STATE_ON_STR; 
        output[1].controlRelay = outputs::RelayClose;
        output[1].powerRelay = outputs::RelayClose;
        output[1].state = STATE_ON;
        
        output[2].displayState = STATE_MENU_STR; 
        output[2].displayMenu = "menu1"; 
        output[2].state = STATE_MENU;
        
        initInputs(input, num_updates);
        initOutputs(output, num_updates);

        
        
        // Act
        controlPanel.init();
        for (int i = 0; i < num_updates; i++)
                controlPanel.update(i);
        
        //Assert
        ASSERT_EQ(controlPanel.stateMachine()->getState(), output[num_updates-1].state);
}
