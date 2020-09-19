#include "gmock/gmock.h"
#include <IEventTimer.h>

#ifndef __EVENT_BUTTON_H
#define __EVENT_BUTTON_H

class MockTimer : public IEventTimer
{
public:
        MOCK_METHOD(void, setTimeout, (unsigned long milliseconds, int event));
        MOCK_METHOD(int16_t, update, (unsigned long t));
};

#endif // __EVENT_BUTTON_H
