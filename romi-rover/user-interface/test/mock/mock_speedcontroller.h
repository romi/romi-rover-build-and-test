#include "gmock/gmock.h"
#include "SpeedController.h"

class MockSpeedController : public romi::SpeedController
{
public:
        MOCK_METHOD(bool, stop, (), (override));
        MOCK_METHOD(bool, drive_at, (double speed, double direction), (override));
        MOCK_METHOD(bool, drive_accurately_at, (double speed, double direction), (override));
        MOCK_METHOD(bool, spin, (double direction), (override));
};
