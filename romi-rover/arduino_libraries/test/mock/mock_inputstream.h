#include "gmock/gmock.h"
#include "IInputStream.h"

#ifndef __MOCK_INPUTSTREAM_H
#define __MOCK_INPUTSTREAM_H

class MockInputStream : public IInputStream
{
public:
        MOCK_METHOD(int, available, (), (override));
        MOCK_METHOD(int, read, (), (override));
        MOCK_METHOD(void, set_timeout, (float seconds), (override));
};

#endif // __MOCK_INPUTSTREAM_H
