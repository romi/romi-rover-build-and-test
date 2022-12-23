#include "gmock/gmock.h"
#include <IRomiSerial.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
class MockRomiSerial : public romiserial::IRomiSerial
{
public:
        MOCK_METHOD(void, set_handlers, (const romiserial::MessageHandler *handlers, uint8_t num_handlers), (override));
        MOCK_METHOD(void, handle_input, (), (override));
        MOCK_METHOD(void, send_ok, (), (override));
        MOCK_METHOD(void, send_error, (int code, const char *message), (override));
        MOCK_METHOD(void, send, (const char *message), (override));
        MOCK_METHOD(void, log, (const char *message), (override));
};
#pragma GCC diagnostic pop
