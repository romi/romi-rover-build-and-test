#include "gmock/gmock.h"
#include "IRPCClient.h"
        
class MockRPCClient : public rcom::IRPCClient
{
public:
        MOCK_METHOD(void, execute, (JSON &command, JSON &result), (override));
        MOCK_METHOD(bool, is_status_ok, (JSON &result), (override));
        MOCK_METHOD(const char *, get_error_message, (JSON &result), (override));
};
