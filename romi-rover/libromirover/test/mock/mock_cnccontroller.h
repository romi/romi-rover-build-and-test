#include "gmock/gmock.h"
#include "oquam/CNCController.h"

class MockCNCController : public romi::CNCController
{
public:
        MOCK_METHOD(bool, get_position, (int32_t *pos), (override));
        MOCK_METHOD(bool, homing, (), (override));
        MOCK_METHOD(bool, synchronize, (double timeout), (override));
        MOCK_METHOD(bool, move, (int16_t millis, int16_t steps_x, int16_t steps_y, int16_t steps_z), (override));
        MOCK_METHOD(bool, pause_activity, (), (override));
        MOCK_METHOD(bool, continue_activity, (), (override));
        MOCK_METHOD(bool, reset_activity, (), (override));
        MOCK_METHOD(bool, enable, (), (override));
        MOCK_METHOD(bool, disable, (), (override));
        MOCK_METHOD(bool, configure_homing, (AxisIndex axis1, AxisIndex axis2, AxisIndex axis3), (override));
};
