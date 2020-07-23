#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <vector>
#include <Parser.h>

using namespace std;
using namespace testing;

// See also https://stackoverflow.com/questions/780819/how-can-i-unit-test-arduino-code


class parser_tests : public ::testing::Test
{
protected:
	parser_tests() {
	}

	~parser_tests() override = default;

	void SetUp() override {
	}

	void TearDown() override {
	}

        void send_command(Parser *parser,
                          const char *s,
                          vector<bool> &r,
                          vector<char> &e) {
                for (int i = 0; s[i] != 0; i++) {
                        r.push_back(parser->process(s[i]));
                        e.push_back(parser->error());
                }
        }
        
};

TEST_F(parser_tests, parser_returns_true_on_opcode_with_value)
{
        // Arrange
        Parser parser("ab", "xy");

        // Act
        vector<bool> r;
        vector<char> e;
        send_command(&parser, "a1\r", r, e);
        
        //Assert
        ASSERT_THAT(r, ElementsAre(false, false, true));
        ASSERT_THAT(e, ElementsAre(0, 0, 0));
        ASSERT_EQ(parser.opcode(), 'a');
        ASSERT_EQ(parser.length(), 1);
        ASSERT_EQ(parser.value(0), 1);
}

TEST_F(parser_tests, parser_returns_false_on_opcode_with_value)
{
        // Arrange
        Parser parser("ab", "xy");

        // Act
        vector<bool> r;
        vector<char> e;
        send_command(&parser, "a\r", r, e);
        
        //Assert
        ASSERT_THAT(r, ElementsAre(false, false));
        ASSERT_THAT(e, ElementsAre(0, parser_unexpected_char));
        ASSERT_EQ(parser.opcode(), 0);
        ASSERT_EQ(parser.length(), 0);
}

TEST_F(parser_tests, parser_returns_true_on_opcode_without_value)
{
        // Arrange
        Parser parser("ab", "xy");

        // Act
        vector<bool> r;
        vector<char> e;
        send_command(&parser, "x\r", r, e);
        
        //Assert
        ASSERT_THAT(r, ElementsAre(false, true));
        ASSERT_THAT(e, ElementsAre(0, 0));
        ASSERT_EQ(parser.opcode(), 'x');
        ASSERT_EQ(parser.length(), 0);
}

TEST_F(parser_tests, parser_returns_false_on_opcode_without_value)
{
        // Arrange
        Parser parser("ab", "xy");

        // Act
        vector<bool> r;
        vector<char> e;
        send_command(&parser, "x0\r", r, e);
        
        //Assert
        ASSERT_THAT(r, ElementsAre(false, false, false));
        ASSERT_THAT(e, ElementsAre(0, parser_unexpected_char,
                                   parser_unexpected_char));
        ASSERT_EQ(parser.opcode(), 0);
        ASSERT_EQ(parser.length(), 0);
}

TEST_F(parser_tests, parser_returns_false_on_delimiter_space)
{
        // Arrange
        Parser parser("ab", "xy");

        // Act
        vector<bool> r;
        vector<char> e;
        send_command(&parser, "x ", r, e);
        
        //Assert
        ASSERT_THAT(r, ElementsAre(false, false));
        ASSERT_THAT(e, ElementsAre(0, parser_unexpected_char));
        ASSERT_EQ(parser.opcode(), 0);
        ASSERT_EQ(parser.length(), 0);
}

TEST_F(parser_tests, parser_returns_true_on_atomic_opcode_with_cr_lf)
{
        // Arrange
        Parser parser("ab", "xy");

        // Act
        vector<bool> r;
        vector<char> e;
        send_command(&parser, "x\r\n", r, e);
        
        //Assert
        ASSERT_THAT(r, ElementsAre(false, true, false));
        ASSERT_EQ(parser.opcode(), 'x');
        ASSERT_THAT(e, ElementsAre(0, 0, 0));
        ASSERT_EQ(parser.length(), 0);
}

TEST_F(parser_tests, parser_returns_true_on_atomic_opcode_with_lf)
{
        // Arrange
        Parser parser("ab", "xy");

        // Act
        vector<bool> r;
        vector<char> e;
        send_command(&parser, "x\n", r, e);
        
        //Assert
        ASSERT_THAT(r, ElementsAre(false, true));
        ASSERT_THAT(e, ElementsAre(0, 0));
        ASSERT_EQ(parser.opcode(), 'x');
        ASSERT_EQ(parser.length(), 0);
}

TEST_F(parser_tests, parser_returns_true_on_scalar_opcode_with_cr_lf)
{
        // Arrange
        Parser parser("ab", "xy");

        // Act
        vector<bool> r;
        vector<char> e;
        send_command(&parser, "a1\r\n", r, e);
        
        //Assert
        ASSERT_THAT(r, ElementsAre(false, false, true, false));
        ASSERT_EQ(parser.opcode(), 'a');
        ASSERT_THAT(e, ElementsAre(0, 0, 0, 0));
        ASSERT_EQ(parser.length(), 1);
        ASSERT_EQ(parser.value(), 1);
}

