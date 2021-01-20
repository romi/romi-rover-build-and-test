#include "gmock/gmock.h"
#include "api/CNC.h"

class MockCNC : public romi::CNC
{
public:
        MOCK_METHOD(bool, get_range, (romi::CNCRange &range), (override));
        MOCK_METHOD(bool, moveto, (double x, double y, double z, double relative_speed), (override));
        MOCK_METHOD(bool, spindle, (double speed), (override));
        MOCK_METHOD(bool, travel, (romi::Path &path, double relative_speed), (override));
        MOCK_METHOD(bool, homing, (), (override));
        MOCK_METHOD(bool, pause_activity, (), (override));
        MOCK_METHOD(bool, continue_activity, (), (override));
        MOCK_METHOD(bool, reset_activity, (), (override));
};
