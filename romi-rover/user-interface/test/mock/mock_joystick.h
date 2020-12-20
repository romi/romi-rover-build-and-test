#include "gmock/gmock.h"
#include "IJoystick.h"

class MockJoystick : public romi::IJoystick
{
public:
        MOCK_METHOD(bool, update, (JoystickEvent &e), (override));
        MOCK_METHOD(int, num_axes, (), (override));
        MOCK_METHOD(double, get_axis, (int i), (override));
        MOCK_METHOD(int, num_buttons, (), (override));
        MOCK_METHOD(bool, get_button, (int i), (override));
};
