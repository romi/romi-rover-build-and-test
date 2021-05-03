#include "gmock/gmock.h"
#include "api/EventTimer.h"

class MockEventTimer : public romi::EventTimer
{
public:
        MOCK_METHOD(int, get_next_event, (), (override));
        MOCK_METHOD(void, set_timeout, (double timeout), (override));
        MOCK_METHOD(void, reset, (), (override));
};
