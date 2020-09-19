#include "gmock/gmock.h"
#include "IOutputStream.h"

#ifndef __MOCK_OUTPUTSTREAM_H
#define __MOCK_OUTPUTSTREAM_H

class MockOutputStream : public IOutputStream
{
public:
        MOCK_METHOD(void, print, (const char *s));
        MOCK_METHOD(void, println, (const char *s));
};

#endif // __MOCK_OUTPUTSTREAM_H
