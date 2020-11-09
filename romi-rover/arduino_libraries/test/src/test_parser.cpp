#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <vector>
#include <Parser.h>
#include <RomiSerialErrors.h>
#include <CRC8.h>

using namespace std;
using namespace testing;

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

        void assert_success(vector<bool> &r, vector<int8_t> &e) {                
                ASSERT_EQ(true, r[r.size()-1]);
                for (size_t i = 0; i < r.size()-1; i++)
                        ASSERT_EQ(false, r[i]);
                
                for (size_t i = 0; i < e.size(); i++)
                        ASSERT_EQ(0, e[i]);
	}

        void assert_failure(vector<bool> &r, vector<int8_t> &e,
                            int8_t error, size_t index) {
                for (size_t i = 0; i < r.size(); i++)
                        ASSERT_EQ(false, r[i]);
                
                for (size_t i = 0; i < e.size(); i++) {
                        if (i == index)
                                ASSERT_EQ(error, e[i]);
                        else
                                ASSERT_EQ(0, e[i]);
                }
	}

        void send_command(Parser *parser,
                          const char *s,
                          vector<bool> &r,
                          vector<int8_t> &e) {
                for (int i = 0; s[i] != 0; i++) {
                        r.push_back(parser->process(s[i]));
                        e.push_back(parser->error());
                }
        }
        
};

TEST_F(parser_tests, parser_succeeds_on_opcode_without_value)
{
        // Arrange
        Parser parser;

        // Act
        const char *s = "#x\r";
        vector<bool> r;
        vector<int8_t> e;
        send_command(&parser, s, r, e);
        
        //Assert
        assert_success(r, e);
        ASSERT_EQ(parser.opcode(), 'x');
        ASSERT_EQ(parser.length(), 0);
}

TEST_F(parser_tests, parser_succeeds_on_opcode_with_value)
{
        // Arrange
        Parser parser;

        // Act
        const char *s = "#a[1]\r";
        vector<bool> r;
        vector<int8_t> e;
        send_command(&parser, s, r, e);
        
        //Assert
        assert_success(r, e);
        ASSERT_EQ(parser.opcode(), 'a');
        ASSERT_EQ(parser.length(), 1);
        ASSERT_EQ(parser.value(0), 1);
}

TEST_F(parser_tests, parser_fails_on_opcode_without_value)
{
        // Arrange
        Parser parser;

        // Act
        const char *s = "#x0\r";
        vector<bool> r;
        vector<int8_t> e;
        send_command(&parser, s, r, e);
        
        //Assert
        assert_failure(r, e, romiserial_unexpected_char, 2);
}

TEST_F(parser_tests, parser_fails_on_delimiter_space)
{
        // Arrange
        Parser parser;

        // Act
        const char *s = "#x ";
        vector<bool> r;
        vector<int8_t> e;
        send_command(&parser, s, r, e);
        
        //Assert
        assert_failure(r, e, romiserial_unexpected_char, 2);
}

TEST_F(parser_tests, parser_fails_on_atomic_opcode_with_lf)
{
        // Arrange
        Parser parser;

        // Act
        const char *s = "#x\n";
        vector<bool> r;
        vector<int8_t> e;
        send_command(&parser, s, r, e);
        
        //Assert
        assert_failure(r, e, romiserial_unexpected_char, 2);
}

TEST_F(parser_tests, parser_fails_on_scalar_opcode_with_lf)
{
        // Arrange
        Parser parser;

        // Act
        const char *s = "#a[1]\n";
        vector<bool> r;
        vector<int8_t> e;
        send_command(&parser, s, r, e);
        
        //Assert
        assert_failure(r, e, romiserial_unexpected_char, 5);
}

TEST_F(parser_tests, parser_fails_on_delimiter_lf_cr)
{
        // Arrange
        Parser parser;

        // Act
        const char *s = "#x\n\r";
        vector<bool> r;
        vector<int8_t> e;
        send_command(&parser, s, r, e);
        
        //Assert
        assert_failure(r, e, romiserial_unexpected_char, 2);
}

TEST_F(parser_tests, parser_fails_on_delimiter_space_cr)
{
        // Arrange
        Parser parser;

        // Act
        const char *s = "#x \r";
        vector<bool> r;
        vector<int8_t> e;
        send_command(&parser, s, r, e);
        
        //Assert
        assert_failure(r, e, romiserial_unexpected_char, 2);
}

TEST_F(parser_tests, parser_fails_on_empty_vector)
{
        // Arrange
        Parser parser;

        // Act
        const char *s = "#a[]\r";
        vector<bool> r;
        vector<int8_t> e;
        send_command(&parser, s, r, e);
        
        //Assert
        assert_failure(r, e, romiserial_unexpected_char, 3);
}

