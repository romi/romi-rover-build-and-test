#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "RPCNavigationServerAdaptor.h"
#include "../mock/mock_navigation.h"

using namespace std;
using namespace testing;
using namespace romi;

class rpcnavigation_tests : public ::testing::Test
{
protected:
        MockNavigation navigation;
        
	rpcnavigation_tests() {
	}

	~rpcnavigation_tests() override = default;

	void SetUp() override {
	}

	void TearDown() override {
	}
};

TEST_F(rpcnavigation_tests, test_execute_invalid_command_returns_error)
{
        RPCNavigationServerAdaptor adaptor(navigation);

        JSON params;
        JSON result;
        rcom::RPCError error;
        
        adaptor.execute(0, params, result, error);

        ASSERT_NE(error.code, 0);
        ASSERT_NE(error.message.length(), 0);
}

TEST_F(rpcnavigation_tests, test_execute_unknown_command_returns_error)
{
        RPCNavigationServerAdaptor adaptor(navigation);

        JSON params;
        JSON result;
        rcom::RPCError error;

        adaptor.execute("dummy", params, result, error);

        ASSERT_NE(error.code, 0);
        ASSERT_NE(error.message.length(), 0);
}

TEST_F(rpcnavigation_tests, test_move_command_returns_ok)
{
        RPCNavigationServerAdaptor adaptor(navigation);

        JSON params = JSON::parse("{'distance':1,'speed':0.1}");
        JSON result;
        rcom::RPCError error;

        EXPECT_CALL(navigation, move(1.0, 0.1))
                .WillOnce(Return(true));

        adaptor.execute("move", params, result, error);

        ASSERT_EQ(error.code, 0);
}

TEST_F(rpcnavigation_tests, test_move_command_returns_error_on_missing_parameters_1)
{
        RPCNavigationServerAdaptor adaptor(navigation);

        JSON params = JSON::parse("{'command':'move'}");
        JSON result;
        rcom::RPCError error;

        adaptor.execute("move", params, result, error);

        ASSERT_NE(error.code, 0);
        ASSERT_NE(error.message.length(), 0);
}

TEST_F(rpcnavigation_tests, test_move_command_returns_error_on_missing_parameters_2)
{
        RPCNavigationServerAdaptor adaptor(navigation);

        JSON params = JSON::parse("{'command':'move','distance':0}");
        JSON result;
        rcom::RPCError error;

        adaptor.execute("move", params, result, error);

        ASSERT_NE(error.code, 0);
        ASSERT_NE(error.message.length(), 0);
}

TEST_F(rpcnavigation_tests, test_move_command_returns_error_on_invalid_parameters_1)
{
        RPCNavigationServerAdaptor adaptor(navigation);

        JSON params = JSON::parse("{'command':'move','distance':'foo'}");
        JSON result;
        rcom::RPCError error;

        adaptor.execute("move", params, result, error);

        ASSERT_NE(error.code, 0);
        ASSERT_NE(error.message.length(), 0);
}

TEST_F(rpcnavigation_tests, test_move_command_returns_error_on_invalid_parameters_2)
{
        RPCNavigationServerAdaptor adaptor(navigation);

        JSON params = JSON::parse("{'command':'move','distance':10,'speed':'foo'}");
        JSON result;
        rcom::RPCError error;

        adaptor.execute("move", params, result, error);

        ASSERT_NE(error.code, 0);
        ASSERT_NE(error.message.length(), 0);
}

TEST_F(rpcnavigation_tests, test_move_returns_error_when_move_fails)
{
        RPCNavigationServerAdaptor adaptor(navigation);

        JSON params = JSON::parse("{'command':'move','distance':1,'speed':0.1}");
        JSON result;
        rcom::RPCError error;

        EXPECT_CALL(navigation, move(1.0, 0.1))
                .WillOnce(Return(false));

        adaptor.execute("move", params, result, error);

        ASSERT_NE(error.code, 0);
        ASSERT_NE(error.message.length(), 0);
}





