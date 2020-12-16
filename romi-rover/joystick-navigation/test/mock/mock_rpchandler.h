#include "gmock/gmock.h"
#include "IRPCHandler.h"
        
class MockRPCHandler : public rcom::IRPCHandler
{
public:
        MOCK_METHOD(void, execute, (const char *method, JSON &params, JSON &result, rcom::RPCError &error), (override));
};
