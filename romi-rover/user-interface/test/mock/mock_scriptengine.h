#include "gmock/gmock.h"
#include "ScriptEngine.h"
#include "Rover.h"

class MockScriptEngine : public romi::ScriptEngine<romi::Rover>
{
        MOCK_METHOD(int, get_next_event, (), (override));
        MOCK_METHOD(void, execute_script, (romi::Rover& target, int index), (override));
};
