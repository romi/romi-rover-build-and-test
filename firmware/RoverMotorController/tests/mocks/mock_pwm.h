#include "gmock/gmock.h"
#include "IPWM.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
class MockPWM : public IPWM
{
public:
        MOCK_METHOD(int16_t, center, (), (override));
        MOCK_METHOD(int16_t, amplitude, (), (override));
        MOCK_METHOD(void, set, (int16_t pulse_width), (override));
};
#pragma GCC diagnostic pop
