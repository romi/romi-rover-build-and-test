#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../mock/mock_rpchandler.h"
#include "RPCNavigationClientAdaptor.h"

using namespace std;
using namespace testing;
using namespace romi;

class rpcnavigationclient_tests : public ::testing::Test
{
protected:
        MockRPCHandler handler;
         
	rpcnavigationclient_tests() {
	}

	~rpcnavigationclient_tests() override = default;

	void SetUp() override {
	}

	void TearDown() override {
	}
};

TEST_F(rpcnavigationclient_tests, check_stop_request)
{
        RPCNavigationClientAdaptor adapter(handler);
        
        EXPECT_CALL(handler, exec(_,_));

        adapter.stop();
}
