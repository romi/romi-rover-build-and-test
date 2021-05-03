#include <string>
#include <unistd.h>
#include <sys/syscall.h>

#include "gtest/gtest.h"

#include "util.h"

extern "C" {
#include "log.mock.h"
}

class utils_tests : public ::testing::Test {
protected:
    utils_tests() = default;

    ~utils_tests() override = default;

    void SetUp() override {
        RESET_FAKE(r_err);
        RESET_FAKE(r_warn);
    }

    void TearDown() override {
    }
};

TEST_F(utils_tests, rprintf_with_long_enough_buffer_prints_to_buffer)
{
    // Arrange
    const int buffsize = 128;
    char buffer[buffsize];
    memset(buffer, 0, buffsize);
    std::string expected("string 10 string 0x01");

    // Act
    char * res = rprintf(buffer, buffsize, "%s %d %s %#04x", "string", 10, "string", 1);
    std::string actual(buffer);

    // Assert
    ASSERT_EQ(res, buffer);
    ASSERT_EQ(actual, expected);
}

TEST_F(utils_tests, rprintf_with_small_buffer_returns_null)
{
    // Arrange
    const int buffsize = 6;
    char buffer[buffsize];
    memset(buffer, 0, buffsize);
    std::string expected("string 10 string 0x01");

    // Act
    char * res = rprintf(buffer, buffsize, "%s %d %s %#04x", "string", 10, "string", 1);
    std::string actual(buffer);

    // Assert
    ASSERT_EQ(res, nullptr);
}


TEST_F(utils_tests, rrandom_returns_filled_buffer)
{
    // Arrange
    const int buffsize = 16;
    char buffer[buffsize+1];
    memset(buffer, 0, buffsize+1);

    // Act
    long int actual = r_random(buffer, buffsize);
    std::string actual_string = buffer;


    // Assert
    ASSERT_EQ(actual, buffsize);
}

TEST_F(utils_tests, rrandom_returns_filled_buffer_correct_size)
{
    // Arrange
    const int buffsize = 4;
    char buffer[buffsize+1];
    memset(buffer, 0, buffsize+1);

    // Act
    long int actual = r_random(buffer, buffsize);
    std::string actual_string;
    actual_string = buffer;

    // Assert
    ASSERT_EQ(actual, buffsize);
}

TEST_F(utils_tests, ruuid_returns_correct_length_string)
{
    // Arrange
    int actualsize = (16*2) + 4; // 4 '-' chars.
    // Act
    char *actual = r_uuid();
    std::string actual_string(actual);

    // Assert
    ASSERT_EQ(actual_string.size(), actualsize);
    free(actual);
}