TEST_F(parser_tests, parser_returns_true_on_scalar_opcode_with_lf)
{
        // Arrange
        Parser parser("ab", "xy");

        // Act
        vector<bool> r;
        vector<char> e;
        send_command(&parser, "a1\n", r, e);
        
        //Assert
        ASSERT_THAT(r, ElementsAre(false, false, true));
        ASSERT_THAT(e, ElementsAre(0, 0, 0));
        ASSERT_EQ(parser.opcode(), 'a');
        ASSERT_EQ(parser.length(), 1);
        ASSERT_EQ(parser.value(), 1);
}

TEST_F(parser_tests, parser_returns_true_false_on_delimiter_nl_cr)
{
        // Arrange
        Parser parser("ab", "xy");

        // Act
        vector<bool> r;
        vector<char> e;
        send_command(&parser, "x\n\r", r, e);
        
        //Assert
        ASSERT_THAT(r, ElementsAre(false, true, false));
        ASSERT_THAT(e, ElementsAre(0, 0, parser_unexpected_char));
        ASSERT_EQ(parser.opcode(), 0);
        ASSERT_EQ(parser.length(), 0);
}

TEST_F(parser_tests, parser_returns_false_on_delimiter_space_cr)
{
        // Arrange
        Parser parser("ab", "xy");

        // Act
        vector<bool> r;
        vector<char> e;
        send_command(&parser, "x \r", r, e);
        
        //Assert
        ASSERT_THAT(r, ElementsAre(false, false, false));
        ASSERT_THAT(e, ElementsAre(0, parser_unexpected_char,
                                   parser_unexpected_char));
        ASSERT_EQ(parser.opcode(), 0);
        ASSERT_EQ(parser.length(), 0);
}

TEST_F(parser_tests, parser_returns_true_on_negative_scalar)
{
        // Arrange
        Parser parser("ab", "xy");

        // Act
        vector<bool> r;
        vector<char> e;
        send_command(&parser, "a-1\r", r, e);
        
        //Assert
        ASSERT_THAT(r, ElementsAre(false, false, false, true));
        ASSERT_THAT(e, ElementsAre(0, 0, 0, 0));
        ASSERT_EQ(parser.opcode(), 'a');
        ASSERT_EQ(parser.length(), 1);
        ASSERT_EQ(parser.value(0), -1);
}

TEST_F(parser_tests, parser_returns_false_on_empty_vector)
{
        // Arrange
        Parser parser("ab", "xy");

        // Act
        vector<bool> r;
        vector<char> e;
        send_command(&parser, "a[]\r", r, e);
        
        //Assert
        ASSERT_THAT(r, ElementsAre(false, false, false, false));
        ASSERT_THAT(e, ElementsAre(0, 0, parser_unexpected_char,
                                   parser_unexpected_char));
        ASSERT_EQ(parser.opcode(), 0);
        ASSERT_EQ(parser.length(), 0);
}

TEST_F(parser_tests, parser_returns_true_on_vector_size_1)
{
        // Arrange
        Parser parser("ab", "xy");

        // Act
        vector<bool> r;
        vector<char> e;
        send_command(&parser, "a[1]\r", r, e);
        
        //Assert
        ASSERT_THAT(r, ElementsAre(false, false, false, false, true));
        ASSERT_THAT(e, ElementsAre(0, 0, 0, 0, 0));
        ASSERT_EQ(parser.opcode(), 'a');
        ASSERT_EQ(parser.length(), 1);
        ASSERT_EQ(parser.value(0), 1);
}

