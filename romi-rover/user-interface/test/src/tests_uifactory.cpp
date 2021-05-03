#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../mock/mock_navigation.h"
#include <configuration/GetOpt.h>
#include <ui/CrystalDisplay.h>
#include "configuration/ConfigurationProvider.h"
#include <rpc/RemoteNavigation.h>
#include <ui/JoystickInputDevice.h>
#include <rover/RoverOptions.h>
#include <fake/FakeDisplay.h>
#include <fake/FakeInputDevice.h>
#include <fake/FakeNavigation.h>

#include "UIFactory.h"

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

TEST_F(uifactory_tests, create_display_uses_config)
{
        std::vector<Option> options_list;
        GetOpt options(options_list);
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
        std::vector<Option> options_list;
        GetOpt options(options_list);
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
        std::vector<Option> options_list;
        GetOpt options(options_list);
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
        std::vector<Option> options_list;
        GetOpt options(options_list);
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
        std::vector<Option> options_list = { {"display-device", true, "/foo/bar", ""} };
        GetOpt options(options_list);
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
        std::vector<Option> options_list;
        GetOpt options(options_list);
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

TEST_F(uifactory_tests, create_navigation_uses_config)
{
        std::vector<Option> options_list;
        GetOpt options(options_list);
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
        std::vector<Option> options_list;
        GetOpt options(options_list);
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
        std::vector<Option> options_list;
        GetOpt options(options_list);
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
        std::vector<Option> options_list;
        GetOpt options(options_list);
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

TEST_F(uifactory_tests, create_input_device_uses_config)
{
        std::vector<Option> options_list;
        GetOpt options(options_list);
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
        std::vector<Option> options_list;
        GetOpt options(options_list);
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
        std::vector<Option> options_list;
        GetOpt options(options_list);
        JsonCpp config = JsonCpp::parse("{'user-interface': {'input-device-classname':'foo'}}");
        
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
        std::vector<Option> options_list = {{ "input-device-classname", true, JoystickInputDevice::ClassName, "" }};
        GetOpt options(options_list);
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
        std::vector<Option> options_list = {{"input-device-classname",true, JoystickInputDevice::ClassName, ""}};
        GetOpt options(options_list);
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
        std::vector<Option> options_list = {{ RoverOptions::script, true, "/foo/bar", "" }};
        GetOpt options(options_list);
        JsonCpp config = JsonCpp::parse("{'user-interface': {}}");
        
        try {
              //  UIFactory factory;
                std::string path = get_script_file(options, config);

                ASSERT_EQ(path, "/foo/bar");
                
        } catch (...) {
                FAIL() << "Didn't expect an exception";
        }
}

TEST_F(uifactory_tests, successfully_uses_script_file_from_config)
{
        std::vector<Option> options_list;
        GetOpt options(options_list);
        JsonCpp config = JsonCpp::construct("{'user-interface': {'script-engine': "
                                            "{'%s':'/bar/foo'}}}",
                                            RoverOptions::script);
        
        try {
               // UIFactory factory;
                std::string path = get_script_file(options, config);

                ASSERT_EQ(path, "/bar/foo");
                
        } catch (...) {
                FAIL() << "Didn't expect an exception";
        }
}

TEST_F(uifactory_tests, throws_exception_on_missing_script_file)
{
        std::vector<Option> options_list;
        GetOpt options(options_list);
        JsonCpp config = JsonCpp::parse("{'user-interface': {}}");
        
        try {
//                UIFactory factory;
                get_script_file(options, config);

                FAIL() << "Expected an exception";
                
        } catch (std::runtime_error& e) {
                // OK
                
        } catch (...) {
                FAIL() << "Expected a runtime exception";
        }
}
