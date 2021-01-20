#include "gmock/gmock.h"
#include "api/Notifications.h"

class MockNotifications : public romi::Notifications
{
public:
        MOCK_METHOD(void, notify, (const char *name), (override));
        MOCK_METHOD(void, stop, (const char *name), (override));
        MOCK_METHOD(void, reset, (), (override));
};
