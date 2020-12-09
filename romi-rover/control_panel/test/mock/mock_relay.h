#include "gmock/gmock.h"
#include <IRelay.h>

#ifndef __MOCK_RELAY_H
#define __MOCK_RELAY_H

class MockRelay : public IRelay
{
public:
        MOCK_METHOD(void, open, ());
        MOCK_METHOD(void, close, ());
};

#endif // __MOCK_RELAY_H
