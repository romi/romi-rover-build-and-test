#include "gmock/gmock.h"
#include "Display.h"

class MockDisplay : public romi::Display
{
public:
        MOCK_METHOD(bool, show, (int line, const char *s), (override));
        MOCK_METHOD(bool, clear, (int line), (override));
        MOCK_METHOD(int, count_lines, (), (override));
};
