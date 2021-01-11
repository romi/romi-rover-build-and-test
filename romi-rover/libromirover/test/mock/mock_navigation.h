#include "gmock/gmock.h"
#include "Navigation.h"

class MockNavigation : public romi::Navigation
{
public:
        MOCK_METHOD(bool, moveat, (double left, double right), (override));
        MOCK_METHOD(bool, move, (double distance, double speed), (override));
        MOCK_METHOD(bool, stop, (), (override));
        MOCK_METHOD(bool, pause_activity, (), (override));
        MOCK_METHOD(bool, continue_activity, (), (override));
        MOCK_METHOD(bool, reset_activity, (), (override));
};
