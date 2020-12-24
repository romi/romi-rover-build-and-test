#include "gmock/gmock.h"
#include "ScriptEngine.h"

class MockScriptEngine : public romi::ScriptEngine
{
        MOCK_METHOD(int, get_next_event, (), (override));
        MOCK_METHOD(int, count_scripts, (), (override));
        MOCK_METHOD(void, get_script, (int i, std::string& id, std::string& name), (override));
        MOCK_METHOD(void, execute_script, (std::string& id), (override));
};