TEST_F(parser_tests, parser_returns_true_on_vector_size_5)
{
        // Arrange
        Parser parser("ab", "xy");

        // Act
        vector<bool> r;
        vector<char> e;
        send_command(&parser, "a[0,1,2,3,4]\r", r, e);
        
        //Assert
        ASSERT_THAT(r, ElementsAre(false, false, false, false, false,
                                   false, false, false, false, false,
                                   false, false, true));
        ASSERT_THAT(e, ElementsAre(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
        ASSERT_EQ(parser.opcode(), 'a');
        ASSERT_EQ(parser.length(), 5);
        for (int i = 0; i < 5; i++)
                ASSERT_EQ(parser.value(i), i);
}

TEST_F(parser_tests, parser_returns_true_on_vector_1_negative)
{
        // Arrange
        Parser parser("ab", "xy");

        // Act
        vector<bool> r;
        vector<char> e;
        send_command(&parser, "a[-1]\r", r, e);
        
        //Assert
        ASSERT_THAT(r, ElementsAre(false, false, false, false, false, true));
        ASSERT_THAT(e, ElementsAre(0, 0, 0, 0, 0, 0));
        ASSERT_EQ(parser.opcode(), 'a');
        ASSERT_EQ(parser.length(), 1);
        ASSERT_EQ(parser.value(0), -1);
}

TEST_F(parser_tests, parser_returns_true_on_vector_2_negative)
{
        // Arrange
        Parser parser("ab", "xy");

        // Act
        vector<bool> r;
        vector<char> e;
        send_command(&parser, "a[-1,-1]\r", r, e);
        
        //Assert
        ASSERT_THAT(r, ElementsAre(false, false, false, false, false,
                                   false, false, false, true));
        ASSERT_THAT(e, ElementsAre(0, 0, 0, 0, 0, 0, 0, 0, 0));
        ASSERT_EQ(parser.opcode(), 'a');
        ASSERT_EQ(parser.length(), 2);
        ASSERT_EQ(parser.value(0), -1);
        ASSERT_EQ(parser.value(1), -1);
}

TEST_F(parser_tests, parser_returns_false_on_vector_with_spaces)
{
        // Arrange
        Parser parser("ab", "xy");

        // Act
        vector<bool> r;
        vector<char> e;
        send_command(&parser, "a[ -1 ]\r", r, e);
        
        //Assert
        ASSERT_THAT(r, ElementsAre(false, false, false, false, false, false, false, false));
        ASSERT_EQ(e[3], parser_unexpected_char);
        ASSERT_EQ(parser.opcode(), 0);
        ASSERT_EQ(parser.length(), 0);
}

TEST_F(parser_tests, parser_returns_true_on_two_commands)
{
        // Arrange
        Parser parser("ab", "xy");

        // Act
        vector<bool> r;
        vector<char> e;
        send_command(&parser, "x\ry\r", r, e);
        
        //Assert
        ASSERT_THAT(r, ElementsAre(false, true, false, true));
        ASSERT_EQ(parser.opcode(), 'y');
        ASSERT_EQ(parser.length(), 0);
}

TEST_F(parser_tests, parser_returns_true_on_32_args)
{
        // Arrange
        Parser parser("ab", "xy");

        // Act
        const char *s = ("a[0,1,2,3,4,5,6,7,8,9,"
                         "0,1,2,3,4,5,6,7,8,9,"
                         "0,1,2,3,4,5,6,7,8,9,"
                         "0,1]\r");
        int len = strlen(s);
        vector<bool> r;
        vector<char> e;
        send_command(&parser, s, r, e);
        
        //Assert
        for (int i = 0; i < len - 1; i++)
                ASSERT_EQ(false, r[i]);
        ASSERT_EQ(true, r[len-1]);
        ASSERT_EQ(parser.opcode(), 'a');
        ASSERT_EQ(parser.length(), 32);
        for (int i = 0; i < 32; i++)
                ASSERT_EQ(parser.value(i), (i % 10));
}

TEST_F(parser_tests, parser_returns_false_on_33_args)
{
        // Arrange
        Parser parser("ab", "xy");

        // Act
        const char *s = ("a[0,1,2,3,4,5,6,7,8,9,"
                         "0,1,2,3,4,5,6,7,8,9,"
                         "0,1,2,3,4,5,6,7,8,9,"
                         "0,1,2]\r");
        int len = strlen(s);
        vector<bool> r;
        vector<char> e;
        send_command(&parser, s, r, e);
        
        //Assert
        for (int i = 0; i < len; i++)
                ASSERT_EQ(false, r[i]);
        ASSERT_EQ(e[66], 0);
        ASSERT_EQ(e[67], parser_vector_too_long);
        ASSERT_EQ(e[68], parser_unexpected_char);
        ASSERT_EQ(parser.opcode(), 0);
        ASSERT_EQ(parser.length(), 0);
}

TEST_F(parser_tests, parser_returns_false_on_large_value)
{
        // Arrange
        Parser parser("ab", "xy");

        // Act
        const char *s = ("a100000\r");
        int len = strlen(s);
        vector<bool> r;
        vector<char> e;
        send_command(&parser, s, r, e);
        
        //Assert
        for (int i = 0; i < len; i++)
                ASSERT_EQ(false, r[i]);
        ASSERT_EQ(e[5], 0);
        ASSERT_EQ(e[6], parser_value_out_of_range);
        ASSERT_EQ(e[7], parser_unexpected_char);
        ASSERT_EQ(parser.opcode(), 0);
        ASSERT_EQ(parser.length(), 0);
}

TEST_F(parser_tests, parser_returns_false_on_large_negative_value)
{
        // Arrange
        Parser parser("ab", "xy");

        // Act
        const char *s = ("a-100000\r");
        int len = strlen(s);
        vector<bool> r;
        vector<char> e;
        send_command(&parser, s, r, e);
        
        //Assert
        for (int i = 0; i < len; i++)
                ASSERT_EQ(false, r[i]);
        ASSERT_EQ(e[6], 0);
        ASSERT_EQ(e[7], parser_value_out_of_range);
        ASSERT_EQ(e[8], parser_unexpected_char);
        ASSERT_EQ(parser.opcode(), 0);
        ASSERT_EQ(parser.length(), 0);
}
