#include "gmock/gmock.h"
#include "IOutputStream.h"

#ifndef __MOCK_OUTPUTSTREAM_H
#define __MOCK_OUTPUTSTREAM_H

class MockOutputStream : public IOutputStream
{
public:
        MOCK_METHOD(size_t, write, (char c), (override));
        MOCK_METHOD(size_t, print, (const char *s), (override));
        MOCK_METHOD(size_t, println, (const char *s), (override));
};

#endif // __MOCK_OUTPUTSTREAM_H
