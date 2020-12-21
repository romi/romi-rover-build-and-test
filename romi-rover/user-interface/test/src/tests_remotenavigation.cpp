#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../mock/mock_rpchandler.h"
#include "RemoteNavigation.h"

using namespace std;
using namespace testing;
using namespace romi;

class remotenavigation_tests : public ::testing::Test
{
protected:
        MockRPCHandler rpc_handler;
         
	remotenavigation_tests() {
	}

	~remotenavigation_tests() override = default;

	void SetUp() override {
	}

	void TearDown() override {
	}
};

// TEST_F(remotenavigation_tests, stop_returns_true_when_request_succeeds)
// {
//         RemoteNavigation adapter(rpc_handler);
        
//         EXPECT_CALL(rpc_handler, execute(_,_,_,_))
//                 .Times(1);

//         bool success = adapter.stop();

//         ASSERT_EQ(success, true);
// }

// TEST_F(remotenavigation_tests, stop_returns_false_when_request_fails)
// {
//         RemoteNavigation adapter(rpc_handler);
        
//         EXPECT_CALL(rpc_handler, execute(_,_,_,_))
//                 .Times(1);
//         EXPECT_CALL(rpc_handler, is_status_ok(_))
//                 .WillOnce(Return(false));
//         EXPECT_CALL(rpc_handler, get_error_message(_))
//                 .WillOnce(Return("TEST"));

//         bool success = adapter.stop();

//         ASSERT_EQ(success, false);
// }
