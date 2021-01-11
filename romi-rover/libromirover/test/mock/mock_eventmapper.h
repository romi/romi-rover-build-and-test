#include "gmock/gmock.h"
#include "JoystickEventMapper.h"

class MockEventMapper : public romi::JoystickEventMapper
{
public:
        MOCK_METHOD(int, map, (romi::Joystick& joystick, romi::JoystickEvent& event), (override));
};
