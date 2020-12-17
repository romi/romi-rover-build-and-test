#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "JsonCpp.h"

using namespace std;
using namespace testing;

class JsonCpp_tests : public ::testing::Test
{
protected:
	JsonCpp_tests() {
	}

	~JsonCpp_tests() override = default;

	void SetUp() override {
	}

	void TearDown() override {
	}
};

TEST_F(JsonCpp_tests, JsonCpp_parse_returns_number)
{
        // Arrange

        // Act
        JsonCpp value = JsonCpp::parse("1");
        
        //Assert
        ASSERT_EQ(1.0, value.num());
}

TEST_F(JsonCpp_tests, JsonCpp_parse_returns_string)
{
        // Arrange

        // Act
        JsonCpp value = JsonCpp::parse("\"hello\"");
        
        //Assert
        ASSERT_STREQ("hello", value.str());
}

TEST_F(JsonCpp_tests, JsonCpp_parse_returns_array_of_numbers)
{
        // Arrange

        // Act
        JsonCpp value = JsonCpp::parse("[0,1,2]");
        
        //Assert
        ASSERT_EQ(0.0, value.num(0));
        ASSERT_EQ(1.0, value.num(1));
        ASSERT_EQ(2.0, value.num(2));
}

TEST_F(JsonCpp_tests, JsonCpp_parse_returns_object_with_array_of_numbers)
{
        // Arrange

        // Act
        JsonCpp value = JsonCpp::parse("{\"x\": [0,1,2]}");
        
        //Assert
        ASSERT_EQ(1.0, value.array("x").num(1));
}
