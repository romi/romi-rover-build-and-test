#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "OquamOptions.h"

using namespace std;
using namespace testing;
using namespace romi;

class oquamoptions_tests : public ::testing::Test
{
protected:
        
	oquamoptions_tests() {
        }

	~oquamoptions_tests() override = default;

	void SetUp() override {
        }

	void TearDown() override {}
};

TEST_F(oquamoptions_tests, test_constructor_default_values)
{
        OquamOptions options;

        EXPECT_TRUE(options.controller_classname == nullptr);
        EXPECT_TRUE(options.controller_device == nullptr);
        EXPECT_TRUE(options.config_file != nullptr);
        EXPECT_TRUE(options.server_name != nullptr);
}

TEST_F(oquamoptions_tests, parse_config_file)
{
        OquamOptions options;
        const char *arguments[] = { "program", "-C", "foo", 0 };
        optind = 1;
        
        options.parse(3, (char**) arguments);
        
        EXPECT_STREQ(options.config_file, "foo");
}

TEST_F(oquamoptions_tests, parse_controller_device)
{
        OquamOptions options;
        const char *arguments[] = { "program", "-D", "foo", 0 };
        optind = 1;
        
        options.parse(3, (char**) arguments);
        
        EXPECT_STREQ(options.controller_device, "foo");
}

TEST_F(oquamoptions_tests, parse_server_name)
{
        OquamOptions options;
        const char *arguments[] = { "program", "-N", "foo", 0 };
        optind = 1;
        
        options.parse(3, (char**) arguments);
        
        EXPECT_STREQ(options.server_name, "foo");
}

TEST_F(oquamoptions_tests, parse_controller_classname)
{
        OquamOptions options;
        const char *arguments[] = { "program", "-c", "foo", 0 };
        optind = 1;
        
        options.parse(3, (char**) arguments);
        
        EXPECT_STREQ(options.controller_classname, "foo");
}

TEST_F(oquamoptions_tests, parse_output_directory)
{
        OquamOptions options;
        const char *arguments[] = { "program", "-d", "foo", 0 };
        optind = 1;
        
        options.parse(3, (char**) arguments);
        
        EXPECT_STREQ(options.output_directory, "foo");
}


