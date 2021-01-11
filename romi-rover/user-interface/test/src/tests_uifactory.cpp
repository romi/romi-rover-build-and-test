#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../mock/mock_navigation.h"
#include "GetOpt.h"
#include "UIFactory.h"
#include "FakeDisplay.h"
#include "CrystalDisplay.h"
#include "FakeNavigation.h"
#include "rpc/RemoteNavigation.h"
#include "FakeInputDevice.h"
#include "JoystickInputDevice.h"

using namespace std;
using namespace testing;
using namespace romi;

class uifactory_tests : public ::testing::Test
{
protected:
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
        Option option = { "display-classname", true, FakeDisplay::ClassName, "" };
        GetOpt options(&option, 1);
        
        JsonCpp config = JsonCpp::parse("{'user-interface': {'display-classname': 'none'}}");
        
        try {
                UIFactory factory;
                factory.create_display(options, config);
                
        } catch (...) {
                FAIL() << "Expected successful creation";
        }
}

TEST_F(uifactory_tests, create_display_uses_config_second)
{
        GetOpt options(0, 0);
        JsonCpp config = JsonCpp::construct("{'user-interface': {'display-classname': '%s'}}",
                                         FakeDisplay::ClassName);
        
        try {
                UIFactory factory;
                factory.create_display(options, config);
                
        } catch (...) {
                FAIL() << "Expected successful creation";
        }
}

TEST_F(uifactory_tests, throws_exception_on_unknown_display_classname_1)
{
        GetOpt options(0, 0);
        JsonCpp config = JsonCpp::parse("{}");
        
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
        Option option = { "display-classname", true, "foo", "" };
        GetOpt options(&option, 1);
        JsonCpp config = JsonCpp::parse("{}");
        
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
        GetOpt options(0, 0);
        JsonCpp config = JsonCpp::parse("{'user-interface': {'display-classname': 'foo'}}");
        
        try {
                UIFactory factory;
                factory.create_display(options, config);
                FAIL() << "Expected an exception";
                
        } catch (...) {
                // NOP
        }
}

TEST_F(uifactory_tests, throws_exception_on_missing_crystal_display_device)
{
        GetOpt options(0, 0);
        JsonCpp config = JsonCpp::construct("{'user-interface': "
                                            "{'display-classname': '%s'}}",
                                            CrystalDisplay::ClassName);
        
        try {
                UIFactory factory;
                factory.create_display(options, config);
                FAIL() << "Expected an exception";
                
        } catch (...) {
                // NOP
        }
}

TEST_F(uifactory_tests, throws_exception_on_invalid_crystal_display_device_1)
{
        Option option = { "display-device", true, "/foo/bar", "" };
        GetOpt options(&option, 1);
        JsonCpp config = JsonCpp::construct("{'user-interface': "
                                            "{'display-classname': '%s'}}",
                                            CrystalDisplay::ClassName);
        
        try {
                UIFactory factory;
                factory.create_display(options, config);
                FAIL() << "Expected an exception";
                
        } catch (...) {
                // NOP
        }
}

TEST_F(uifactory_tests, throws_exception_on_invalid_crystal_display_device_2)
{
        GetOpt options(0, 0);
        JsonCpp config = JsonCpp::construct("{'user-interface': "
                                            "{'display-classname': '%s'}, "
                                            "'ports': {'crystal-display': "
                                            "{'port': '/foo/bar'}}}",
                                            CrystalDisplay::ClassName);
        
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
        Option option = { "navigation-classname", true, FakeNavigation::ClassName, "" };
        GetOpt options(&option, 1);
        JsonCpp config = JsonCpp::parse("{'user-interface': {'navigation-classname': 'none'}}");
        
        try {
                UIFactory factory;
                factory.create_navigation(options, config);
                
        } catch (...) {
                FAIL() << "Expected successful creation";
        }
}

TEST_F(uifactory_tests, create_navigation_uses_config_second)
{
        GetOpt options(0, 0);
        JsonCpp config = JsonCpp::construct("{'user-interface': "
                                            "{'navigation-classname': '%s'}}",
                                            FakeNavigation::ClassName);
        
        try {
                UIFactory factory;
                factory.create_navigation(options, config);
                
        } catch (...) {
                FAIL() << "Expected successful creation";
        }
}

TEST_F(uifactory_tests, throws_exception_on_unknown_navigation_type_1)
{
        GetOpt options(0, 0);
        JsonCpp config = JsonCpp::parse("{'user-interface': {}}");
        
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
        Option option = { "navigation-classname", true, "foo", "" };
        GetOpt options(&option, 1);
        JsonCpp config = JsonCpp::parse("{'user-interface': {}}");
        
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
        GetOpt options(0, 0);
        JsonCpp config = JsonCpp::parse("{'user-interface': "
                                        "{'navigation-classname': 'foo'}}");
        
        try {
                UIFactory factory;
                factory.create_navigation(options, config);
                FAIL() << "Expected an exception";
                
        } catch (...) {
                // NOP
        }
}

