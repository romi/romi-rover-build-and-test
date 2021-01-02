#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "OquamFactory.h"
#include "FakeCNCController.h"
#include "StepperController.h"

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
        OquamOptions options;
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
        OquamOptions options;
        options.controller_classname = "foo";
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
        OquamOptions options;
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

TEST_F(oquamfactory_tests, successfullt_create_fake_controller_1)
{
        OquamOptions options;
        options.controller_classname = FakeCNCController::ClassName;
        JsonCpp config = JsonCpp::parse("{'oquam': {}}");
        
        OquamFactory factory;

        try {
                factory.create_controller(options, config);
                
        } catch (std::runtime_error& re) {
                FAIL() << "Expected successful creation";
        } catch (...) {
                FAIL() << "Expected successful creation";
        }
}

TEST_F(oquamfactory_tests, successfullt_create_fake_controller_2)
{
        OquamOptions options;
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
        OquamOptions options;
        options.controller_classname = StepperController::ClassName;
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
        OquamOptions options;
        options.controller_classname = StepperController::ClassName;
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


