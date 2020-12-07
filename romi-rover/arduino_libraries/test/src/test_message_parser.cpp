#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <vector>
#include <MessageParser.h>
#include <RomiSerialErrors.h>
#include <CRC8.h>
#include <iostream>

using namespace std;
using namespace testing;

class message_parser_tests : public ::testing::Test
{
protected:
	message_parser_tests() {}

	~message_parser_tests() override = default;

	void SetUp() override {}

	void TearDown() override {}
};

TEST_F(message_parser_tests, parser_succeeds_on_opcode_without_value)
{
        // Arrange
        MessageParser parser;

        // Act
        const char *s = "x";
        bool message = parser.parse(s, strlen(s)+1); 
        
        //Assert
        ASSERT_EQ(message, true);
        ASSERT_EQ(parser.opcode(), 'x');
        ASSERT_EQ(parser.length(), 0);
}

TEST_F(message_parser_tests, parser_succeeds_on_opcode_with_value)
{
        // Arrange
        MessageParser parser;

        // Act
        const char *s = "a[1]";
        bool message = parser.parse(s, strlen(s)+1); 
        
        //Assert
        ASSERT_EQ(message, true);
        ASSERT_EQ(parser.opcode(), 'a');
        ASSERT_EQ(parser.length(), 1);
        ASSERT_EQ(parser.value(0), 1);
}

TEST_F(message_parser_tests, parser_fails_on_opcode_without_value)
{
        // Arrange
        MessageParser parser;

        // Act
        const char *s = "x0";
        bool message = parser.parse(s, strlen(s)+1); 
        
        //Assert
        ASSERT_EQ(message, false);
        ASSERT_EQ(parser.error(), romiserial_unexpected_char);
}

TEST_F(message_parser_tests, parser_fails_on_delimiter_space)
{
        // Arrange
        MessageParser parser;

        // Act
        const char *s = "x ";
        bool message = parser.parse(s, strlen(s)+1); 
        
        //Assert
        ASSERT_EQ(message, false);
        ASSERT_EQ(parser.error(), romiserial_unexpected_char);
}

TEST_F(message_parser_tests, parser_fails_on_atomic_opcode_with_lf)
{
        // Arrange
        MessageParser parser;

        // Act
        const char *s = "x\n";
        bool message = parser.parse(s, strlen(s)+1); 
        
        //Assert
        ASSERT_EQ(message, false);
        ASSERT_EQ(parser.error(), romiserial_unexpected_char);
}

TEST_F(message_parser_tests, parser_fails_on_scalar_opcode_with_lf)
{
        // Arrange
        MessageParser parser;

        // Act
        const char *s = "a[1]\n";
        bool message = parser.parse(s, strlen(s)+1); 
        
        //Assert
        ASSERT_EQ(message, false);
        ASSERT_EQ(parser.error(), romiserial_unexpected_char);
}

TEST_F(message_parser_tests, parser_fails_on_delimiter_lf_cr)
{
        // Arrange
        MessageParser parser;

        // Act
        const char *s = "x\n";
        bool message = parser.parse(s, strlen(s)+1); 
        
        //Assert
        ASSERT_EQ(message, false);
        ASSERT_EQ(parser.error(), romiserial_unexpected_char);
}

TEST_F(message_parser_tests, parser_fails_on_delimiter_space_cr)
{
        // Arrange
        MessageParser parser;

        // Act
        const char *s = "x ";
        bool message = parser.parse(s, strlen(s)+1); 
        
        //Assert
        ASSERT_EQ(message, false);
        ASSERT_EQ(parser.error(), romiserial_unexpected_char);
}

TEST_F(message_parser_tests, parser_fails_on_empty_vector)
{
        // Arrange
        MessageParser parser;

        // Act
        const char *s = "a[]";
        bool message = parser.parse(s, strlen(s)+1); 
        
        //Assert
        ASSERT_EQ(message, false);
        ASSERT_EQ(parser.error(), romiserial_unexpected_char);
}

