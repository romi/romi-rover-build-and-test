#include "gmock/gmock.h"
#include "IDisplay.h"

class MockDisplay : public romi::IDisplay
{
public:
        MOCK_METHOD(bool, show, (int line, const char *s), (override));
        MOCK_METHOD(bool, clear, (int line), (override));
        MOCK_METHOD(int, count_lines, (), (override));
};
