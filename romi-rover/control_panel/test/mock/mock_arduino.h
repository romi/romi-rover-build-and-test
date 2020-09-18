#include "gmock/gmock.h"
#include <IArduino.h>

#ifndef __MOCK_ARDUINO_H
#define __MOCK_ARDUINO_H

class MockArduino : public IArduino
{
public:
        MOCK_METHOD(unsigned long, millis, (), (override));
        MOCK_METHOD(int, analogRead, (int pin), (override));
        MOCK_METHOD(void, digitalWrite, (int pin, int high_low), (override));
        MOCK_METHOD(void, pinMode, (uint8_t pin, uint8_t mode), (override));
};

#endif // __MOCK_ARDUINO_H