TEST_F(parser_tests, parser_succeeds_on_vector_size_1)
{
        // Arrange
        Parser parser;

        // Act
        const char *s = "#a[1]\r";
        vector<bool> r;
        vector<int8_t> e;
        send_command(&parser, s, r, e);
        
        //Assert
        assert_success(r, e);
        ASSERT_EQ(parser.opcode(), 'a');
        ASSERT_EQ(parser.length(), 1);
        ASSERT_EQ(parser.value(0), 1);
}

TEST_F(parser_tests, parser_succeeds_on_vector_size_5)
{
        // Arrange
        Parser parser;

        // Act
        const char *s = "#a[0,1,2,3,4]\r";
        vector<bool> r;
        vector<int8_t> e;
        send_command(&parser, s, r, e);
        
        //Assert
        assert_success(r, e);
        ASSERT_EQ(parser.opcode(), 'a');
        ASSERT_EQ(parser.length(), 5);
        for (int i = 0; i < 5; i++)
                ASSERT_EQ(parser.value(i), i);
}

TEST_F(parser_tests, parser_succeeds_on_vector_1_negative)
{
        // Arrange
        Parser parser;

        // Act
        const char *s = "#a[-1]\r";
        vector<bool> r;
        vector<int8_t> e;
        send_command(&parser, s, r, e);
        
        //Assert
        assert_success(r, e);
        ASSERT_EQ(parser.opcode(), 'a');
        ASSERT_EQ(parser.length(), 1);
        ASSERT_EQ(parser.value(0), -1);
}

TEST_F(parser_tests, parser_succeeds_on_vector_2_negative)
{
        // Arrange
        Parser parser;

        // Act
        const char *s = "#a[-1,-1]\r";
        vector<bool> r;
        vector<int8_t> e;
        send_command(&parser, s, r, e);
        
        //Assert
        assert_success(r, e);
        ASSERT_EQ(parser.opcode(), 'a');
        ASSERT_EQ(parser.length(), 2);
        ASSERT_EQ(parser.value(0), -1);
        ASSERT_EQ(parser.value(1), -1);
}

TEST_F(parser_tests, parser_fails_on_vector_with_spaces)
{
        // Arrange
        Parser parser;

        // Act
        const char *s = "#a[ -1 ]\r";
        vector<bool> r;
        vector<int8_t> e;
        send_command(&parser, s, r, e);
        
        //Assert
        assert_failure(r, e, romiserial_unexpected_char, 3);
}

TEST_F(parser_tests, parser_succeeds_on_two_commands)
{
        // Arrange
        Parser parser;

        // Act
        const char *s = "#x\r#y\r";
        int len = strlen(s);
        vector<bool> r;
        vector<int8_t> e;
        send_command(&parser, s, r, e);
        
        //Assert
        ASSERT_EQ(true, r[2]);
        ASSERT_EQ(true, r[len-1]);
        for (int i = 0; i < len; i++)
                ASSERT_EQ(0, e[i]);
        ASSERT_EQ(parser.opcode(), 'y');
        ASSERT_EQ(parser.length(), 0);
}

TEST_F(parser_tests, parser_succeeds_on_26_args)
{
        // Arrange
        Parser parser;

        // Act
        const char *s = ("#a[0,1,2,3,4,5,6,7,8,9,"
                         "0,1,2,3,4,5,6,7,8,9,"
                         "0,1,2,3,4,5]\r");
        vector<bool> r;
        vector<int8_t> e;
        send_command(&parser, s, r, e);
        
        //Assert
        assert_success(r, e);
        ASSERT_EQ(parser.opcode(), 'a');
        ASSERT_EQ(parser.length(), 26);
        for (int i = 0; i < 26; i++)
                ASSERT_EQ(parser.value(i), (i % 10));
}

TEST_F(parser_tests, parser_fails_on_27_args)
{
        // Arrange
        Parser parser;

        // Act
        const char *s = ("#a[0,1,2,3,4,5,6,7,8,9,"
                         "0,1,2,3,4,5,6,7,8,9,"
                         "0,1,2,3,4,5,6]\r");
        vector<bool> r;
        vector<int8_t> e;
        send_command(&parser, s, r, e);
        
        //Assert
        assert_failure(r, e, romiserial_vector_too_long, 56);
}

TEST_F(parser_tests, parser_fails_on_large_value)
{
        // Arrange
        Parser parser;

        // Act
        const char *s = "#a[100000]\r";
        vector<bool> r;
        vector<int8_t> e;
        send_command(&parser, s, r, e);
        
        //Assert
        assert_failure(r, e, romiserial_value_out_of_range, 8);
}

