#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "JsonCpp.h"
#include "oquam/SmoothPath.h"
#include "oquam/print.h"

using namespace std;
using namespace testing;
using namespace romi;

class print_tests : public ::testing::Test
{
protected:

        double vmax[3] = { 1.0, 1.0, 0.0};
        double amax[3] = { 1.0, 1.0, 1.0};
        double deviation = 0.01;
        double period = 0.100;
        double maxlen = 32.0;
        
	print_tests() {
	}

	~print_tests() override = default;

	void SetUp() override {
	}

	void TearDown() override {
	}
};

TEST_F(print_tests, test_valid_json_without_slices)
{
        // Arrange
        v3 start_position(0, 0, 0);
        SmoothPath script(start_position);
        script.moveto(0.10, 0.00, 0.0, 1.0);
        script.moveto(0.00, 0.00, 0.0, 1.0);
        script.convert(vmax, amax, deviation, period, maxlen);

        membuf_t *text = new_membuf();

        try {
                print(script, text, false);
                membuf_append_zero(text);
                JsonCpp json = JsonCpp::parse(membuf_data(text));
                
        } catch (JSONError& je) {
                printf("Generated JSON:\n%s", membuf_data(text));
                r_err("Parsing failed: %s", je.what());
                FAIL() << "Failed to parse the JSON";
        }

        delete_membuf(text);
}

TEST_F(print_tests, test_valid_json_with_slices)
{
        // Arrange
        v3 start_position(0, 0, 0);
        SmoothPath script(start_position);
        script.moveto(0.10, 0.00, 0.0, 1.0);
        script.moveto(0.00, 0.00, 0.0, 1.0);
        script.convert(vmax, amax, deviation, period, maxlen);

        membuf_t *text = new_membuf();

        try {
                print(script, text, true);
                membuf_append_zero(text);
                JsonCpp json = JsonCpp::parse(membuf_data(text));
                
        } catch (JSONError& je) {
                printf("Generated JSON:\n%s", membuf_data(text));
                r_err("Parsing failed: %s", je.what());
                FAIL() << "Failed to parse the JSON";
        }

        delete_membuf(text);
}
