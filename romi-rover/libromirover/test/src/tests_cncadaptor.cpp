#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../mock/mock_cnc.h"
#include "rpc/CNCAdaptor.h"
#include "rpc/MethodsCNC.h"
#include "rpc/MethodsActivity.h"

using namespace std;
using namespace testing;
using namespace romi;
using namespace rcom;

class cncadaptor_tests : public ::testing::Test
{
protected:
        const double xmin[3] =  {0, 0, 0};
        const double xmax[3] =  {0.5, 0.5, 0};
        CNCRange range;
        MockCNC cnc;
        
	cncadaptor_tests() : range(xmin, xmax) {}

	~cncadaptor_tests() override = default;

	void SetUp() override {}

	void TearDown() override {}
};

TEST_F(cncadaptor_tests, retuns_error_on_null_method)
{
        JsonCpp params;
        JsonCpp result;
        RPCError error;

        CNCAdaptor adaptor(cnc);
        adaptor.execute(0, params, result, error);

        ASSERT_EQ(error.code, RPCError::MethodNotFound);
        ASSERT_NE(error.message.length(), 0);
}

TEST_F(cncadaptor_tests, retuns_error_on_unknown_method)
{
        JsonCpp params;
        JsonCpp result;
        RPCError error;

        CNCAdaptor adaptor(cnc);
        adaptor.execute("foo", params, result, error);

        ASSERT_EQ(error.code, RPCError::MethodNotFound);
        ASSERT_NE(error.message.length(), 0);
}

TEST_F(cncadaptor_tests, get_range_retuns_no_error_on_success)
{
        JsonCpp params;
        JsonCpp result;
        RPCError error;
        
        EXPECT_CALL(cnc, get_range(_))
                .WillOnce(Return(true));

        CNCAdaptor adaptor(cnc);
        adaptor.execute(MethodsCNC::get_range, params, result, error);

        ASSERT_EQ(error.code, 0);
        ASSERT_EQ(error.message.length(), 0);
}

TEST_F(cncadaptor_tests, get_range_sets_error_on_failure)
{
        JsonCpp params;
        JsonCpp result;
        RPCError error;
        
        EXPECT_CALL(cnc, get_range(_))
                .WillOnce(Return(false));

        CNCAdaptor adaptor(cnc);
        adaptor.execute(MethodsCNC::get_range, params, result, error);

        ASSERT_NE(error.code, 0);
        ASSERT_NE(error.message.length(), 0);
}

TEST_F(cncadaptor_tests, homing_retuns_no_error_on_success)
{
        JsonCpp params;
        JsonCpp result;
        RPCError error;
        
        EXPECT_CALL(cnc, homing())
                .WillOnce(Return(true));

        CNCAdaptor adaptor(cnc);
        adaptor.execute(MethodsCNC::homing, params, result, error);

        ASSERT_EQ(error.code, 0);
        ASSERT_EQ(error.message.length(), 0);
}

TEST_F(cncadaptor_tests, homing_sets_error_on_failure)
{
        JsonCpp params;
        JsonCpp result;
        RPCError error;
        
        EXPECT_CALL(cnc, homing())
                .WillOnce(Return(false));

        CNCAdaptor adaptor(cnc);
        adaptor.execute(MethodsCNC::homing, params, result, error);

        ASSERT_NE(error.code, 0);
        ASSERT_NE(error.message.length(), 0);
}

TEST_F(cncadaptor_tests, stop_retuns_no_error_on_success)
{
        JsonCpp params;
        JsonCpp result;
        RPCError error;
        
        EXPECT_CALL(cnc, pause_activity())
                .WillOnce(Return(true));

        CNCAdaptor adaptor(cnc);
        adaptor.execute(MethodsActivity::activity_pause, params, result, error);

        ASSERT_EQ(error.code, 0);
        ASSERT_EQ(error.message.length(), 0);
}

TEST_F(cncadaptor_tests, stop_sets_error_on_failure)
{
        JsonCpp params;
        JsonCpp result;
        RPCError error;
        
        EXPECT_CALL(cnc, pause_activity())
                .WillOnce(Return(false));

        CNCAdaptor adaptor(cnc);
        adaptor.execute(MethodsActivity::activity_pause, params, result, error);

        ASSERT_NE(error.code, 0);
        ASSERT_NE(error.message.length(), 0);
}

TEST_F(cncadaptor_tests, continue_retuns_no_error_on_success)
{
        JsonCpp params;
        JsonCpp result;
        RPCError error;
        
        EXPECT_CALL(cnc, continue_activity())
                .WillOnce(Return(true));

        CNCAdaptor adaptor(cnc);
        adaptor.execute(MethodsActivity::activity_continue, params, result, error);

        ASSERT_EQ(error.code, 0);
        ASSERT_EQ(error.message.length(), 0);
}

TEST_F(cncadaptor_tests, continue_sets_error_on_failure)
{
        JsonCpp params;
        JsonCpp result;
        RPCError error;
        
        EXPECT_CALL(cnc, continue_activity())
                .WillOnce(Return(false));

        CNCAdaptor adaptor(cnc);
        adaptor.execute(MethodsActivity::activity_continue, params, result, error);

        ASSERT_NE(error.code, 0);
        ASSERT_NE(error.message.length(), 0);
}

TEST_F(cncadaptor_tests, reset_retuns_no_error_on_success)
{
        JsonCpp params;
        JsonCpp result;
        RPCError error;
        
        EXPECT_CALL(cnc, reset_activity())
                .WillOnce(Return(true));

        CNCAdaptor adaptor(cnc);
        adaptor.execute(MethodsActivity::activity_reset, params, result, error);

        ASSERT_EQ(error.code, 0);
        ASSERT_EQ(error.message.length(), 0);
}

