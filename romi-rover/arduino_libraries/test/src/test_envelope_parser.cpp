#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <vector>
#include <EnvelopeParser.h>
#include <RomiSerialErrors.h>
#include <CRC8.h>
#include <iostream>

using namespace std;
using namespace testing;

class envelope_parser_tests : public ::testing::Test
{
protected:
	envelope_parser_tests() {
	}

	~envelope_parser_tests() override = default;

	void SetUp() override {
	}

	void TearDown() override {
	}

        void dump(vector<bool> &r, vector<int8_t> &e) {
                for (size_t i = 0; i < r.size(); i++)
                        std::cout << "r[" << i << "]: " << r[i] << std::endl;
                for (size_t i = 0; i < e.size(); i++)
                        std::cout << "e[" << i << "]: " << (int) e[i] << std::endl;
        }
        
        void assert_success(vector<bool> &r, vector<int8_t> &e) {
                //dump(r, e);
                ASSERT_EQ(true, r[r.size()-1]);
                for (size_t i = 0; i < r.size()-1; i++)
                        ASSERT_EQ(false, r[i]);
                
                for (size_t i = 0; i < e.size(); i++)
                        ASSERT_EQ(0, e[i]);
	}

        void assert_failure(vector<bool> &r, vector<int8_t> &e,
                            int8_t error, int index = -1) {
                //dump(r, e);                
                for (size_t i = 0; i < r.size(); i++)
                        ASSERT_EQ(false, r[i]);
                
                for (size_t i = 0; i < e.size(); i++) {
                        if (e[i] != 0) {
                                ASSERT_EQ(error, e[i]);
                                if (index >= 0) {
                                        ASSERT_EQ(i, (size_t) index);
                                }
                        }
                }
	}

        void send_command(EnvelopeParser &parser,
                          const char *s,
                          vector<bool> &r,
                          vector<int8_t> &e) {
                for (int i = 0; s[i] != 0; i++) {
                        r.push_back(parser.process(s[i]));
                        e.push_back(parser.error());
                }
        }        
};

TEST_F(envelope_parser_tests, parser_returns_correct_crc_1)
{
        // Arrange
        CRC8 crc8;

        // Act
        const char s[] = { 0x00 };
        uint8_t result = crc8.compute(s, 1);

        //Assert
        ASSERT_EQ(result, 0x00);
}

TEST_F(envelope_parser_tests, parser_returns_correct_crc_2)
{
        // Arrange
        CRC8 crc8;

        // Act
        const char s[] = { 0x01 };
        uint8_t result = crc8.compute(s, 1);

        //Assert
        ASSERT_EQ(result, 0x07);
}

TEST_F(envelope_parser_tests, parser_returns_correct_crc_3)
{
        // Arrange
        CRC8 crc8;

        // Act
        const char s[] = "Hello world!";
        uint8_t result = crc8.compute(s, 12);

        //Assert
        ASSERT_EQ(result, 0x27);
}

TEST_F(envelope_parser_tests, parser_returns_correct_crc_4)
{
        // Arrange
        CRC8 crc8;

        // Act
        const char *s = "#e[0]:00";
        uint8_t result = crc8.compute(s, strlen(s));
        
        //Assert
        ASSERT_EQ(result, 0x92);
}

TEST_F(envelope_parser_tests, parser_returns_correct_crc_5)
{
        // Arrange
        CRC8 crc8;

        // Act
        const char *s = "#e:7b";
        uint8_t result = crc8.compute(s, strlen(s));
        
        //Assert
        ASSERT_EQ(result, 0x04);
}

TEST_F(envelope_parser_tests, parser_succeeds_on_opcode_without_metadata)
{
        // Arrange
        EnvelopeParser parser;

        // Act
        const char *s = "#e\r";
        vector<bool> r;
        vector<int8_t> e;
        send_command(parser, s, r, e);
        
        //Assert
        assert_success(r, e);
        ASSERT_STREQ(parser.message(), "e");
        ASSERT_EQ(parser.id(), 0);
        ASSERT_EQ(parser.has_id(), false);
}

TEST_F(envelope_parser_tests, parser_returns_correct_id)
{
        // Arrange
        EnvelopeParser parser;

        // Act
        const char *s = "#e:7b04\r";
        vector<bool> r;
        vector<int8_t> e;
        send_command(parser, s, r, e);
        
        //Assert
        assert_success(r, e);
        ASSERT_STREQ(parser.message(), "e");
        ASSERT_EQ(parser.id(), 0x7b);
        ASSERT_EQ(parser.crc(), 0x04);
        ASSERT_EQ(parser.has_id(), true);
}

TEST_F(envelope_parser_tests, parser_fails_on_bad_crc)
{
        // Arrange
        EnvelopeParser parser;

        // Act
        const char *s = "#e:7b00\r";
        vector<bool> r;
        vector<int8_t> e;
        send_command(parser, s, r, e);
        
        //Assert
        assert_failure(r, e, romiserial_envelope_crc_mismatch);
}

TEST_F(envelope_parser_tests, parser_fails_on_incomplete_metadata_1)
{
        // Arrange
        EnvelopeParser parser;

        // Act
        const char *s = "#e:\r";
        vector<bool> r;
        vector<int8_t> e;
        send_command(parser, s, r, e);
        
        //Assert
        assert_failure(r, e, romiserial_envelope_invalid_id);
}

TEST_F(envelope_parser_tests, parser_fails_on_incomplete_metadata_2)
{
        // Arrange
        EnvelopeParser parser;

        // Act
        const char *s = "#e:7b2\r";
        vector<bool> r;
        vector<int8_t> e;
        send_command(parser, s, r, e);
        
        //Assert
        assert_failure(r, e, romiserial_envelope_invalid_crc);
}

TEST_F(envelope_parser_tests, parser_fails_on_too_long_message)
{
        // Arrange
        EnvelopeParser parser;

        // Act
        const char *s = "#012345678901234567890123456789012345678901234567890123456789\r";
        vector<bool> r;
        vector<int8_t> e;
        send_command(parser, s, r, e);
        
        //Assert
        assert_failure(r, e, romiserial_envelope_too_long);
}
