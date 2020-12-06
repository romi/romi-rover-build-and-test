#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <string>
#include <vector>
#include <RomiSerial.h>
#include <RomiSerialErrors.h>
#include <Parser.h>
#include <CRC8.h>
#include "../mock/mock_inputstream.h"
#include "../mock/mock_outputstream.h"

using namespace std;
using namespace testing;



void handler_without_args(RomiSerial *romi_serial, int16_t *args, const char *string_arg)
{
        romi_serial->send_ok();
}

void handler_fails_to_send_message(RomiSerial *romi_serial, int16_t *args, const char *string_arg)
{
}

void handler_with_two_arguments(RomiSerial *romi_serial, int16_t *args, const char *string_arg)
{
        romi_serial->send_ok();
}

void handler_with_string_arg(RomiSerial *romi_serial, int16_t *args, const char *string_arg)
{
        romi_serial->send_ok();
}

void handler_returning_values(RomiSerial *romi_serial, int16_t *args, const char *string_arg)
{
        romi_serial->send("[0,1,2]");
}

void handler_returning_error(RomiSerial *romi_serial, int16_t *args, const char *string_arg)
{
        romi_serial->send_error(101, "I will try again tomorrow");
}

const static MessageHandler handlers[] = {
        { 'a', 0, false, handler_without_args },
        { 'b', 0, false, handler_fails_to_send_message },
        { 'c', 2, false, handler_with_two_arguments },
        { 'd', 0, true,  handler_with_string_arg },
        { 'e', 0, false, handler_returning_values },
        { 'f', 0, false, handler_returning_error }
};

int num_handlers = sizeof(handlers) / sizeof(MessageHandler);



class romiserial_tests : public ::testing::Test
{
protected:
        MockInputStream in;
        MockOutputStream out;
        RomiSerial serial;
        string output_message;
        string expected_message;
        
	romiserial_tests() : serial(&in, &out, handlers, num_handlers) {}

	~romiserial_tests() override = default;

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

        void initSerialAvailable(const char *s) {
                InSequence seq;
                int len = strlen(s);
                for (int i = 0; i <= len; i++) {
                        EXPECT_CALL(in, available)
                                .WillOnce(Return(len - i))
                                .RetiresOnSaturation();
                }
        }
        
        void initInputs(const char *s) {
                initSerialAvailable(s);
                initSerialRead(s);
        }

        bool append_output(char c) { 
                output_message += c; 
                return true; 
        }

        void initAppendOutput() {
                EXPECT_CALL(out, write(_))
                        .WillRepeatedly(Invoke(this, &romiserial_tests::append_output));
        }
        
        char hex(uint8_t value) {
                value &= 0x0f;
                return (value < 10)? '0' + value : 'a' + (value - 10);
        }

        void expectErrorMessage(char opcode, uint8_t id, int error, const char *message = 0) {
                initAppendOutput();                
                CRC8 crc;
                expected_message += "#";
                expected_message += opcode;
                expected_message += "[";
                expected_message += std::to_string(error);
                if (message) {
                        expected_message += ",\"";
                        expected_message += message;
                        expected_message += "\"";
                }
                expected_message += "]:";
                expected_message += hex(id >> 4);
                expected_message += hex(id);
                uint8_t code = crc.compute(expected_message.c_str(), expected_message.length());
                expected_message += hex(code >> 4);
                expected_message += hex(code);
                expected_message += "\r";
                expected_message += "\n";
        }
        
        void expectOKMessage(char opcode, uint8_t id) {
                initAppendOutput();                
                CRC8 crc;
                expected_message += "#";
                expected_message += opcode;
                expected_message += "[0]:";
                expected_message += hex(id >> 4);
                expected_message += hex(id);
                uint8_t code = crc.compute(expected_message.c_str(), expected_message.length());
                expected_message += hex(code >> 4);
                expected_message += hex(code);
                expected_message += "\r";
                expected_message += "\n";
        }
        
