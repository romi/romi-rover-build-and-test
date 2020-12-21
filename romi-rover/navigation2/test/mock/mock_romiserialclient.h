#include "gmock/gmock.h"
#include "IRomiSerialClient.h"

class MockRomiSerialClient : public IRomiSerialClient
{
public:
        MOCK_METHOD(void, send, (const char *request, JsonCpp& response), (override));
};
