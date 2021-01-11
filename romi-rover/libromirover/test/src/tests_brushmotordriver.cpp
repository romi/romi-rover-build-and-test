#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "BrushMotorDriver.h"
#include "../mock/mock_romiserialclient.h"

using namespace std;
using namespace testing;
using namespace romi;

class brushmotordriver_tests : public ::testing::Test
{
protected:
        MockRomiSerialClient serial;
        vector<string> expected_output;
        vector<string> observed_output;
        vector<string> mock_response;
        JsonCpp driver_config;
        int encoder_steps;
        double maximum_revolutions_per_second;
        
	brushmotordriver_tests() {
                const char * config_string = "{"
                        "'maximum_signal_amplitude': 71,"
                        "'use_pid': false,"
                        "'pid': {'kp': 1.1, 'ki': 2.2, 'kd': 3.3},"
                        "'encoder_directions': {'left': -1, 'right': 1 }}";
                driver_config = JsonCpp::parse(config_string);
                encoder_steps = 123;
                maximum_revolutions_per_second = 1.7;
	}

	~brushmotordriver_tests() override = default;

	void SetUp() override {
	}

	void TearDown() override {
	}

        void append_output(const char *s, JsonCpp& response) {
                size_t index = observed_output.size();
                observed_output.push_back(s);
                response = JsonCpp::parse(mock_response[index].c_str());
        }

        void add_expected_output(const char *command, const char *response) {
                expected_output.push_back(command);
                mock_response.push_back(response);
                EXPECT_CALL(serial, send(_,_))
                        .WillOnce(Invoke(this, &brushmotordriver_tests::append_output))
                        .RetiresOnSaturation();
        }
};

TEST_F(brushmotordriver_tests, parse_config)
{
        BrushMotorDriverSettings settings;

        settings.parse(driver_config);
        
        //Assert
        ASSERT_EQ(settings.max_signal, 71);
        ASSERT_EQ(settings.use_pid, false);
        ASSERT_EQ(settings.kp, 1.1);
        ASSERT_EQ(settings.ki, 2.2);
        ASSERT_EQ(settings.kd, 3.3);
        ASSERT_EQ(settings.dir_left, -1);
        ASSERT_EQ(settings.dir_right, 1);
}

TEST_F(brushmotordriver_tests, successful_config_and_enable)
{
        add_expected_output("C[123,170,71,0,1100,2200,3300,-1,1]", "[0]");
        add_expected_output("E[1]", "[0]");
        
        BrushMotorDriver driver(serial, driver_config, encoder_steps,
                                maximum_revolutions_per_second);

        ASSERT_EQ(expected_output.size(), observed_output.size());
        for (size_t i = 0; i < expected_output.size(); i++) {
                ASSERT_STREQ(observed_output[i].c_str(),
                             expected_output[i].c_str());
        }
}

TEST_F(brushmotordriver_tests, throws_exception_at_failed_config)
{
        add_expected_output("C[123,170,71,0,1100,2200,3300,-1,1]", "[1]");

        try {
                BrushMotorDriver driver(serial, driver_config, encoder_steps,
                                        maximum_revolutions_per_second);
                FAIL() << "Expected std::runtime_error";
        }  catch (std::runtime_error const &e) {
                EXPECT_STREQ(e.what(), "BrushMotorDriver: Initialization failed");
        } catch (...) {
                FAIL() << "Expected std::runtime_error";
        }
}

TEST_F(brushmotordriver_tests, throws_exception_at_failed_enable)
{
        add_expected_output("C[123,170,71,0,1100,2200,3300,-1,1]", "[0]");
        add_expected_output("E[1]", "[1]");

        try {
                BrushMotorDriver driver(serial, driver_config, encoder_steps,
                                        maximum_revolutions_per_second);
                FAIL() << "Expected std::runtime_error";
        }  catch (std::runtime_error const &e) {
                
                EXPECT_STREQ(e.what(), "BrushMotorDriver: Initialization failed");
                ASSERT_EQ(expected_output.size(), observed_output.size());
                ASSERT_STREQ(observed_output[0].c_str(),
                             expected_output[0].c_str());

        } catch (...) {
                FAIL() << "Expected std::runtime_error";
        }
}