TEST_F(message_parser_tests, parser_succeeds_on_vector_size_1)
{
        // Arrange
        MessageParser parser;

        // Act
        const char *s = "a[1]";
        bool message = parser.parse(s, strlen(s)+1); 
        
        //Assert
        ASSERT_EQ(message, true);
        ASSERT_EQ(parser.opcode(), 'a');
        ASSERT_EQ(parser.length(), 1);
        ASSERT_EQ(parser.value(0), 1);
}

TEST_F(message_parser_tests, parser_succeeds_on_vector_size_5)
{
        // Arrange
        MessageParser parser;

        // Act
        const char *s = "a[0,1,2,3,4]";
        bool message = parser.parse(s, strlen(s)+1); 
        
        //Assert
        ASSERT_EQ(message, true);
        ASSERT_EQ(parser.opcode(), 'a');
        ASSERT_EQ(parser.length(), 5);
        for (int i = 0; i < 5; i++)
                ASSERT_EQ(parser.value(i), i);
}

TEST_F(message_parser_tests, parser_succeeds_on_vector_1_negative)
{
        // Arrange
        MessageParser parser;

        // Act
        const char *s = "a[-1]";
        bool message = parser.parse(s, strlen(s)+1); 
        
        //Assert
        ASSERT_EQ(message, true);
        ASSERT_EQ(parser.opcode(), 'a');
        ASSERT_EQ(parser.length(), 1);
        ASSERT_EQ(parser.value(0), -1);
}

TEST_F(message_parser_tests, parser_succeeds_on_vector_2_negative)
{
        // Arrange
        MessageParser parser;

        // Act
        const char *s = "a[-1,-1]";
        bool message = parser.parse(s, strlen(s)+1); 
        
        //Assert
        ASSERT_EQ(message, true);
        ASSERT_EQ(parser.opcode(), 'a');
        ASSERT_EQ(parser.length(), 2);
        ASSERT_EQ(parser.value(0), -1);
        ASSERT_EQ(parser.value(1), -1);
}

TEST_F(message_parser_tests, parser_fails_on_vector_with_spaces)
{
        // Arrange
        MessageParser parser;

        // Act
        const char *s = "a[ -1 ]";
        bool message = parser.parse(s, strlen(s)+1); 
        
        //Assert
        ASSERT_EQ(message, false);
        ASSERT_EQ(parser.error(), romiserial_unexpected_char);
}

TEST_F(message_parser_tests, parser_succeeds_on_12_args)
{
        // Arrange
        MessageParser parser;

        // Act
        const char *s = ("a[0,1,2,3,4,5,6,7,8,9,0,1]");
        bool message = parser.parse(s, strlen(s)+1); 
        
        //Assert
        ASSERT_EQ(message, true);
        ASSERT_EQ(parser.opcode(), 'a');
        ASSERT_EQ(parser.length(), 12);
        for (int i = 0; i < 12; i++)
                ASSERT_EQ(parser.value(i), (i % 10));
}

TEST_F(message_parser_tests, parser_fails_on_13_args)
{
        // Arrange
        MessageParser parser;

        // Act
        const char *s = ("a[0,1,2,3,4,5,6,7,8,9,0,1,2]");
        bool message = parser.parse(s, strlen(s)+1); 
        
        //Assert
        ASSERT_EQ(message, false);
        ASSERT_EQ(parser.error(), romiserial_vector_too_long);
}

TEST_F(message_parser_tests, parser_fails_on_large_value)
{
        // Arrange
        MessageParser parser;

        // Act
        const char *s = "a[100000]";
        bool message = parser.parse(s, strlen(s)+1); 
        
        //Assert
        ASSERT_EQ(message, false);
        ASSERT_EQ(parser.error(), romiserial_value_out_of_range);
}

TEST_F(message_parser_tests, parser_fails_on_large_negative_value)
{
        // Arrange
        MessageParser parser;

        // Act
        const char *s = "a[-100000]";
        bool message = parser.parse(s, strlen(s)+1); 
        
        //Assert
        ASSERT_EQ(message, false);
        ASSERT_EQ(parser.error(), romiserial_value_out_of_range);
}

TEST_F(message_parser_tests, parser_succeeds_on_array_with_single_string)
{
        // Arrange
        MessageParser parser;

        // Act
        const char *s = "a[\"toto\"]";
        bool message = parser.parse(s, strlen(s)+1); 
        
        //Assert
        ASSERT_EQ(message, true);
        ASSERT_EQ(parser.opcode(), 'a');
        ASSERT_EQ(parser.length(), 0);
        ASSERT_EQ(parser.has_string(), 1);
        ASSERT_STREQ(parser.string(), "toto");
}

