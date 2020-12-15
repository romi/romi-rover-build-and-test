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

        JSON request = JSON::parse("{}");
        JSON reply;

        adaptor.execute(request, reply);

        ASSERT_STREQ("error", reply.str("status"));
        ASSERT_EQ(true, reply.str("message") != 0);
}

TEST_F(rpcnavigation_tests, test_execute_unknown_command_returns_error)
{
        RPCNavigationServerAdaptor adaptor(navigation);

        JSON request = JSON::parse("{'command':'foo'}");
        JSON reply;

        adaptor.execute(request, reply);

        ASSERT_STREQ("error", reply.str("status"));
        ASSERT_EQ(true, reply.str("message") != 0);
}



TEST_F(rpcnavigation_tests, test_move_command_returns_ok)
{
        RPCNavigationServerAdaptor adaptor(navigation);

        JSON request = JSON::parse("{'command':'move','distance':1,'speed':0.1}");
        JSON reply;

        EXPECT_CALL(navigation, move(1.0, 0.1))
                .WillOnce(Return(true));

        adaptor.execute(request, reply);

        ASSERT_STREQ("ok", reply.str("status"));
}

TEST_F(rpcnavigation_tests, test_move_command_returns_error_on_missing_parameters_1)
{
        RPCNavigationServerAdaptor adaptor(navigation);

        JSON request = JSON::parse("{'command':'move'}");
        JSON reply;

        adaptor.execute(request, reply);

        ASSERT_STREQ("error", reply.str("status"));
        ASSERT_EQ(true, reply.str("message") != 0);
}

TEST_F(rpcnavigation_tests, test_move_command_returns_error_on_missing_parameters_2)
{
        RPCNavigationServerAdaptor adaptor(navigation);

        JSON request = JSON::parse("{'command':'move','distance':0}");
        JSON reply;

        adaptor.execute(request, reply);

        ASSERT_STREQ("error", reply.str("status"));
        ASSERT_EQ(true, reply.str("message") != 0);
}

TEST_F(rpcnavigation_tests, test_move_command_returns_error_on_invalid_parameters_1)
{
        RPCNavigationServerAdaptor adaptor(navigation);

        JSON request = JSON::parse("{'command':'move','distance':'foo'}");
        JSON reply;

        adaptor.execute(request, reply);

        ASSERT_STREQ("error", reply.str("status"));
        ASSERT_EQ(true, reply.str("message") != 0);
}

TEST_F(rpcnavigation_tests, test_move_command_returns_error_on_invalid_parameters_2)
{
        RPCNavigationServerAdaptor adaptor(navigation);

        JSON request = JSON::parse("{'command':'move','distance':10,'speed':'foo'}");
        JSON reply;

        adaptor.execute(request, reply);

        ASSERT_STREQ("error", reply.str("status"));
        ASSERT_EQ(true, reply.str("message") != 0);
}

TEST_F(rpcnavigation_tests, test_move_returns_error_when_move_fails)
{
        RPCNavigationServerAdaptor adaptor(navigation);

        JSON request = JSON::parse("{'command':'move','distance':1,'speed':0.1}");
        JSON reply;

        EXPECT_CALL(navigation, move(1.0, 0.1))
                .WillOnce(Return(false));

        adaptor.execute(request, reply);

        ASSERT_STREQ("error", reply.str("status"));
        ASSERT_EQ(true, reply.str("message") != 0);
}





TEST_F(rpcnavigation_tests, test_moveat_returns_ok)
{
        RPCNavigationServerAdaptor adaptor(navigation);

        JSON request = JSON::parse("{'command':'moveat','speed':[0.1,0.2]}");
        JSON reply;

        EXPECT_CALL(navigation, moveat(0.1, 0.2))
                .WillOnce(Return(true));

        adaptor.execute(request, reply);

        ASSERT_STREQ("ok", reply.str("status"));
}

TEST_F(rpcnavigation_tests, test_moveat_returns_error_on_missing_parameters_1)
{
        RPCNavigationServerAdaptor adaptor(navigation);

        JSON request = JSON::parse("{'command':'moveat'}");
        JSON reply;

        adaptor.execute(request, reply);

        ASSERT_STREQ("error", reply.str("status"));
        ASSERT_EQ(true, reply.str("message") != 0);
}

TEST_F(rpcnavigation_tests, test_moveat_returns_error_on_invalid_parameters_1)
{
        RPCNavigationServerAdaptor adaptor(navigation);

        JSON request = JSON::parse("{'command':'moveat','speed':'foo'}");
        JSON reply;

        adaptor.execute(request, reply);

        ASSERT_STREQ("error", reply.str("status"));
        ASSERT_EQ(true, reply.str("message") != 0);
}

TEST_F(rpcnavigation_tests, test_moveat_returns_error_on_invalid_parameters_2)
{
        RPCNavigationServerAdaptor adaptor(navigation);

        JSON request = JSON::parse("{'command':'moveat','speed':['foo','bar']}");
        JSON reply;

        adaptor.execute(request, reply);

        ASSERT_STREQ("error", reply.str("status"));
        ASSERT_EQ(true, reply.str("message") != 0);
}

TEST_F(rpcnavigation_tests, test_moveat_returns_error_on_invalid_parameters_3)
{
        RPCNavigationServerAdaptor adaptor(navigation);

        JSON request = JSON::parse("{'command':'moveat','speed':[0.1,'bar']}");
        JSON reply;

        adaptor.execute(request, reply);

        ASSERT_STREQ("error", reply.str("status"));
        ASSERT_EQ(true, reply.str("message") != 0);
}

TEST_F(rpcnavigation_tests, test_moveat_returns_error_on_invalid_parameters_4)
{
        RPCNavigationServerAdaptor adaptor(navigation);

        JSON request = JSON::parse("{'command':'moveat','speed':['foo',0.2]}");
        JSON reply;

        adaptor.execute(request, reply);

        ASSERT_STREQ("error", reply.str("status"));
        ASSERT_EQ(true, reply.str("message") != 0);
}

TEST_F(rpcnavigation_tests, test_moveat_returns_error_when_moveat_fails)
{
        RPCNavigationServerAdaptor adaptor(navigation);

        JSON request = JSON::parse("{'command':'moveat','speed':[0.1,0.2]}");
        JSON reply;

        EXPECT_CALL(navigation, moveat(0.1, 0.2))
                .WillOnce(Return(false));

        adaptor.execute(request, reply);

        ASSERT_STREQ("error", reply.str("status"));
        ASSERT_EQ(true, reply.str("message") != 0);
}



TEST_F(rpcnavigation_tests, test_stop_returns_ok)
{
        RPCNavigationServerAdaptor adaptor(navigation);

        JSON request = JSON::parse("{'command':'stop'}");
        JSON reply;

        EXPECT_CALL(navigation, stop())
                .WillOnce(Return(true));

        adaptor.execute(request, reply);

        ASSERT_STREQ("ok", reply.str("status"));
}

TEST_F(rpcnavigation_tests, test_stop_returns_error_when_stop_fails)
{
        RPCNavigationServerAdaptor adaptor(navigation);

        JSON request = JSON::parse("{'command':'stop'}");
        JSON reply;

        EXPECT_CALL(navigation, stop())
                .WillOnce(Return(false));

        adaptor.execute(request, reply);

        ASSERT_STREQ("error", reply.str("status"));
        ASSERT_EQ(true, reply.str("message") != 0);
}
