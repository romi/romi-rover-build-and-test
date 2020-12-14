#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <string>
#include <vector>
#include <r.h>
#include <RomiSerialClient.h>
#include <RomiSerialErrors.h>
#include "../mock/mock_inputstream.h"
#include "../mock/mock_outputstream.h"

using namespace std;
using namespace testing;

class romiserialclient_tests : public ::testing::Test
{
protected:
        MockInputStream in;
        MockOutputStream out;
        RomiSerialClient client;
        string output_message;
        string expected_message;
        
	romiserialclient_tests() {}

	~romiserialclient_tests() override = default;

	void SetUp() override {
                EXPECT_CALL(in, set_timeout(_));
                client.init(&in, &out);
	}

	void TearDown() override {
	}

        void initSerialAvailable(const char *s) {
                InSequence seq;
                int len = strlen(s);
                for (int i = len; i >= 1; i--) {
                        EXPECT_CALL(in, available)
                                .WillOnce(Return(i))
                                .RetiresOnSaturation();
                }
        }

        void initSerialRead(const char *s) {
                InSequence seq;
                int len = strlen(s);
                for (int i = 0; i < len; i++) {
                        EXPECT_CALL(in, read)
                                .WillOnce(Return(s[i]))
                                .RetiresOnSaturation();
                }
        }
        
        void initInput(const char *s) {
                initSerialAvailable(s);
                initSerialRead(s);
        }

        size_t append_output(const char *s) { 
                output_message += s; 
                return strlen(s); 
        }

        void setExpectedOutput(const char *s) {
                expected_message = s;
                EXPECT_CALL(out, print(_))
                        .WillOnce(Invoke(this, &romiserialclient_tests::append_output));
        }
        
};

TEST_F(romiserialclient_tests, message_without_args)
{
        // Arrange
        setExpectedOutput("#a:008e\r");
        initInput("#a[0]:00e7\r");

        // Act
        JSON response;
        client.send("a", response);
        
        //Assert
        EXPECT_EQ(expected_message, output_message);
        EXPECT_EQ(true, response.isarray());
        EXPECT_EQ(1, response.length());
        EXPECT_EQ(0, response.num(0));
}

TEST_F(romiserialclient_tests, message_with_args)
{
        // Arrange
        setExpectedOutput("#a[1,2,3]:00dd\r");
        initInput("#a[0]:00e7\r");

        // Act
        JSON response;
        client.send("a[1,2,3]", response);
        
        //Assert
        EXPECT_EQ(expected_message, output_message);
        EXPECT_EQ(true, response.isarray());
        EXPECT_EQ(1, response.length());
        EXPECT_EQ(0, response.num(0));
}

TEST_F(romiserialclient_tests, error_reponse)
{
        // Arrange
        setExpectedOutput("#a:008e\r");
        initInput("#a[1,\"Went to bed early\"]:00f2\r");

        // Act
        JSON response;
        client.send("a", response);
        
        //Assert
        EXPECT_EQ(expected_message, output_message);
        EXPECT_EQ(true, response.isarray());
        EXPECT_EQ(2, response.length());
        EXPECT_EQ(1, response.num(0));
        EXPECT_EQ(1, response.get(1).isstring());
        EXPECT_STREQ("Went to bed early", response.str(1));
}

TEST_F(romiserialclient_tests, error_reponse_without_message)
{
        // Arrange
        setExpectedOutput("#a:008e\r");
        initInput("#a[1]:0085\r");

        // Act
        JSON response;
        client.send("a", response);
        
        //Assert
        EXPECT_EQ(expected_message, output_message);
        EXPECT_EQ(true, response.isarray());
        EXPECT_EQ(2, response.length());
        EXPECT_EQ(1, response.num(0));
        EXPECT_EQ(1, response.get(1).isstring());
}

TEST_F(romiserialclient_tests, log_message)
{
        // Arrange
        setExpectedOutput("#a:008e\r");
        initInput("#!LOG MESSAGE:008e\r#a[0]:00e7\r");

        // Act
        JSON response;
        client.send("a", response);
        
        //Assert
        EXPECT_EQ(expected_message, output_message);
        EXPECT_EQ(true, response.isarray());
        EXPECT_EQ(1, response.length());
        EXPECT_EQ(0, response.num(0));
}

TEST_F(romiserialclient_tests, message_too_long)
{
        // Arrange

        // Act
        JSON response;

        client.send("a[\"0123456789012345678901234567890123456789"
                    "012345678901234567890123456789\"]",
                    response);
        
        //Assert
        EXPECT_EQ(true, response.isarray());
        EXPECT_EQ(2, response.length());
        EXPECT_EQ(romiserialclient_too_long, response.num(0));
}

TEST_F(romiserialclient_tests, error_number_has_string_representation)
{
        // Arrange

        // Act

        //Assert
        EXPECT_STREQ("No error", RomiSerialClient::get_error_message(0));
        EXPECT_STREQ("Application error", RomiSerialClient::get_error_message(1));
        EXPECT_STREQ("Unknown error code", RomiSerialClient::get_error_message(-99399));
        
        for (int i = -1; i > romiserial_last_error; i--) {
                EXPECT_EQ(0, rstreq("Unknown error code",
                                    RomiSerialClient::get_error_message(i)));
        }
}

