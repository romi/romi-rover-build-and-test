#include "gmock/gmock.h"
#include "IRPCHandler.h"
        
class MockRPCHandler : public rcom::IRPCHandler
{
public:
        MOCK_METHOD(void, execute, (JSON &command, JSON &result), (override));
}