TEST_F(brushmotordriver_tests, returns_true_on_successful_moveat)
{
        add_expected_output("C[123,170,71,0,1100,2200,3300,-1,1]", "[0]");
        add_expected_output("E[1]", "[0]");
        add_expected_output("V[100,200]", "[0]");

        BrushMotorDriver driver(serial, driver_config, encoder_steps,
                                maximum_revolutions_per_second);
        bool success = driver.moveat(0.1, 0.2);
        
        ASSERT_EQ(expected_output.size(), observed_output.size());
        for (size_t i = 0; i < expected_output.size(); i++) {
                ASSERT_STREQ(observed_output[i].c_str(),
                             expected_output[i].c_str());
        }
        ASSERT_EQ(success, true);
}

TEST_F(brushmotordriver_tests, returns_false_on_unsuccessful_moveat)
{
        add_expected_output("C[123,170,71,0,1100,2200,3300,-1,1]", "[0]");
        add_expected_output("E[1]", "[0]");
        add_expected_output("V[100,200]", "[1,'Just fooling you']");
        
        BrushMotorDriver driver(serial, driver_config, encoder_steps,
                                maximum_revolutions_per_second);
        bool success = driver.moveat(0.1, 0.2);
        
        ASSERT_EQ(expected_output.size(), observed_output.size());
        for (size_t i = 0; i < expected_output.size(); i++) {
                ASSERT_STREQ(observed_output[i].c_str(),
                             expected_output[i].c_str());
        }
        
        ASSERT_EQ(success, false);
}

TEST_F(brushmotordriver_tests, returns_true_on_successful_get_encoders)
{
        add_expected_output("C[123,170,71,0,1100,2200,3300,-1,1]", "[0]");
        add_expected_output("E[1]", "[0]");
        add_expected_output("e", "[0,100,200,300]");

        BrushMotorDriver driver(serial, driver_config, encoder_steps,
                                maximum_revolutions_per_second);
        double left, right, timestamp;
        bool success = driver.get_encoder_values(left, right, timestamp);
        
        ASSERT_EQ(expected_output.size(), observed_output.size());
        for (size_t i = 0; i < expected_output.size(); i++) {
                ASSERT_STREQ(observed_output[i].c_str(),
                             expected_output[i].c_str());
        }
        ASSERT_EQ(success, true);
        ASSERT_EQ(left, 100.0);
        ASSERT_EQ(right, 200.0);
        ASSERT_EQ(timestamp, 0.300);
}

TEST_F(brushmotordriver_tests, returns_false_on_unsuccessful_get_encoders)
{
        add_expected_output("C[123,170,71,0,1100,2200,3300,-1,1]", "[0]");
        add_expected_output("E[1]", "[0]");
        add_expected_output("e", "[1,'TEST']");

        BrushMotorDriver driver(serial, driver_config, encoder_steps,
                                maximum_revolutions_per_second);
        double left, right, timestamp;
        bool success = driver.get_encoder_values(left, right, timestamp);
        
        ASSERT_EQ(expected_output.size(), observed_output.size());
        for (size_t i = 0; i < expected_output.size(); i++) {
                ASSERT_STREQ(observed_output[i].c_str(),
                             expected_output[i].c_str());
        }
        ASSERT_EQ(success, false);
}

