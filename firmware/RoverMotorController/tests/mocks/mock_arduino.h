#include "gmock/gmock.h"
#include "IArduino.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
class MockArduino : public IArduino
{
public:
        MOCK_METHOD(IEncoder&, left_encoder, (), (override));
        MOCK_METHOD(IEncoder&, right_encoder, (), (override));
        MOCK_METHOD(IPWM&, left_pwm, (), (override));
        MOCK_METHOD(IPWM&, right_pwm, (), (override));
        MOCK_METHOD(uint32_t, milliseconds, (), (override));
        MOCK_METHOD(void, init_encoders,(uint16_t encoder_steps,
                                         int8_t left_increment,
                                         int8_t right_increment), (override));
};
#pragma GCC diagnostic pop