TEST_F(message_parser_tests, parser_succeeds_on_array_with_string_and_value)
{
        // Arrange
        MessageParser parser;

        // Act
        const char *s = "a[\"toto\",1]";
        bool message = parser.parse(s, strlen(s)+1); 
        
        //Assert
        ASSERT_EQ(message, true);
        ASSERT_EQ(parser.opcode(), 'a');
        ASSERT_EQ(parser.has_string(), 1);
        ASSERT_STREQ(parser.string(), "toto");
        ASSERT_EQ(parser.length(), 1);
        ASSERT_EQ(parser.value(), 1);
}

TEST_F(message_parser_tests, parser_succeeds_on_array_with_value_and_string)
{
        // Arrange
        MessageParser parser;

        // Act
        const char *s = "a[1,\"toto\"]";
        bool message = parser.parse(s, strlen(s)+1); 
        
        //Assert
        ASSERT_EQ(message, true);
        ASSERT_EQ(parser.opcode(), 'a');
        ASSERT_EQ(parser.has_string(), 1);
        ASSERT_STREQ(parser.string(), "toto");
        ASSERT_EQ(parser.length(), 1);
        ASSERT_EQ(parser.value(), 1);
}

TEST_F(message_parser_tests, parser_succeeds_on_array_with_string_and_three_values)
{
        // Arrange
        MessageParser parser;

        // Act
        const char *s = "a[\"toto\",1,2,3]";
        bool message = parser.parse(s, strlen(s)+1); 
        
        //Assert
        ASSERT_EQ(message, true);
        ASSERT_EQ(parser.opcode(), 'a');
        ASSERT_EQ(parser.has_string(), 1);
        ASSERT_STREQ(parser.string(), "toto");
        ASSERT_EQ(parser.length(), 3);
        ASSERT_EQ(parser.value(0), 1);
        ASSERT_EQ(parser.value(1), 2);
        ASSERT_EQ(parser.value(2), 3);
}

TEST_F(message_parser_tests, parser_succeeds_on_array_with_three_values_and_string)
{
        // Arrange
        MessageParser parser;

        // Act
        const char *s = "a[1,2,3,\"toto\"]";
        bool message = parser.parse(s, strlen(s)+1); 
        
        //Assert
        ASSERT_EQ(message, true);
        ASSERT_EQ(parser.opcode(), 'a');
        ASSERT_EQ(parser.has_string(), 1);
        ASSERT_STREQ(parser.string(), "toto");
        ASSERT_EQ(parser.length(), 3);
        ASSERT_EQ(parser.value(0), 1);
        ASSERT_EQ(parser.value(1), 2);
        ASSERT_EQ(parser.value(2), 3);
}

TEST_F(message_parser_tests, parser_fails_on_string_without_closing_quote)
{
        // Arrange
        MessageParser parser;

        // Act
        const char *s = "a[\"toto";
        bool message = parser.parse(s, strlen(s)+1); 
        
        //Assert
        ASSERT_EQ(message, false);
        ASSERT_EQ(parser.error(), romiserial_invalid_string);
}

TEST_F(message_parser_tests, parser_fails_on_long_string)
{
        // Arrange
        MessageParser parser;

        // Act
        const char *s = "a[\"0123456789012345678901234567890123456789\"]";
        bool message = parser.parse(s, strlen(s)+1); 
        
        //Assert
        ASSERT_EQ(parser.error(), romiserial_string_too_long);
        ASSERT_EQ(message, false);
        ASSERT_EQ(parser.has_string(), 0);
}

TEST_F(message_parser_tests, parser_fails_on_array_with_two_string)
{
        // Arrange
        MessageParser parser;

        // Act
        const char *s = "a[\"hello\",\"world\"]";
        bool message = parser.parse(s, strlen(s)+1); 
        
        //Assert
        ASSERT_EQ(message, false);
        ASSERT_EQ(parser.error(), romiserial_too_many_strings);
}