TEST_F(parser_tests, parser_fails_on_large_negative_value)
{
        // Arrange
        Parser parser;

        // Act
        const char *s = "#a[-100000]\r";
        vector<bool> r;
        vector<int8_t> e;
        send_command(&parser, s, r, e);
        
        //Assert
        assert_failure(r, e, romiserial_value_out_of_range, 9);
}

TEST_F(parser_tests, parser_succeeds_on_array_with_single_string)
{
        // Arrange
        Parser parser;

        // Act
        const char *s = "#a[\"toto\"]\r";
        vector<bool> r;
        vector<int8_t> e;
        send_command(&parser, s, r, e);
        
        //Assert
        assert_success(r, e);
        ASSERT_EQ(parser.opcode(), 'a');
        ASSERT_EQ(parser.length(), 0);
        ASSERT_EQ(parser.has_string(), 1);
        ASSERT_STREQ(parser.string(), "toto");
}

TEST_F(parser_tests, parser_succeeds_on_array_with_string_and_value)
{
        // Arrange
        Parser parser;

        // Act
        const char *s = "#a[\"toto\",1]\r";
        vector<bool> r;
        vector<int8_t> e;
        send_command(&parser, s, r, e);
        
        //Assert
        assert_success(r, e);
        ASSERT_EQ(parser.opcode(), 'a');
        ASSERT_EQ(parser.has_string(), 1);
        ASSERT_STREQ(parser.string(), "toto");
        ASSERT_EQ(parser.length(), 1);
        ASSERT_EQ(parser.value(), 1);
}

TEST_F(parser_tests, parser_succeeds_on_array_with_value_and_string)
{
        // Arrange
        Parser parser;

        // Act
        const char *s = "#a[1,\"toto\"]\r";
        vector<bool> r;
        vector<int8_t> e;
        send_command(&parser, s, r, e);
        
        //Assert
        assert_success(r, e);
        ASSERT_EQ(parser.opcode(), 'a');
        ASSERT_EQ(parser.has_string(), 1);
        ASSERT_STREQ(parser.string(), "toto");
        ASSERT_EQ(parser.length(), 1);
        ASSERT_EQ(parser.value(), 1);
}

TEST_F(parser_tests, parser_succeeds_on_array_with_string_and_three_values)
{
        // Arrange
        Parser parser;

        // Act
        const char *s = "#a[\"toto\",1,2,3]\r";
        vector<bool> r;
        vector<int8_t> e;
        send_command(&parser, s, r, e);
        
        //Assert
        assert_success(r, e);
        ASSERT_EQ(parser.opcode(), 'a');
        ASSERT_EQ(parser.has_string(), 1);
        ASSERT_STREQ(parser.string(), "toto");
        ASSERT_EQ(parser.length(), 3);
        ASSERT_EQ(parser.value(0), 1);
        ASSERT_EQ(parser.value(1), 2);
        ASSERT_EQ(parser.value(2), 3);
}

TEST_F(parser_tests, parser_succeeds_on_array_with_three_values_and_string)
{
        // Arrange
        Parser parser;

        // Act
        const char *s = "#a[1,2,3,\"toto\"]\r";
        vector<bool> r;
        vector<int8_t> e;
        send_command(&parser, s, r, e);
        
        //Assert
        assert_success(r, e);
        ASSERT_EQ(parser.opcode(), 'a');
        ASSERT_EQ(parser.has_string(), 1);
        ASSERT_STREQ(parser.string(), "toto");
        ASSERT_EQ(parser.length(), 3);
        ASSERT_EQ(parser.value(0), 1);
        ASSERT_EQ(parser.value(1), 2);
        ASSERT_EQ(parser.value(2), 3);
}

TEST_F(parser_tests, parser_fails_on_string_without_closing_quote)
{
        // Arrange
        Parser parser;

        // Act
        const char *s = "#a[\"toto\r";
        vector<bool> r;
        vector<int8_t> e;
        send_command(&parser, s, r, e);
        
        //Assert
        assert_failure(r, e, romiserial_invalid_string, 8);
}

TEST_F(parser_tests, parser_fails_on_long_string)
{
        // Arrange
        Parser parser;

        // Act
        const char *s = "#a[\"0123456789012345678901234567890123456789\"]\r";
        vector<bool> r;
        vector<int8_t> e;
        send_command(&parser, s, r, e);
        
        //Assert
        assert_failure(r, e, romiserial_string_too_long, 36);
        ASSERT_EQ(parser.has_string(), 0);
}

