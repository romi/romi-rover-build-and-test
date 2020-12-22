#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../mock/mock_inputdevice.h"
#include "../mock/mock_display.h"
#include "../mock/mock_speedcontroller.h"
#include "UserInterface.h"
#include "EventsAndStates.h"

using namespace std;
using namespace testing;
using namespace romi;

class userinterface_tests : public ::testing::Test
{
protected:
        MockInputDevice input_device;
        MockDisplay display;
        MockSpeedController speed_controller;
        
	userinterface_tests() {
	}

	~userinterface_tests() override = default;

	void SetUp() override {
	}

	void TearDown() override {
	}
};

TEST_F(userinterface_tests, create_display_uses_options_first)
{
        UserInterface interface(input_device, display, speed_controller);

        ASSERT_EQ(interface.get_state(), state_stopped);
}
