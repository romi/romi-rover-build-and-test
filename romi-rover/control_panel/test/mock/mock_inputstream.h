#include "gmock/gmock.h"
#include "IInputStream.h"

#ifndef __MOCK_INPUTSTREAM_H
#define __MOCK_INPUTSTREAM_H

class MockInputStream : public IInputStream
{
public:
        MOCK_METHOD(int, available, ());
        MOCK_METHOD(int, read, ());
};

#endif // __MOCK_INPUTSTREAM_H
