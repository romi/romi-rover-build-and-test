#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "GetOpt.h"

using namespace std;
using namespace testing;
using namespace romi;

class roveroptions_tests : public ::testing::Test
{
protected:
        
	roveroptions_tests() {
        }

	~roveroptions_tests() override = default;

	void SetUp() override {
        }

	void TearDown() override {}
};

TEST_F(roveroptions_tests, test_parse)
{
        Option list[] = {
                { "help", false, 0, "Print help message" },
                { "config", true, 0, "Path of the config file" },
        };
        GetOpt options(list, 2);

        const char *argv[] = { "app", "--help", "--config", "config.json", 0 };

        options.parse(4, (char**) argv);

        ASSERT_EQ(options.is_help_requested(), true);
        ASSERT_STREQ(options.get_value("config"), "config.json");
}

TEST_F(roveroptions_tests, test_print_usage)
{
        Option list[] = {
                { "help", false, 0, "Print help message" },
                { "config", true, 0, "Path of the config file" },
        };
        GetOpt options(list, 2);
        options.print_usage();
}

