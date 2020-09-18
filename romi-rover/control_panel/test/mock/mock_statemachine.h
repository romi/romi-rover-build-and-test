#include "gmock/gmock.h"
#include <IStateMachine.h>

#ifndef __MOCK_STATE_MACHINE_H
#define __MOCK_STATE_MACHINE_H

class MockStateMachine : public IStateMachine
{
public:
        MOCK_METHOD(int, getState, (), (override));
        MOCK_METHOD(int, getError, (), (override));
        MOCK_METHOD(void, add, (IStateTransition *transition), (override));
        MOCK_METHOD(int, handleEvent, (int event), (override));
};

#endif // __MOCK_STATE_MACHINE_H
