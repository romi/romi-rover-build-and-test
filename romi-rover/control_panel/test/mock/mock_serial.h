#include "gmock/gmock.h"
#include <ISerial.h>

#ifndef __MOCK_SERIAL_H
#define __MOCK_SERIAL_H

class MockSerial : public ISerial
{
public:
        MOCK_METHOD(void, init, (long baudrate));
        MOCK_METHOD(int, available, ());
        MOCK_METHOD(int, read, ());
        MOCK_METHOD(void, print, (const char *s));
        MOCK_METHOD(void, println, (const char *s));
};

#endif // __MOCK_SERIAL_H
