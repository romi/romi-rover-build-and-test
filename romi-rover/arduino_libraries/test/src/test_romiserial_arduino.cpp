#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <RSerial.h>
#include <RomiSerialClient.h>
#include <RomiSerialErrors.h>

class romiserial_arduino_tests : public ::testing::Test
{
protected:
        RSerial serial;
        RomiSerialClient romiserial;

	romiserial_arduino_tests()
                : serial("/dev/ttyACM0", 115200, 1),
                  romiserial(&serial, &serial) {
                //romiserial.set_debug(true);
	}
        
	~romiserial_arduino_tests() override = default;

	void SetUp() override {
	}

	void TearDown() override {
	}
};

TEST_F(romiserial_arduino_tests, test_success_of_simple_command)
{
        JSON response;
        romiserial.send("a", response);
        int code = (int) response.num(0);
        
        ASSERT_EQ(0, code);        
}

TEST_F(romiserial_arduino_tests, test_failure_bad_number_of_arguments)
{
        JSON response;
        romiserial.send("b", response);
        int code = (int) response.num(0);
        
        ASSERT_EQ(romiserial_bad_number_of_arguments, code);
}

TEST_F(romiserial_arduino_tests, test_success_two_arguments)
{
        JSON response;
        romiserial.send("b[1,2]", response);
        int code = (int) response.num(0);
        
        ASSERT_EQ(0, code);
}

TEST_F(romiserial_arduino_tests, test_success_string_argument)
{
        JSON response;
        romiserial.send("c[1,\"Dinner's ready\"]", response);
        int code = (int) response.num(0);
        
        ASSERT_EQ(0, code);
}

TEST_F(romiserial_arduino_tests, test_success_two_arguments_returning_value)
{
        JSON response;
        romiserial.send("d[1,1]", response);
        int code = (int) response.num(0);
        int result = (int) response.num(1);
        
        ASSERT_EQ(0, code);
        ASSERT_EQ(2, result);
}

TEST_F(romiserial_arduino_tests, test_success_string_argument_returning_string)
{
        JSON response;
        romiserial.send("e[\"he's resting\"]", response);
        int code = (int) response.num(0);
        const char *result = response.str(1);
        
        ASSERT_EQ(0, code);
        ASSERT_STREQ("he's resting", result);
}

