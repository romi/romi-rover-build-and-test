#include "gmock/gmock.h"
#include <IStateTransition.h>

#ifndef __MOCK_STATE_TRANSITION_H
#define __MOCK_STATE_TRANSITION_H

class MockStateTransition : public IStateTransition
{
public:
        MOCK_METHOD(void, doTransition, (unsigned long t), (override));
        MOCK_METHOD(int16_t, event, (), (override));
        MOCK_METHOD(int8_t, state, (), (override));
        MOCK_METHOD(int8_t, nextState, (), (override));
};

#endif // __MOCK_STATE_TRANSITION_H
