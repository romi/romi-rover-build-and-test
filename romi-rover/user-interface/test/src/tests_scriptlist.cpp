#include <stdio.h>
#include <unistd.h>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ScriptList.h"

using namespace std;
using namespace testing;
using namespace romi;

class scriptlist_tests : public ::testing::Test
{
protected:
        
	scriptlist_tests() {
	}

	~scriptlist_tests() override = default;

	void SetUp() override {
	}

	void TearDown() override {
	}
};

TEST_F(scriptlist_tests, successfully_load_empty_scriplist)
{
        JsonCpp json = JsonCpp::parse("[]");
        ScriptList scripts(json);

        ASSERT_EQ(scripts.size(), 0);
}

TEST_F(scriptlist_tests, successfully_load_simple_scriplist)
{
        JsonCpp json = JsonCpp::parse("[{ 'id': 'foo', 'title': 'bar', 'actions': []}]");
        ScriptList scripts(json);

        ASSERT_EQ(scripts.size(), 1);
        ASSERT_STREQ(scripts[0].id.c_str(), "foo");
        ASSERT_STREQ(scripts[0].title.c_str(), "bar");
        ASSERT_EQ(scripts[0].actions.size(), 0);
}

TEST_F(scriptlist_tests, successfully_load_scriplist_with_hoe_action)
{
        JsonCpp json = JsonCpp::parse("[{ 'id': 'foo', 'title': 'bar', 'actions': [{'action':'hoe'}]}]");
        ScriptList scripts(json);

        ASSERT_EQ(scripts.size(), 1);
        ASSERT_STREQ(scripts[0].id.c_str(), "foo");
        ASSERT_STREQ(scripts[0].title.c_str(), "bar");
        ASSERT_EQ(scripts[0].actions.size(), 1);
        ASSERT_EQ(scripts[0].actions[0].type, romi::Action::Hoe);
}


TEST_F(scriptlist_tests, successfully_load_scriplist_with_move_action)
{
        JsonCpp json = JsonCpp::parse("[{ 'id': 'foo', 'title': 'bar', 'actions': [{'action':'move', 'distance': 1, 'speed': 0.5}]}]");
        ScriptList scripts(json);

        ASSERT_EQ(scripts.size(), 1);
        ASSERT_STREQ(scripts[0].id.c_str(), "foo");
        ASSERT_STREQ(scripts[0].title.c_str(), "bar");
        ASSERT_EQ(scripts[0].actions.size(), 1);
        ASSERT_EQ(scripts[0].actions[0].type, romi::Action::Move);
        ASSERT_EQ(scripts[0].actions[0].params[0], 1.0);
        ASSERT_EQ(scripts[0].actions[0].params[1], 0.5);
}

TEST_F(scriptlist_tests, successfully_load_scriplist_with_two_scripts)
{
        JsonCpp json = JsonCpp::parse("[{ 'id': 'foo', 'title': 'Foo', 'actions': [{'action':'move', 'distance': 1, 'speed': 0.5}]},"
                                      "{ 'id': 'bar', 'title': 'Bar', 'actions': [{'action':'hoe'}]}]");
        ScriptList scripts(json);

        ASSERT_EQ(scripts.size(), 2);
        
        ASSERT_STREQ(scripts[0].id.c_str(), "foo");
        ASSERT_STREQ(scripts[0].title.c_str(), "Foo");
        ASSERT_EQ(scripts[0].actions.size(), 1);
        ASSERT_EQ(scripts[0].actions[0].type, romi::Action::Move);
        ASSERT_EQ(scripts[0].actions[0].params[0], 1.0);
        ASSERT_EQ(scripts[0].actions[0].params[1], 0.5);
        
        ASSERT_STREQ(scripts[1].id.c_str(), "bar");
        ASSERT_STREQ(scripts[1].title.c_str(), "Bar");
        ASSERT_EQ(scripts[1].actions.size(), 1);
        ASSERT_EQ(scripts[1].actions[0].type, romi::Action::Hoe);
}

TEST_F(scriptlist_tests, throws_runtime_exception_on_invalid_move_1)
{
        JsonCpp json = JsonCpp::parse("[{ 'id': 'foo', 'title': 'bar', 'actions': [{'action':'move'}]}]");

        try {
                ScriptList scripts(json);
                FAIL() << "Expected a runtime exception";
                
        } catch (std::runtime_error& e) {
                // OK
                
        } catch (...) {
                FAIL() << "Expected a runtime exception";
        }
}

