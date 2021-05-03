#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "OquamFactory.h"
#include "configuration/GetOpt.h"

#include "oquam/FakeCNCController.h"
#include "oquam/StepperController.h"

using namespace std;
using namespace testing;
using namespace romi;

class oquamfactory_tests : public ::testing::Test
{
protected:
        
	oquamfactory_tests() {
        }

	~oquamfactory_tests() override = default;

	void SetUp() override {
        }

	void TearDown() override {}
};

TEST_F(oquamfactory_tests, create_controller_fails_when_missing_classname)
{
        std::vector<Option> options_list;
        GetOpt options(options_list);
        JsonCpp config = JsonCpp::parse("{'oquam': {}}");
        
        OquamFactory factory;

        try {
                factory.create_controller(options, config);
                FAIL() << "Expected a runtime_error";
                
        } catch (std::runtime_error& re) {
                // OK
        } catch (...) {
                FAIL() << "Expected a runtime_error";
        }
}

TEST_F(oquamfactory_tests, create_controller_fails_unknown_classname_1)
{
        std::vector<Option> options_list = {{ "oquam-controller-classname", true, nullptr, "foo"}};
        GetOpt options(options_list);
        JsonCpp config = JsonCpp::parse("{'oquam': {}}");
        
        OquamFactory factory;

        try {
                factory.create_controller(options, config);
                FAIL() << "Expected a runtime_error";
                
        } catch (std::runtime_error& re) {
                // OK
        } catch (...) {
                FAIL() << "Expected a runtime_error";
        }
}

TEST_F(oquamfactory_tests, create_controller_fails_unknown_classname_2)
{
        std::vector<Option> options_list;
        GetOpt options(options_list);
        JsonCpp config = JsonCpp::parse("{'oquam': {'controller-classname': 'bar'}}");
        
        OquamFactory factory;

        try {
                factory.create_controller(options, config);
                FAIL() << "Expected a runtime_error";
                
        } catch (std::runtime_error& re) {
                // OK
        } catch (...) {
                FAIL() << "Expected a runtime_error";
        }
}

TEST_F(oquamfactory_tests, successfully_create_fake_controller)
{
        std::vector<Option> options_list;
        GetOpt options(options_list);
        JsonCpp config = JsonCpp::construct("{'oquam': {'controller-classname': '%s'}}",
                                            FakeCNCController::ClassName);
        OquamFactory factory;

        try {
                factory.create_controller(options, config);
                
        } catch (std::runtime_error& re) {
                FAIL() << "Expected successful creation";
        } catch (...) {
                FAIL() << "Expected successful creation";
        }
}

TEST_F(oquamfactory_tests, create_stepper_controller_fails_when_missing_device)
{
        std::vector<Option> options_list = {{ "oquam-controller-classname", true, nullptr,
                                 StepperController::ClassName }};
        GetOpt options(options_list);
        JsonCpp config = JsonCpp::parse("{'oquam': {}}");
        
        OquamFactory factory;

        try {
                factory.create_controller(options, config);
                FAIL() << "Expected a runtime_error";
                
        } catch (std::runtime_error& re) {
                // OK
        } catch (...) {
                FAIL() << "Expected a runtime_error";
        }
}

TEST_F(oquamfactory_tests, create_stepper_controller_fails_when_bad_device)
{
        std::vector<Option> options_list = {{ "oquam-controller-classname", true, nullptr,
                                 StepperController::ClassName }};
        GetOpt options(options_list);
        JsonCpp config = JsonCpp::parse("{'ports':{'oquam':{'port':'/foo/bar'}}}");
        
        OquamFactory factory;

        try {
                factory.create_controller(options, config);
                FAIL() << "Expected a runtime_error";
                
        } catch (std::runtime_error& re) {
                // OK
        } catch (...) {
                FAIL() << "Expected a runtime_error";
        }
}


