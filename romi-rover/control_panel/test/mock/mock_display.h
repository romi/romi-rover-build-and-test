#include "gmock/gmock.h"
#include <IDisplay.h>

#ifndef __MOCK_DISPLAY_H
#define __MOCK_DISPLAY_H

class MockDisplay : public IDisplay
{
public:
        MOCK_METHOD(void, showState, (const char* s));        
        MOCK_METHOD(void, showMenu, (const char* s));        
        MOCK_METHOD(void, clearMenu, ());
};

#endif // __MOCK_DISPLAY_H