TEST_F(scriptlist_tests, throws_runtime_exception_on_invalid_move_2)
{
        JsonCpp json = JsonCpp::parse("[{ 'id': 'foo', 'title': 'bar', 'actions': [{'action':'move', 'distance': 1}]}]");

        try {
                ScriptList scripts(json);
                FAIL() << "Expected a runtime exception";
                
        } catch (std::runtime_error& e) {
                // OK
                
        } catch (...) {
                FAIL() << "Expected a runtime exception";
        }
}

TEST_F(scriptlist_tests, throws_runtime_exception_on_invalid_move_3)
{
        JsonCpp json = JsonCpp::parse("[{ 'id': 'foo', 'title': 'bar', 'actions': [{'action':'move', 'distance': 100000, 'speed': 0.5}]}]");

        try {
                ScriptList scripts(json);
                FAIL() << "Expected a runtime exception";
                
        } catch (std::runtime_error& e) {
                // OK
                
        } catch (...) {
                FAIL() << "Expected a runtime exception";
        }
}

TEST_F(scriptlist_tests, throws_runtime_exception_on_invalid_move_4)
{
        JsonCpp json = JsonCpp::parse("[{ 'id': 'foo', 'title': 'bar', 'actions': [{'action':'move', 'distance': 1, 'speed': 100}]}]");

        try {
                ScriptList scripts(json);
                FAIL() << "Expected a runtime exception";
                
        } catch (std::runtime_error& e) {
                // OK
                
        } catch (...) {
                FAIL() << "Expected a runtime exception";
        }
}

TEST_F(scriptlist_tests, throws_runtime_exception_on_missing_script_file)
{
        try {
                ScriptList scripts("/foo/bar");
                FAIL() << "Expected a runtime exception";
                
        } catch (std::runtime_error& e) {
                // OK
                
        } catch (...) {
                FAIL() << "Expected a runtime exception";
        }
}

TEST_F(scriptlist_tests, successfully_loads_a_scrip_file)
{
        FILE* fp = fopen("/tmp/scriptlist_tests.json", "w");
        const char *json = "[{ 'id': 'foo', 'title': 'bar', 'actions': [{'action':'move', 'distance': 1, 'speed': 0.5}]}]";
        fprintf(fp, "%s", json);
        fclose(fp);
        
        try {
                ScriptList scripts("/tmp/scriptlist_tests.json");
                
                ASSERT_EQ(scripts.size(), 1);
                ASSERT_STREQ(scripts[0].id.c_str(), "foo");
                ASSERT_STREQ(scripts[0].title.c_str(), "bar");
                ASSERT_EQ(scripts[0].actions.size(), 1);
                ASSERT_EQ(scripts[0].actions[0].type, romi::Action::Move);
                ASSERT_EQ(scripts[0].actions[0].params[0], 1.0);
                ASSERT_EQ(scripts[0].actions[0].params[1], 0.5);
                
        } catch (std::runtime_error& e) {
                FAIL() << "Expected a successful file load";
                
        } catch (...) {
                FAIL() << "Expected a successful file load";
        }

        unlink("/tmp/scriptlist_tests.json");
}

TEST_F(scriptlist_tests, throws_runtime_exception_on_invalid_script_file_1)
{
        FILE* fp = fopen("/tmp/scriptlist_tests.json", "w");
        const char *json = "[{ 'id': 'foo', 'title': 'bar', 'actions': [{'action':'move'}]}]";
        fprintf(fp, "%s", json);
        fclose(fp);
        
        try {
                ScriptList scripts("/tmp/scriptlist_tests.json");
                FAIL() << "Expected a runtime exception";
                
        } catch (std::runtime_error& e) {
                // OK
                
        } catch (...) {
                FAIL() << "Expected a runtime exception";
        }

        unlink("/tmp/scriptlist_tests.json");
}

TEST_F(scriptlist_tests, throws_runtime_exception_on_invalid_script_file_2)
{
        FILE* fp = fopen("/tmp/scriptlist_tests.json", "w");
        const char *json = "[{ 'id': 'foo', 'title': 'bar', 'actions': [{'action':'move', 'distance': 1, 'speed': 10}]}]";
        fprintf(fp, "%s", json);
        fclose(fp);
        
        try {
                ScriptList scripts("/tmp/scriptlist_tests.json");
                FAIL() << "Expected a runtime exception";
                
        } catch (std::runtime_error& e) {
                // OK
                
        } catch (...) {
                FAIL() << "Expected a runtime exception";
        }

        unlink("/tmp/scriptlist_tests.json");
}
                

