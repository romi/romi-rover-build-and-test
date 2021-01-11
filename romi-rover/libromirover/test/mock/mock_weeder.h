#include "gmock/gmock.h"
#include "Weeder.h"

class MockWeeder : public romi::Weeder
{
public:
        MOCK_METHOD(bool, hoe, (), (override));
        MOCK_METHOD(bool, stop, (), (override));
        MOCK_METHOD(bool, pause_activity, (), (override));
        MOCK_METHOD(bool, continue_activity, (), (override));
        MOCK_METHOD(bool, reset_activity, (), (override));
};
