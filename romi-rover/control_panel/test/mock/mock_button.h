#include "gmock/gmock.h"
#include <IButton.h>

#ifndef __MOCK_BUTTON_H
#define __MOCK_BUTTON_H

class MockButton : public IButton
{
public:
        MOCK_METHOD(uint8_t, id, ());
        MOCK_METHOD(uint8_t, update, (unsigned long t));
};

#endif // __MOCK_BUTTON_H
