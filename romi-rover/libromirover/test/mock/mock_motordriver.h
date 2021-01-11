#include "gmock/gmock.h"
#include "MotorDriver.h"

class MockMotorDriver : public romi::MotorDriver
{
public:
        MOCK_METHOD(bool, moveat, (double left, double right), (override));
        MOCK_METHOD(bool, stop, (), (override));
        MOCK_METHOD(bool, get_encoder_values, (double &left, double &right, double &timestamp), (override));
};