TEST_F(rpcnavigation_tests, test_moveat_returns_ok)
{
        RPCNavigationServerAdaptor adaptor(navigation);

        JSON params = JSON::parse("{'command':'moveat','speed':[0.1,0.2]}");
        JSON result;
        rcom::RPCError error;

        EXPECT_CALL(navigation, moveat(0.1, 0.2))
                .WillOnce(Return(true));

        adaptor.execute("moveat", params, result, error);

        ASSERT_EQ(error.code, 0);
}

TEST_F(rpcnavigation_tests, test_moveat_returns_error_on_missing_parameters_1)
{
        RPCNavigationServerAdaptor adaptor(navigation);

        JSON params = JSON::parse("{'command':'moveat'}");
        JSON result;
        rcom::RPCError error;

        adaptor.execute("moveat", params, result, error);

        ASSERT_NE(error.code, 0);
        ASSERT_NE(error.message.length(), 0);
}

TEST_F(rpcnavigation_tests, test_moveat_returns_error_on_invalid_parameters_1)
{
        RPCNavigationServerAdaptor adaptor(navigation);

        JSON params = JSON::parse("{'command':'moveat','speed':'foo'}");
        JSON result;
        rcom::RPCError error;

        adaptor.execute("moveat", params, result, error);

        ASSERT_NE(error.code, 0);
        ASSERT_NE(error.message.length(), 0);
}

TEST_F(rpcnavigation_tests, test_moveat_returns_error_on_invalid_parameters_2)
{
        RPCNavigationServerAdaptor adaptor(navigation);

        JSON params = JSON::parse("{'command':'moveat','speed':['foo','bar']}");
        JSON result;
        rcom::RPCError error;

        adaptor.execute("moveat", params, result, error);

        ASSERT_NE(error.code, 0);
        ASSERT_NE(error.message.length(), 0);
}

TEST_F(rpcnavigation_tests, test_moveat_returns_error_on_invalid_parameters_3)
{
        RPCNavigationServerAdaptor adaptor(navigation);

        JSON params = JSON::parse("{'command':'moveat','speed':[0.1,'bar']}");
        JSON result;
        rcom::RPCError error;

        adaptor.execute("moveat", params, result, error);

        ASSERT_NE(error.code, 0);
        ASSERT_NE(error.message.length(), 0);
}

TEST_F(rpcnavigation_tests, test_moveat_returns_error_on_invalid_parameters_4)
{
        RPCNavigationServerAdaptor adaptor(navigation);

        JSON params = JSON::parse("{'command':'moveat','speed':['foo',0.2]}");
        JSON result;
        rcom::RPCError error;

        adaptor.execute("moveat", params, result, error);

        ASSERT_NE(error.code, 0);
        ASSERT_NE(error.message.length(), 0);
}

TEST_F(rpcnavigation_tests, test_moveat_returns_error_when_moveat_fails)
{
        RPCNavigationServerAdaptor adaptor(navigation);

        JSON params = JSON::parse("{'command':'moveat','speed':[0.1,0.2]}");
        JSON result;
        rcom::RPCError error;

        EXPECT_CALL(navigation, moveat(0.1, 0.2))
                .WillOnce(Return(false));

        adaptor.execute("moveat", params, result, error);

        ASSERT_NE(error.code, 0);
        ASSERT_NE(error.message.length(), 0);
}



TEST_F(rpcnavigation_tests, test_stop_returns_ok)
{
        RPCNavigationServerAdaptor adaptor(navigation);

        JSON params = JSON::parse("{'command':'stop'}");
        JSON result;
        rcom::RPCError error;

        EXPECT_CALL(navigation, stop())
                .WillOnce(Return(true));

        adaptor.execute("stop", params, result, error);

        ASSERT_EQ(error.code, 0);
}

TEST_F(rpcnavigation_tests, test_stop_returns_error_when_stop_fails)
{
        RPCNavigationServerAdaptor adaptor(navigation);

        JSON params = JSON::parse("{'command':'stop'}");
        JSON result;
        rcom::RPCError error;

        EXPECT_CALL(navigation, stop())
                .WillOnce(Return(false));

        adaptor.execute("stop", params, result, error);

        ASSERT_NE(error.code, 0);
        ASSERT_NE(error.message.length(), 0);
}
