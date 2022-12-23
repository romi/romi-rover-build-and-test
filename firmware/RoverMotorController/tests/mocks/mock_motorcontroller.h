#include "gmock/gmock.h"
#include "IMotorController.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
class MockMotorController : public IMotorController
{
public:
        MOCK_METHOD(void, setup, (), (override));
        MOCK_METHOD(bool, configure, (MotorControllerConfiguration& config), (override));
        MOCK_METHOD(bool, enable, (), (override));
        MOCK_METHOD(bool, disable, (), (override));
        MOCK_METHOD(void, stop, (), (override));
        MOCK_METHOD(bool, set_target_speeds, (int16_t left, int16_t right), (override));
        MOCK_METHOD(void, get_target_speeds, (int16_t& left, int16_t& right), (override));
        MOCK_METHOD(void, get_current_speeds, (int16_t& left, int16_t& right), (override));
        MOCK_METHOD(void, get_measured_speeds, (int16_t& left, int16_t& right), (override));
        MOCK_METHOD(bool, update, (), (override));
        MOCK_METHOD(void, get_encoders, (int32_t& left, int32_t& right, uint32_t& time), (override));
        MOCK_METHOD(PIController&, left_controller, (), (override));
        MOCK_METHOD(PIController&, right_controller, (), (override));
        MOCK_METHOD(SpeedEnvelope&, left_speed_envelope, (), (override));
        MOCK_METHOD(SpeedEnvelope&, right_speed_envelope, (), (override));
};
#pragma GCC diagnostic pop
