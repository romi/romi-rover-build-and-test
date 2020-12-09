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
        json_object_t response = romiserial.send("a");
        int code = (int) json_array_getnum(response, 0);
        json_unref(response);
        
        ASSERT_EQ(0, code);        
}

TEST_F(romiserial_arduino_tests, test_failure_bad_number_of_arguments)
{
        json_object_t response = romiserial.send("b");
        int code = (int) json_array_getnum(response, 0);
        json_unref(response);
        
        ASSERT_EQ(romiserial_bad_number_of_arguments, code);
}

TEST_F(romiserial_arduino_tests, test_success_two_arguments)
{
        json_object_t response = romiserial.send("b[1,2]");
        int code = (int) json_array_getnum(response, 0);
        json_unref(response);
        
        ASSERT_EQ(0, code);
}

TEST_F(romiserial_arduino_tests, test_success_string_argument)
{
        json_object_t response = romiserial.send("c[1,\"Dinner's ready\"]");
        int code = (int) json_array_getnum(response, 0);
        json_unref(response);
        
        ASSERT_EQ(0, code);
}

TEST_F(romiserial_arduino_tests, test_success_two_arguments_returning_value)
{
        json_object_t response = romiserial.send("d[1,1]");
        int code = (int) json_array_getnum(response, 0);
        int result = (int) json_array_getnum(response, 1);
        json_unref(response);
        
        ASSERT_EQ(0, code);
        ASSERT_EQ(2, result);
}

TEST_F(romiserial_arduino_tests, test_success_string_argument_returning_string)
{
        json_object_t response = romiserial.send("e[\"he's resting\"]");
        
        //bool is_array = json_isarray(response);
        //int len = json_array_length(response);
        int code = (int) json_array_getnum(response, 0);
        //bool is_string = json_isstring(json_array_get(response, 1));
        std::string result = json_array_getstr(response, 1);
        json_unref(response);
        
        ASSERT_EQ(0, code);
        ASSERT_STREQ("he's resting", result.c_str());
}