TEST_F(brushmotordriver_tests, returns_false_on_invalid_speeds_1)
{
        add_expected_output("C[123,170,71,0,1100,2200,3300,-1,1]", "[0]");
        add_expected_output("E[1]", "[0]");

        BrushMotorDriver driver(serial, driver_config, encoder_steps,
                                maximum_revolutions_per_second);
        bool success = driver.moveat(2.0, 0.0);
        
        ASSERT_EQ(expected_output.size(), observed_output.size());
        for (size_t i = 0; i < expected_output.size(); i++) {
                ASSERT_STREQ(observed_output[i].c_str(),
                             expected_output[i].c_str());
        }
        ASSERT_EQ(success, false);
}

TEST_F(brushmotordriver_tests, returns_false_on_invalid_speeds_2)
{
        add_expected_output("C[123,170,71,0,1100,2200,3300,-1,1]", "[0]");
        add_expected_output("E[1]", "[0]");

        BrushMotorDriver driver(serial, driver_config, encoder_steps,
                                maximum_revolutions_per_second);
        bool success = driver.moveat(-2.0, 0.0);
        
        ASSERT_EQ(expected_output.size(), observed_output.size());
        for (size_t i = 0; i < expected_output.size(); i++) {
                ASSERT_STREQ(observed_output[i].c_str(),
                             expected_output[i].c_str());
        }
        ASSERT_EQ(success, false);
}

TEST_F(brushmotordriver_tests, returns_false_on_invalid_speeds_3)
{
        add_expected_output("C[123,170,71,0,1100,2200,3300,-1,1]", "[0]");
        add_expected_output("E[1]", "[0]");

        BrushMotorDriver driver(serial, driver_config, encoder_steps,
                                maximum_revolutions_per_second);
        bool success = driver.moveat(0.0, -1.1);
        
        ASSERT_EQ(expected_output.size(), observed_output.size());
        for (size_t i = 0; i < expected_output.size(); i++) {
                ASSERT_STREQ(observed_output[i].c_str(),
                             expected_output[i].c_str());
        }
        ASSERT_EQ(success, false);
}

TEST_F(brushmotordriver_tests, returns_false_on_invalid_speeds_4)
{
        add_expected_output("C[123,170,71,0,1100,2200,3300,-1,1]", "[0]");
        add_expected_output("E[1]", "[0]");

        BrushMotorDriver driver(serial, driver_config, encoder_steps,
                                maximum_revolutions_per_second);
        bool success = driver.moveat(0.0, 1.1);
        
        ASSERT_EQ(expected_output.size(), observed_output.size());
        for (size_t i = 0; i < expected_output.size(); i++) {
                ASSERT_STREQ(observed_output[i].c_str(),
                             expected_output[i].c_str());
        }
        ASSERT_EQ(success, false);
}

TEST_F(brushmotordriver_tests, returns_true_on_successful_stop)
{
        add_expected_output("C[123,170,71,0,1100,2200,3300,-1,1]", "[0]");
        add_expected_output("E[1]", "[0]");
        add_expected_output("X", "[0]");

        BrushMotorDriver driver(serial, driver_config, encoder_steps,
                                maximum_revolutions_per_second);
        bool success = driver.stop();
        
        ASSERT_EQ(expected_output.size(), observed_output.size());
        for (size_t i = 0; i < expected_output.size(); i++) {
                ASSERT_STREQ(observed_output[i].c_str(),
                             expected_output[i].c_str());
        }
        ASSERT_EQ(success, true);
}

TEST_F(brushmotordriver_tests, returns_false_on_failed_stop)
{
        add_expected_output("C[123,170,71,0,1100,2200,3300,-1,1]", "[0]");
        add_expected_output("E[1]", "[0]");
        add_expected_output("X", "[1,\"error\"]");

        BrushMotorDriver driver(serial, driver_config, encoder_steps,
                                maximum_revolutions_per_second);
        bool success = driver.stop();
        
        ASSERT_EQ(expected_output.size(), observed_output.size());
        for (size_t i = 0; i < expected_output.size(); i++) {
                ASSERT_STREQ(observed_output[i].c_str(),
                             expected_output[i].c_str());
        }
        ASSERT_EQ(success, false);
}