        void expectMessage(char opcode, uint8_t id, const char *s) {
                initAppendOutput();                
                CRC8 crc;
                expected_message += "#";
                expected_message += opcode;
                expected_message += s;
                expected_message += ":";
                expected_message += hex(id >> 4);
                expected_message += hex(id);
                uint8_t code = crc.compute(expected_message.c_str(), expected_message.length());
                expected_message += hex(code >> 4);
                expected_message += hex(code);
                expected_message += "\r";
                expected_message += "\n";
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

TEST_F(romiserial_tests, romiserial_test_handler_without_args)
{
        // Arrange
        initInputs("#a\r");
        expectOKMessage('a', 0);

        // Act
        serial.handle_input();
        
        //Assert
        EXPECT_EQ(expected_message, output_message);
}

TEST_F(romiserial_tests, romiserial_test_handler_with_id)
{
        // Arrange
        initInputs("#a:1295\r");
        expectOKMessage('a', 0x12);

        // Act
        serial.handle_input();
        
        //Assert
        EXPECT_EQ(expected_message, output_message);
}

TEST_F(romiserial_tests, romiserial_test_handler_with_crc_mismatch)
{
        // Arrange
        initInputs("#a:1200\r");
        expectErrorMessage('_', 0x12, romiserial_envelope_crc_mismatch);

        // Act
        serial.handle_input();
        
        //Assert
        EXPECT_EQ(expected_message, output_message);
}

TEST_F(romiserial_tests, romiserial_test_bad_number_of_arguments)
{
        // Arrange
        initInputs("#a[1]\r");
        expectErrorMessage('a', 0, romiserial_bad_number_of_arguments);

        // Act
        serial.handle_input();
        
        //Assert
        EXPECT_EQ(expected_message, output_message);
}

TEST_F(romiserial_tests, romiserial_test_string_when_none_required)
{
        // Arrange
        initInputs("#a[\"Start worrying, details follow\"]\r");
        expectErrorMessage('a', 0, romiserial_bad_string);

        // Act
        serial.handle_input();
        
        //Assert
        EXPECT_EQ(expected_message, output_message);
}

TEST_F(romiserial_tests, romiserial_test_string_too_long)
{
        // Arrange
        initInputs("#a[\"Start worrying. Details to follow.\"]\r");
        expectErrorMessage('a', 0, romiserial_string_too_long);

        // Act
        serial.handle_input();
        
        //Assert
        EXPECT_EQ(expected_message, output_message);
}

TEST_F(romiserial_tests, romiserial_test_unknown_opcode)
{
        // Arrange
        initInputs("#z\r");
        expectErrorMessage('z', 0, romiserial_unknown_opcode);

        // Act
        serial.handle_input();
        
        //Assert
        EXPECT_EQ(expected_message, output_message);
}

TEST_F(romiserial_tests, romiserial_test_handler_fails_to_send_message)
{
        // Arrange
        initInputs("#b\r");
        expectErrorMessage('b', 0, romiserial_bad_handler);

        // Act
        serial.handle_input();
        
        //Assert
        EXPECT_EQ(expected_message, output_message);
}

TEST_F(romiserial_tests, romiserial_test_handler_with_bad_number_of_arguments_1)
{
        // Arrange
        initInputs("#c\r");
        expectErrorMessage('c', 0, romiserial_bad_number_of_arguments);

        // Act
        serial.handle_input();
        
        //Assert
        EXPECT_EQ(expected_message, output_message);
}

TEST_F(romiserial_tests, romiserial_test_handler_with_bad_number_of_arguments_2)
{
        // Arrange
        initInputs("#c[1]\r");
        expectErrorMessage('c', 0, romiserial_bad_number_of_arguments);

        // Act
        serial.handle_input();
        
        //Assert
        EXPECT_EQ(expected_message, output_message);
}

TEST_F(romiserial_tests, romiserial_test_handler_with_bad_number_of_arguments_3)
{
        // Arrange
        initInputs("#c[1,2,3]\r");
        expectErrorMessage('c', 0, romiserial_bad_number_of_arguments);

        // Act
        serial.handle_input();
        
        //Assert
        EXPECT_EQ(expected_message, output_message);
}

TEST_F(romiserial_tests, romiserial_test_handler_with_two_arguments)
{
        // Arrange
        initInputs("#c[1,2]\r");
        expectOKMessage('c', 0);

        // Act
        serial.handle_input();
        
        //Assert
        EXPECT_EQ(expected_message, output_message);
}

TEST_F(romiserial_tests, romiserial_test_handler_with_string_argument)
{
        // Arrange
        initInputs("#d[\"Do it once, perfectly\"]\r");
        expectOKMessage('d', 0);

        // Act
        serial.handle_input();
        
        //Assert
        EXPECT_EQ(expected_message, output_message);
}

TEST_F(romiserial_tests, romiserial_test_handler_with_string_and_extra_argument)
{
        // Arrange
        initInputs("#d[\"Do it correctly or don't do it\",1]\r");
        expectErrorMessage('d', 0, romiserial_bad_number_of_arguments);

        // Act
        serial.handle_input();
        
        //Assert
        EXPECT_EQ(expected_message, output_message);
}

TEST_F(romiserial_tests, romiserial_test_handler_that_returns_values)
{
        // Arrange
        initInputs("#e\r");
        expectMessage('e', 0, "[0,1,2]");

        // Act
        serial.handle_input();
        
        //Assert
        EXPECT_EQ(expected_message, output_message);
}

TEST_F(romiserial_tests, romiserial_test_handler_that_returns_an_error)
{
        // Arrange
        initInputs("#f:32dd\r");
        expectErrorMessage('f', 0x32, 101, "I will try again tomorrow");

        // Act
        serial.handle_input();
        
        //Assert
        EXPECT_EQ(expected_message, output_message);
}

