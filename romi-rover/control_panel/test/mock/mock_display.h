#include "gmock/gmock.h"
#include <IDisplay.h>

#ifndef __MOCK_DISPLAY_H
#define __MOCK_DISPLAY_H

class MockDisplay : public IDisplay
{
public:
        MOCK_METHOD(void, clear, ());
        MOCK_METHOD(void, setCursor, (int col, int row));
        MOCK_METHOD(void, print, (const char* s));        
};

#endif // __MOCK_DISPLAY_H
