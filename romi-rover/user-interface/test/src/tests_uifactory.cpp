#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../mock/mock_navigation.h"
#include "UIOptions.h"
#include "UIFactory.h"
#include "FakeDisplay.h"
#include "FakeNavigation.h"
#include "JSONConfiguration.h"

using namespace std;
using namespace testing;
using namespace romi;

class uifactory_tests : public ::testing::Test
{
protected:
        UIOptions options;
        
	uifactory_tests() {
	}

	~uifactory_tests() override = default;

	void SetUp() override {
	}

	void TearDown() override {
	}
};

TEST_F(uifactory_tests, create_display_uses_options_first)
{
        JsonCpp config = JsonCpp::parse("{'user-interface': {'display-classname': 'none'}}");
        options.display_classname = FakeDisplay::ClassName;
        
        try {
                UIFactory factory;
                factory.create_display(options, config);
                
        } catch (...) {
                FAIL() << "Expected successful creation";
        }
}

TEST_F(uifactory_tests, create_display_uses_config_second)
{
        JsonCpp config = JsonCpp::construct("{'user-interface': {'display-classname': '%s'}}",
                                         FakeDisplay::ClassName);
        options.display_classname = 0;
        
        try {
                UIFactory factory;
                factory.create_display(options, config);
                
        } catch (...) {
                FAIL() << "Expected successful creation";
        }
}

TEST_F(uifactory_tests, throws_exception_on_unknown_display_classname_1)
{
        JsonCpp config = JsonCpp::parse("{}");
        options.display_classname = 0;
        
        try {
                UIFactory factory;
                factory.create_display(options, config);
                FAIL() << "Expected an exception";
                
        } catch (...) {
                // NOP
        }
}

TEST_F(uifactory_tests, throws_exception_on_unknown_display_classname_2)
{
        JsonCpp config = JsonCpp::parse("{}");
        options.display_classname = "foo";
        
        try {
                UIFactory factory;
                factory.create_display(options, config);
                FAIL() << "Expected an exception";
                
        } catch (...) {
                // NOP
        }
}

TEST_F(uifactory_tests, throws_exception_on_unknown_display_classname_3)
{
        JsonCpp config = JsonCpp::parse("{'display-classname': 'foo'}");
        options.display_classname = 0;
        
        try {
                UIFactory factory;
                factory.create_display(options, config);
                FAIL() << "Expected an exception";
                
        } catch (...) {
                // NOP
        }
}

TEST_F(uifactory_tests, create_navigation_uses_options_first)
{
        JsonCpp config = JsonCpp::parse("{'navigation-classname': 'none'}");
        options.navigation_classname = FakeNavigation::ClassName;
        
        try {
                UIFactory factory;
                factory.create_navigation(options, config);
                
        } catch (...) {
                FAIL() << "Expected successful creation";
        }
}

TEST_F(uifactory_tests, create_navigation_uses_config_second)
{
        JsonCpp config = JsonCpp::construct("{'user-interface': {'navigation-classname': '%s'}}",
                                            FakeNavigation::ClassName);
        options.navigation_classname = 0;
        
        try {
                UIFactory factory;
                factory.create_navigation(options, config);
                
        } catch (...) {
                FAIL() << "Expected successful creation";
        }
}

TEST_F(uifactory_tests, throws_exception_on_unknown_navigation_type_1)
{
        JsonCpp config = JsonCpp::parse("{'user-interface': {}}");
        options.navigation_classname = 0;
        
        try {
                UIFactory factory;
                factory.create_navigation(options, config);
                FAIL() << "Expected an exception";
                
        } catch (...) {
                // NOP
        }
}

TEST_F(uifactory_tests, throws_exception_on_unknown_navigation_type_2)
{
        JsonCpp config = JsonCpp::parse("{'user-interface': {}}");
        options.navigation_classname = "foo";
        
        try {
                UIFactory factory;
                factory.create_navigation(options, config);
                FAIL() << "Expected an exception";
                
        } catch (...) {
                // NOP
        }
}

TEST_F(uifactory_tests, throws_exception_on_unknown_navigation_type_3)
{
        JsonCpp config = JsonCpp::parse("{'user-interface': {'navigation-classname': 'foo'}}");
        options.navigation_classname = 0;
        
        try {
                UIFactory factory;
                factory.create_navigation(options, config);
                FAIL() << "Expected an exception";
                
        } catch (...) {
                // NOP
        }
}