TEST_F(cncadaptor_tests, reset_sets_error_on_failure)
{
        JsonCpp params;
        JsonCpp result;
        RPCError error;
        
        EXPECT_CALL(cnc, reset_activity())
                .WillOnce(Return(false));

        CNCAdaptor adaptor(cnc);
        adaptor.execute(MethodsActivity::activity_reset, params, result, error);

        ASSERT_NE(error.code, 0);
        ASSERT_NE(error.message.length(), 0);
}

TEST_F(cncadaptor_tests, spindle_retuns_no_error_on_success)
{
        JsonCpp params = JsonCpp::parse("{'speed': 0}");
        JsonCpp result;
        RPCError error;
        
        EXPECT_CALL(cnc, spindle(0))
                .WillOnce(Return(true));

        CNCAdaptor adaptor(cnc);
        adaptor.execute(MethodsCNC::spindle, params, result, error);

        ASSERT_EQ(error.code, 0);
        ASSERT_EQ(error.message.length(), 0);
}

TEST_F(cncadaptor_tests, spindle_sets_error_on_failure)
{
        JsonCpp params = JsonCpp::parse("{'speed': 0}");
        JsonCpp result;
        RPCError error;
        
        EXPECT_CALL(cnc, spindle(0))
                .WillOnce(Return(false));

        CNCAdaptor adaptor(cnc);
        adaptor.execute(MethodsCNC::spindle, params, result, error);

        ASSERT_NE(error.code, 0);
        ASSERT_NE(error.message.length(), 0);
}

TEST_F(cncadaptor_tests, spindle_sets_error_on_missing_param)
{
        JsonCpp params = JsonCpp::parse("{}");
        JsonCpp result;
        RPCError error;

        CNCAdaptor adaptor(cnc);
        adaptor.execute(MethodsCNC::spindle, params, result, error);

        ASSERT_EQ(error.code, RPCError::InvalidParams);
        ASSERT_NE(error.message.length(), 0);
}

TEST_F(cncadaptor_tests, moveto_retuns_no_error_on_success)
{
        JsonCpp params = JsonCpp::parse("{'x': 1, 'y': 2, 'z': 3, 'speed': 4}");
        JsonCpp result;
        RPCError error;
        
        EXPECT_CALL(cnc, moveto(1,2,3,4))
                .WillOnce(Return(true));

        CNCAdaptor adaptor(cnc);
        adaptor.execute(MethodsCNC::moveto, params, result, error);

        ASSERT_EQ(error.code, 0);
        ASSERT_EQ(error.message.length(), 0);
}

TEST_F(cncadaptor_tests, moveto_sets_error_on_failure)
{
        JsonCpp params = JsonCpp::parse("{'x': 1, 'y': 2, 'z': 3, 'speed': 4}");
        JsonCpp result;
        RPCError error;
        
        EXPECT_CALL(cnc, moveto(1,2,3,4))
                .WillOnce(Return(false));

        CNCAdaptor adaptor(cnc);
        adaptor.execute(MethodsCNC::moveto, params, result, error);

        ASSERT_NE(error.code, 0);
        ASSERT_NE(error.message.length(), 0);
}

TEST_F(cncadaptor_tests, moveto_sets_error_on_missing_params)
{
        JsonCpp params = JsonCpp::parse("{}");
        JsonCpp result;
        RPCError error;

        CNCAdaptor adaptor(cnc);
        adaptor.execute(MethodsCNC::moveto, params, result, error);

        ASSERT_EQ(error.code, RPCError::InvalidParams);
        ASSERT_NE(error.message.length(), 0);
}

TEST_F(cncadaptor_tests, travel_retuns_no_error_on_success)
{
        JsonCpp params = JsonCpp::parse("{'path': [[1,2,3]]}");
        JsonCpp result;
        RPCError error;
        
        EXPECT_CALL(cnc, travel(_,_))
                .WillOnce(Return(true));

        CNCAdaptor adaptor(cnc);
        adaptor.execute(MethodsCNC::travel, params, result, error);

        ASSERT_EQ(error.code, 0);
        ASSERT_EQ(error.message.length(), 0);
}

TEST_F(cncadaptor_tests, travel_sets_error_on_failure)
{
        JsonCpp params = JsonCpp::parse("{'path': []}");
        JsonCpp result;
        RPCError error;
        
        EXPECT_CALL(cnc, travel(_,_))
                .WillOnce(Return(false));

        CNCAdaptor adaptor(cnc);
        adaptor.execute(MethodsCNC::travel, params, result, error);

        ASSERT_NE(error.code, 0);
        ASSERT_NE(error.message.length(), 0);
}

TEST_F(cncadaptor_tests, travel_sets_error_on_missing_params_1)
{
        JsonCpp params = JsonCpp::parse("{}");
        JsonCpp result;
        RPCError error;

        CNCAdaptor adaptor(cnc);
        adaptor.execute(MethodsCNC::travel, params, result, error);

        ASSERT_EQ(error.code, RPCError::InvalidParams);
        ASSERT_NE(error.message.length(), 0);
}

TEST_F(cncadaptor_tests, travel_sets_error_on_missing_params_2)
{
        JsonCpp params = JsonCpp::parse("{'path': [[0,1]]}");
        JsonCpp result;
        RPCError error;

        CNCAdaptor adaptor(cnc);
        adaptor.execute(MethodsCNC::travel, params, result, error);

        ASSERT_EQ(error.code, RPCError::InvalidParams);
        ASSERT_NE(error.message.length(), 0);
}