TEST_F(parser_tests, parser_fails_on_array_with_two_string)
{
        // Arrange
        Parser parser;

        // Act
        const char *s = "#a[\"hello\",\"world\"]\r";
        vector<bool> r;
        vector<int8_t> e;
        send_command(&parser, s, r, e);
        
        //Assert
        assert_failure(r, e, romiserial_too_many_strings, 11);
}

TEST_F(parser_tests, parser_accepts_message_64_chars_long)
{
        // Arrange
        Parser parser;

        // Act
        const char *s = ("#a[\"012345678901234567890\",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]\r");
        vector<bool> r;
        vector<int8_t> e;
        send_command(&parser, s, r, e);
        
        //Assert
        assert_success(r, e);
        ASSERT_EQ(parser.opcode(), 'a');
        ASSERT_EQ(parser.length(), 18);
        ASSERT_EQ(parser.has_string(), 1);
}

TEST_F(parser_tests, parser_returns_error_on_message_too_long)
{
        // Arrange
        Parser parser;

        // Act
        const char *s = ("#a[\"012345678901234567890\",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]\r");
        vector<bool> r;
        vector<int8_t> e;
        send_command(&parser, s, r, e);
        
        //Assert
        assert_failure(r, e, romiserial_too_long, 64);
}

TEST_F(parser_tests, parser_returns_correct_crc_1)
{
        // Arrange
        CRC8 crc8;

        // Act
        const char s[] = { 0x00 };
        uint8_t result = crc8.compute(s, 1);

        //Assert
        ASSERT_EQ(result, 0x00);
}

TEST_F(parser_tests, parser_returns_correct_crc_2)
{
        // Arrange
        CRC8 crc8;

        // Act
        const char s[] = { 0x01 };
        uint8_t result = crc8.compute(s, 1);

        //Assert
        ASSERT_EQ(result, 0x07);
}

TEST_F(parser_tests, parser_returns_correct_crc_3)
{
        // Arrange
        CRC8 crc8;

        // Act
        const char s[] = "Hello world!";
        uint8_t result = crc8.compute(s, 12);

        //Assert
        ASSERT_EQ(result, 0x27);
}

TEST_F(parser_tests, parser_returns_correct_crc_4)
{
        // Arrange
        CRC8 crc8;

        // Act
        const char *s = "#e[0]:00";
        uint8_t result = crc8.compute(s, strlen(s));
        
        //Assert
        ASSERT_EQ(result, 0x92);
}

TEST_F(parser_tests, parser_returns_correct_crc_5)
{
        // Arrange
        CRC8 crc8;

        // Act
        const char *s = "#e:7b";
        uint8_t result = crc8.compute(s, strlen(s));
        
        //Assert
        ASSERT_EQ(result, 0x04);
}

TEST_F(parser_tests, parser_returns_correct_id)
{
        // Arrange
        Parser parser;

        // Act
        const char *s = "#e:7b04\r";
        vector<bool> r;
        vector<int8_t> e;
        send_command(&parser, s, r, e);
        
        //Assert
        assert_success(r, e);
        ASSERT_EQ(parser.opcode(), 'e');
        ASSERT_EQ(parser.length(), 0);
        ASSERT_EQ(parser.id(), 0x7b);
        ASSERT_EQ(parser.crc(), 0x04);
}

TEST_F(parser_tests, parser_fails_on_incomplete_metadata_1)
{
        // Arrange
        Parser parser;

        // Act
        const char *s = "#e:\r";
        vector<bool> r;
        vector<int8_t> e;
        send_command(&parser, s, r, e);
        
        //Assert
        assert_failure(r, e, romiserial_invalid_id, 3);
}

TEST_F(parser_tests, parser_fails_on_incomplete_metadata_2)
{
        // Arrange
        Parser parser;

        // Act
        const char *s = "#e:7b2\r";
        vector<bool> r;
        vector<int8_t> e;
        send_command(&parser, s, r, e);
        
        //Assert
        assert_failure(r, e, romiserial_invalid_crc, 6);
}

TEST_F(parser_tests, parser_fails_on_invalid_opcode)
{
        // Arrange
        Parser parser;

        // Act
        const char *s = "#=:0000\r";
        vector<bool> r;
        vector<int8_t> e;
        send_command(&parser, s, r, e);
        
        //Assert
        assert_failure(r, e, romiserial_invalid_opcode, 1);
}

TEST_F(parser_tests, parser_fails_on_invalid_string_char)
{
        // Arrange
        Parser parser;

        // Act
        const char *s = "#a[\"#arg!\"]\r";
        vector<bool> r;
        vector<int8_t> e;
        send_command(&parser, s, r, e);
        
        //Assert
        assert_failure(r, e, romiserial_invalid_string, 4);
}
