#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../mock/mock_rpchandler.h"
#include "rpc/RemoteNavigation.h"
#include "rpc/MethodsRover.h"

using namespace std;
using namespace testing;
using namespace romi;

class remotenavigation_tests : public ::testing::Test
{
protected:
        MockRPCHandler rpc_handler;
        rcom::RPCError return_error;
        std::string sent_method;
        JsonCpp sent_params;
        
	remotenavigation_tests() {
	}

	~remotenavigation_tests() override = default;

	void SetUp() override {
	}

	void TearDown() override {
	}

public:
        void set_error(const char *method, JsonCpp& params,
                       JsonCpp& result, rcom::RPCError& error) {
                r_debug("method=%s", method);
                sent_method = method;
                sent_params = params;
                error.code = return_error.code;
                error.message = return_error.message;
        }
};

TEST_F(remotenavigation_tests, stop_returns_true_when_request_succeeds)
{
        EXPECT_CALL(rpc_handler, execute(_,_,_,_))
                .WillOnce(Invoke(this, &remotenavigation_tests::set_error));
        return_error.code = 0;
        return_error.message = "";
        
        RemoteNavigation adapter(rpc_handler);
        bool success = adapter.stop();

        ASSERT_STREQ(sent_method.c_str(), MethodsNavigation::stop);
        ASSERT_EQ(success, true);
}

TEST_F(remotenavigation_tests, stop_returns_false_when_request_fails)
{
        EXPECT_CALL(rpc_handler, execute(_,_,_,_))
                .WillOnce(Invoke(this, &remotenavigation_tests::set_error));        
        return_error.code = 1;
        return_error.message = "MESSAGE";
        
        RemoteNavigation adapter(rpc_handler);
        bool success = adapter.stop();
        
        ASSERT_STREQ(sent_method.c_str(), MethodsNavigation::stop);
        ASSERT_EQ(success, false);
}

TEST_F(remotenavigation_tests, moveat_sends_correct_args_and_returns_true)
{
        EXPECT_CALL(rpc_handler, execute(_,_,_,_))
                .WillOnce(Invoke(this, &remotenavigation_tests::set_error));        
        return_error.code = 0;
        return_error.message = "";
        
        RemoteNavigation adapter(rpc_handler);
        bool success = adapter.moveat(0.3, 0.4);
        
        ASSERT_STREQ(sent_method.c_str(), MethodsNavigation::moveat);
        ASSERT_EQ(success, true);
        ASSERT_EQ(sent_params.array("speed").num(0), 0.3);
        ASSERT_EQ(sent_params.array("speed").num(1), 0.4);
}

TEST_F(remotenavigation_tests, moveat_returns_false_when_request_fails)
{
        EXPECT_CALL(rpc_handler, execute(_,_,_,_))
                .WillOnce(Invoke(this, &remotenavigation_tests::set_error));        
        return_error.code = 1;
        return_error.message = "MESSAGE";
        
        RemoteNavigation adapter(rpc_handler);
        bool success = adapter.moveat(0.3, 0.3);
        
        ASSERT_STREQ(sent_method.c_str(), MethodsNavigation::moveat);
        ASSERT_EQ(success, false);
}

TEST_F(remotenavigation_tests, move_sends_correct_args_and_returns_true)
{
        EXPECT_CALL(rpc_handler, execute(_,_,_,_))
                .WillOnce(Invoke(this, &remotenavigation_tests::set_error));        
        return_error.code = 0;
        return_error.message = "";
        
        RemoteNavigation adapter(rpc_handler);
        bool success = adapter.move(0.7, 0.1);
        
        ASSERT_STREQ(sent_method.c_str(), MethodsNavigation::move);
        ASSERT_EQ(success, true);
        ASSERT_EQ(sent_params.num("distance"), 0.7);
        ASSERT_EQ(sent_params.num("speed"), 0.1);
}

TEST_F(remotenavigation_tests, move_returns_false_when_request_fails)
{
        EXPECT_CALL(rpc_handler, execute(_,_,_,_))
                .WillOnce(Invoke(this, &remotenavigation_tests::set_error));        
        return_error.code = 1;
        return_error.message = "MESSAGE";
        
        RemoteNavigation adapter(rpc_handler);
        bool success = adapter.move(0.3, 0.3);
        
        ASSERT_STREQ(sent_method.c_str(), MethodsNavigation::move);
        ASSERT_EQ(success, false);
}
