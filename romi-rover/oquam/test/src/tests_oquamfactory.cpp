#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "OquamFactory.h"
#include "FakeCNCController.h"
#include "StepperController.h"
#include "GetOpt.h"

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
        GetOpt getopt(0, 0); 
        JsonCpp config = JsonCpp::parse("{'oquam': {}}");
        
        OquamFactory factory;

        try {
                factory.create_controller(getopt, config);
                FAIL() << "Expected a runtime_error";
                
        } catch (std::runtime_error& re) {
                // OK
        } catch (...) {
                FAIL() << "Expected a runtime_error";
        }
}

TEST_F(oquamfactory_tests, create_controller_fails_unknown_classname_1)
{
        static Option option = { "oquam-controller-classname", true, 0, "foo"};
        GetOpt getopt(&option, 1); 
        JsonCpp config = JsonCpp::parse("{'oquam': {}}");
        
        OquamFactory factory;

        try {
                factory.create_controller(getopt, config);
                FAIL() << "Expected a runtime_error";
                
        } catch (std::runtime_error& re) {
                // OK
        } catch (...) {
                FAIL() << "Expected a runtime_error";
        }
}

TEST_F(oquamfactory_tests, create_controller_fails_unknown_classname_2)
{
        GetOpt getopt(0, 0); 
        JsonCpp config = JsonCpp::parse("{'oquam': {'controller-classname': 'bar'}}");
        
        OquamFactory factory;

        try {
                factory.create_controller(getopt, config);
                FAIL() << "Expected a runtime_error";
                
        } catch (std::runtime_error& re) {
                // OK
        } catch (...) {
                FAIL() << "Expected a runtime_error";
        }
}

TEST_F(oquamfactory_tests, successfully_create_fake_controller_1)
{
        static Option option = { "oquam-controller-classname", true,
                                 FakeCNCController::ClassName, "" };
        GetOpt getopt(&option, 1); 
        JsonCpp config = JsonCpp::parse("{'oquam': {}}");
        
        OquamFactory factory;

        try {
                factory.create_controller(getopt, config);
                
        } catch (std::runtime_error& re) {
                FAIL() << "Expected successful creation";
        } catch (...) {
                FAIL() << "Expected successful creation";
        }
}

TEST_F(oquamfactory_tests, successfully_create_fake_controller_2)
{
        GetOpt getopt(0, 0); 
        JsonCpp config = JsonCpp::construct("{'oquam': {'controller-classname': '%s'}}",
                                            FakeCNCController::ClassName);
        OquamFactory factory;

        try {
                factory.create_controller(getopt, config);
                
        } catch (std::runtime_error& re) {
                FAIL() << "Expected successful creation";
        } catch (...) {
                FAIL() << "Expected successful creation";
        }
}

TEST_F(oquamfactory_tests, create_stepper_controller_fails_when_missing_device)
{
        static Option option = { "oquam-controller-classname", true, 0,
                                 StepperController::ClassName };
        GetOpt getopt(&option, 1); 
        JsonCpp config = JsonCpp::parse("{'oquam': {}}");
        
        OquamFactory factory;

        try {
                factory.create_controller(getopt, config);
                FAIL() << "Expected a runtime_error";
                
        } catch (std::runtime_error& re) {
                // OK
        } catch (...) {
                FAIL() << "Expected a runtime_error";
        }
}

TEST_F(oquamfactory_tests, create_stepper_controller_fails_when_bad_device)
{
        static Option option = { "oquam-controller-classname", true, 0,
                                 StepperController::ClassName };
        GetOpt getopt(&option, 1); 
        JsonCpp config = JsonCpp::parse("{'ports':{'oquam':{'port':'/foo/bar'}}}");
        
        OquamFactory factory;

        try {
                factory.create_controller(getopt, config);
                FAIL() << "Expected a runtime_error";
                
        } catch (std::runtime_error& re) {
                // OK
        } catch (...) {
                FAIL() << "Expected a runtime_error";
        }
}


