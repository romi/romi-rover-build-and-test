#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <string>
#include <vector>
#include <r.h>
#include <RomiSerialClient.h>
#include <RomiSerialErrors.h>
#include <Parser.h>
#include <CRC8.h>
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
        
	romiserialclient_tests() : client(&in, &out) {}

	~romiserialclient_tests() override = default;

	void SetUp() override {
	}

	void TearDown() override {
	}

        void initSerialRead(const char *s) {
                InSequence seq;
                int len = strlen(s);
                for (int i = 0; i < len; i++) {
                        EXPECT_CALL(in, read)
                                .WillOnce(Return(s[i]))
                                .RetiresOnSaturation();;
                }
        }
        
        void initInput(const char *s) {
                //EXPECT_CALL(in, set_timeout(_));
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
        initInput("#a[0]:00e7\r\n");

        // Act
        json_object_t response = client.send("a");
        
        //Assert
        EXPECT_EQ(expected_message, output_message);
        EXPECT_EQ(1, json_isarray(response));
        EXPECT_EQ(1, json_array_length(response));
        EXPECT_EQ(0, json_array_getnum(response, 0));

        json_unref(response);
}

TEST_F(romiserialclient_tests, message_with_args)
{
        // Arrange
        setExpectedOutput("#a[1,2,3]:00dd\r");
        initInput("#a[0]:00e7\r\n");

        // Act
        json_object_t response = client.send("a[1,2,3]");
        
        //Assert
        EXPECT_EQ(expected_message, output_message);
        EXPECT_EQ(1, json_isarray(response));
        EXPECT_EQ(1, json_array_length(response));
        EXPECT_EQ(0, json_array_getnum(response, 0));

        json_unref(response);
}

TEST_F(romiserialclient_tests, error_reponse)
{
        // Arrange
        setExpectedOutput("#a:008e\r");
        initInput("#a[1,\"Went to bed early\"]:00f2\r\n");

        // Act
        json_object_t response = client.send("a");
        
        //Assert
        EXPECT_EQ(expected_message, output_message);
        EXPECT_EQ(1, json_isarray(response));
        EXPECT_EQ(2, json_array_length(response));
        EXPECT_EQ(1, json_array_getnum(response, 0));
        EXPECT_EQ(1, json_isstring(json_array_get(response, 1)));
        EXPECT_STREQ("Went to bed early", json_array_getstr(response, 1));

        json_unref(response);
}

TEST_F(romiserialclient_tests, error_reponse_without_message)
{
        // Arrange
        setExpectedOutput("#a:008e\r");
        initInput("#a[1]:0085\r\n");

        // Act
        json_object_t response = client.send("a");
        
        //Assert
        EXPECT_EQ(expected_message, output_message);
        EXPECT_EQ(1, json_isarray(response));
        EXPECT_EQ(2, json_array_length(response));
        EXPECT_EQ(1, json_array_getnum(response, 0));
        EXPECT_EQ(1, json_isstring(json_array_get(response, 1)));

        json_unref(response);
}

TEST_F(romiserialclient_tests, trailing_message)
{
        // Arrange
        setExpectedOutput("#a:008e\r");
        initInput("#a[0]:ff30\r\n#a[0]:00e7\r\n");

        // Act
        json_object_t response = client.send("a");
        
        //Assert
        EXPECT_EQ(expected_message, output_message);
        EXPECT_EQ(1, json_isarray(response));
        EXPECT_EQ(1, json_array_length(response));
        EXPECT_EQ(0, json_array_getnum(response, 0));

        json_unref(response);
}

TEST_F(romiserialclient_tests, log_message)
{
        // Arrange
        setExpectedOutput("#a:008e\r");
        initInput("!LOG MESSAGE\r\n#a[0]:00e7\r\n");

        // Act
        json_object_t response = client.send("a");
        
        //Assert
        EXPECT_EQ(expected_message, output_message);
        EXPECT_EQ(1, json_isarray(response));
        EXPECT_EQ(1, json_array_length(response));
        EXPECT_EQ(0, json_array_getnum(response, 0));

        json_unref(response);
}

TEST_F(romiserialclient_tests, message_too_long)
{
        // Arrange

        // Act
        json_object_t response = client.send("a[\"0123456789012345678901234567890123456789"
                  "012345678901234567890123456789\"]");
        
        //Assert
        EXPECT_EQ(1, json_isarray(response));
        EXPECT_EQ(2, json_array_length(response));
        EXPECT_EQ(romiserialclient_too_long, json_array_getnum(response, 0));

        json_unref(response);
}

TEST_F(romiserialclient_tests, correctly_handle_print_failure)
{
        // Arrange
        EXPECT_CALL(out, print(_))
                .WillOnce(Return(0));

        // Act
        json_object_t response = client.send("a");
        
        //Assert
        EXPECT_EQ(1, json_isarray(response));
        EXPECT_EQ(2, json_array_length(response));
        EXPECT_EQ(romiserialclient_connection_failed, json_array_getnum(response, 0));

        json_unref(response);
}

