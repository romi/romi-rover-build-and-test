#include "gmock/gmock.h"
#include "api/INavigation.h"

class MockNavigation : public romi::INavigation
{
public:
        MOCK_METHOD(bool, moveat, (double left, double right), (override));
        MOCK_METHOD(bool, move, (double distance, double speed), (override));
        MOCK_METHOD(bool, stop, (), (override));
};
