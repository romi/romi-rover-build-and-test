#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "BrushMotorDriver.h"
#include "JSONConfiguration.h"
#include "ConfigurationFile.h"

using namespace std;
using namespace testing;
using namespace romi;

class configuration_tests : public ::testing::Test
{
protected:
	configuration_tests() {
	}

	~configuration_tests() override = default;

	void SetUp() override {
	}

	void TearDown() override {
	}
};

TEST_F(configuration_tests, test_json_configuration_get)
{
        JSONConfiguration config(JSON::parse("{'foo':'bar'}"));

        ASSERT_STREQ("bar", config.get("foo").str());
        
        JSON obj = config.get();
        ASSERT_EQ(true, obj.has("foo"));
        ASSERT_STREQ("bar", obj.get("foo").str());
}

TEST_F(configuration_tests, test_json_configuration_set)
{
        JSONConfiguration config;

        JSON o = JSON::parse("{'foo':'bar'}");
        config.set(o);
        
        ASSERT_STREQ("bar", config.get("foo").str());
        
        JSON obj = config.get();
        ASSERT_EQ(true, obj.has("foo"));
        ASSERT_STREQ("bar", obj.get("foo").str());
}

