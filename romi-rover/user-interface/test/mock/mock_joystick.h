#include "gmock/gmock.h"
#include "Joystick.h"

class MockJoystick : public romi::Joystick
{
public:
        MOCK_METHOD(romi::JoystickEvent&, get_next_event, (), (override));
        MOCK_METHOD(int, get_num_axes, (), (override));
        MOCK_METHOD(double, get_axis, (int i), (override));
        MOCK_METHOD(int, get_num_buttons, (), (override));
        MOCK_METHOD(bool, is_button_pressed, (int i), (override));
};
