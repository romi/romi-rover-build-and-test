#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../mock/mock_rpcclient.h"
#include "RPCNavigationClientAdaptor.h"

using namespace std;
using namespace testing;
using namespace romi;

class rpcnavigationclient_tests : public ::testing::Test
{
protected:
        MockRPCClient rpc_client;
         
	rpcnavigationclient_tests() {
	}

	~rpcnavigationclient_tests() override = default;

	void SetUp() override {
	}

	void TearDown() override {
	}
};

TEST_F(rpcnavigationclient_tests, stop_returns_true_when_request_succeeds)
{
        RPCNavigationClientAdaptor adapter(rpc_client);
        
        EXPECT_CALL(rpc_client, execute(_,_))
                .Times(1);
        EXPECT_CALL(rpc_client, is_status_ok(_))
                .WillOnce(Return(true));

        bool success = adapter.stop();

        ASSERT_EQ(success, true);
}

TEST_F(rpcnavigationclient_tests, stop_returns_false_when_request_fails)
{
        RPCNavigationClientAdaptor adapter(rpc_client);
        
        EXPECT_CALL(rpc_client, execute(_,_))
                .Times(1);
        EXPECT_CALL(rpc_client, is_status_ok(_))
                .WillOnce(Return(false));
        EXPECT_CALL(rpc_client, get_error_message(_))
                .WillOnce(Return("TEST"));

        bool success = adapter.stop();

        ASSERT_EQ(success, false);
}
