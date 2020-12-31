#include "gmock/gmock.h"
#include "CNCController.h"

class MockController : public romi::CNCController
{
public:
        MOCK_METHOD(bool, get_position, (int32_t *pos), (override));
        MOCK_METHOD(bool, homing, (), (override));
        MOCK_METHOD(bool, synchronize, (double timeout), (override));
        MOCK_METHOD(bool, move, (int16_t millis, int16_t steps_x, int16_t steps_y, int16_t steps_z), (override));
        MOCK_METHOD(bool, stop_execution, (), (override));
        MOCK_METHOD(bool, continue_execution, (), (override));
        MOCK_METHOD(bool, reset, (), (override));
};