TEST_F(uifactory_tests, throws_exception_on_unknown_remote_navigation_server_1)
{
        GetOpt options(0, 0);
        JsonCpp config = JsonCpp::construct("{'user-interface': "
                                            "{'navigation-classname': '%s'}}",
                                            RemoteNavigation::ClassName);
        
        try {
                UIFactory factory;
                factory.create_navigation(options, config);
                FAIL() << "Expected an exception";
                
        } catch (...) {
                // NOP
        }
}


TEST_F(uifactory_tests, create_input_device_uses_options_first)
{
        Option option = { "input-device-classname", true, FakeInputDevice::ClassName, "" };
        GetOpt options(&option, 1);
        JsonCpp config = JsonCpp::parse("{'user-interface': "
                                        "{'input-device-classname': 'none'}}");
        
        try {
                UIFactory factory;
                factory.create_input_device(options, config);
                
        } catch (...) {
                FAIL() << "Expected successful creation";
        }
}

TEST_F(uifactory_tests, create_input_device_uses_config_second)
{
        GetOpt options(0, 0);
        JsonCpp config = JsonCpp::construct("{'user-interface': "
                                            "{'input-device-classname': '%s'}}",
                                            FakeInputDevice::ClassName);
        
        try {
                UIFactory factory;
                factory.create_input_device(options, config);
                
        } catch (...) {
                FAIL() << "Expected successful creation";
        }
}

TEST_F(uifactory_tests, throws_exception_on_missing_input_type)
{
        GetOpt options(0, 0);
        JsonCpp config = JsonCpp::parse("{'user-interface': {}}");
        
        try {
                UIFactory factory;
                factory.create_input_device(options, config);
                FAIL() << "Expected an exception";
                
        } catch (...) {
                // NOP
        }
}

TEST_F(uifactory_tests, throws_exception_on_unknown_input_type)
{
        Option option = { "input-device-classname", true, "foo", "" };
        GetOpt options(&option, 1);
        JsonCpp config = JsonCpp::parse("{'user-interface': {}}");
        
        try {
                UIFactory factory;
                factory.create_input_device(options, config);
                FAIL() << "Expected an exception";
                
        } catch (...) {
                // NOP
        }
}

TEST_F(uifactory_tests, throws_exception_on_unknown_joystick_device)
{
        Option option = { "input-device-classname", true, JoystickInputDevice::ClassName, "" };
        GetOpt options(&option, 1);
        JsonCpp config = JsonCpp::parse("{'user-interface': {}}");
        
        try {
                UIFactory factory;
                factory.create_input_device(options, config);
                FAIL() << "Expected an exception";
                
        } catch (...) {
                // NOP
        }
}

TEST_F(uifactory_tests, throws_exception_on_invalid_joystick_device)
{
        Option option = {"input-device-classname",true, JoystickInputDevice::ClassName, ""};
        GetOpt options(&option, 1);
        JsonCpp config = JsonCpp::parse("{'user-interface': {}, 'ports': "
                                        "{'joystick':{'port':'/foo/bar'}}}");
        
        try {
                UIFactory factory;
                factory.create_input_device(options, config);
                FAIL() << "Expected an exception";
                
        } catch (...) {
                // NOP
        }
}

TEST_F(uifactory_tests, successfully_uses_script_file_from_options)
{
        Option option = { "script-file", true, "/foo/bar", "" };
        GetOpt options(&option, 1);
        JsonCpp config = JsonCpp::parse("{'user-interface': {}}");
        
        try {
                UIFactory factory;
                const char *path = factory.get_script_file(options, config);

                ASSERT_STREQ(path, "/foo/bar");
                
        } catch (...) {
                FAIL() << "Didn't expect an exception";
        }
}

TEST_F(uifactory_tests, successfully_uses_script_file_from_config)
{
        GetOpt options(0, 0);
        JsonCpp config = JsonCpp::parse("{'user-interface': {'rover-script-engine': "
                                        "{'script-file':'/bar/foo'}}}");
        
        try {
                UIFactory factory;
                const char *path = factory.get_script_file(options, config);

                ASSERT_STREQ(path, "/bar/foo");
                
        } catch (...) {
                FAIL() << "Didn't expect an exception";
        }
}

TEST_F(uifactory_tests, throws_exception_on_missing_script_file)
{
        GetOpt options(0, 0);
        JsonCpp config = JsonCpp::parse("{'user-interface': {}}");
        
        try {
                UIFactory factory;
                factory.get_script_file(options, config);

                FAIL() << "Expected an exception";
                
        } catch (std::runtime_error& e) {
                // OK
                
        } catch (...) {
                FAIL() << "Expected a runtime exception";
        }
}
