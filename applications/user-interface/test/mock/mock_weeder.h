#include "gmock/gmock.h"
#include "api/Weeder.h"

class MockWeeder : public romi::Weeder
{
public:
        MOCK_METHOD(bool, hoe, (), (override));
};
