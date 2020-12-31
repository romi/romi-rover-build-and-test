#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "Script.h"
#include "print.h"
#include "is_valid.h"

using namespace std;
using namespace testing;
using namespace romi;

class script_tests : public ::testing::Test
{
protected:

        double xmin[3] = { 0.0, 0.0, 0.0};
        double xmax[3] = { 3.0, 2.0, 0.0};
        double vmax[3] = { 1.0, 1.0, 0.0};
        double amax[3] = { 1.0, 1.0, 1.0};
        double deviation = 0.01;
        double period = 0.100;
        double maxlen = 32.0;
        CNCRange range;
        
	script_tests() : range(xmin, xmax) {
	}

	~script_tests() override = default;

	void SetUp() override {
	}

	void TearDown() override {
	}
};

TEST_F(script_tests, test_constructor)
{
        // Arrange
        double start_position[3] = {0, 0, 0};
        Script script(start_position);
        
        //Assert
        ASSERT_EQ(script.segments, (Segment *) 0);
        ASSERT_EQ(script.atdc, (ATDC *) 0);
        ASSERT_EQ(script.slices.size(), 0);
}

TEST_F(script_tests, test_moveto)
{
        // Arrange
        double start_position[3] = {0, 0, 0};
        Script script(start_position);

        script.moveto(1.0, 0.0, 0.0, 1.0);
        script.convert(vmax, amax, deviation, period, maxlen);

        ASSERT_EQ(true, is_valid(script, 120.0, range, vmax, amax));

        //Assert
        ATDC *first = script.atdc;
        ASSERT_NE(first, (ATDC *) 0);
        
        ASSERT_GT(first->accelerate.duration, 0.0);
        ASSERT_EQ(first->accelerate.p0[0], 0.0);
        ASSERT_EQ(first->accelerate.p0[1], 0.0);
        ASSERT_GT(first->accelerate.p1[0], 0.0);
        ASSERT_EQ(first->accelerate.p1[1], 0.0);
        ASSERT_EQ(first->accelerate.v0[0], 0.0);
        ASSERT_EQ(first->accelerate.v0[1], 0.0);
        ASSERT_EQ(first->accelerate.v1[0], 1.0);
        ASSERT_EQ(first->accelerate.v1[1], 0.0);
        ASSERT_EQ(first->accelerate.a[0], 1.0);
        ASSERT_EQ(first->accelerate.a[1], 0.0);

        ASSERT_NEAR(first->travel.duration, 0.0, 0.01);
        ASSERT_EQ(first->travel.p0[0], first->accelerate.p1[0]);
        ASSERT_EQ(first->travel.p0[1], 0.0);
        //ASSERT_GT(first->travel.p1[0], 0.0);
        //ASSERT_EQ(first->travel.p1[1], 0.0);
        ASSERT_EQ(first->travel.v0[0], first->accelerate.v1[0]);
        ASSERT_EQ(first->travel.v0[1], first->accelerate.v1[1]);
        ASSERT_EQ(first->travel.v1[0], 1.0);
        ASSERT_EQ(first->travel.v1[1], 0.0);
        ASSERT_EQ(first->travel.a[0], 0.0);
        ASSERT_EQ(first->travel.a[1], 0.0);

        ASSERT_GT(first->decelerate.duration, 0.0);
        ASSERT_EQ(first->decelerate.p0[0], first->travel.p1[0]);
        ASSERT_EQ(first->decelerate.p0[1], 0.0);
        ASSERT_EQ(first->decelerate.p1[0], 1.0);
        ASSERT_EQ(first->decelerate.p1[1], 0.0);
        ASSERT_EQ(first->decelerate.v0[0], 1.0);
        ASSERT_EQ(first->decelerate.v0[1], 0.0);
        ASSERT_EQ(first->decelerate.v1[0], 0.0);
        ASSERT_EQ(first->decelerate.v1[1], 0.0);
        ASSERT_EQ(first->decelerate.a[0], -1.0);
        ASSERT_EQ(first->decelerate.a[1], 0.0);
        
        ASSERT_EQ(first->curve.duration, 0.0);
        ASSERT_EQ(first->curve.p1[0], 1.0);
        ASSERT_EQ(first->curve.p1[1], 0.0);
        ASSERT_EQ(first->curve.v0[0], 0.0);
        ASSERT_EQ(first->curve.v0[1], 0.0);
        ASSERT_EQ(first->curve.v1[0], 0.0);
        ASSERT_EQ(first->curve.v1[1], 0.0);
        ASSERT_EQ(first->curve.a[0], 0.0);
        ASSERT_EQ(first->curve.a[1], 0.0);

        ATDC *second = first->next;
        ASSERT_EQ(second, (ATDC *) 0);
}

TEST_F(script_tests, test_move_and_back)
{
        // Arrange
        double start_position[3] = {0, 0, 0};
        Script script(start_position);

        script.moveto(1.0, 0.0, 0.0, 1.0);
        script.moveto(0.0, 0.0, 0.0, 1.0);
        script.convert(vmax, amax, deviation, period, maxlen);
        
        //print(script, false);
        
        ASSERT_EQ(true, is_valid(script, 120.0, range, vmax, amax));

        //Assert
        ATDC *first = script.atdc;
        ASSERT_NE(first, (ATDC *) 0);
        
        ASSERT_GT(first->accelerate.duration, 0.0);
        ASSERT_EQ(first->accelerate.p0[0], 0.0);
        ASSERT_EQ(first->accelerate.p0[1], 0.0);
        ASSERT_GT(first->accelerate.p1[0], 0.0);
        ASSERT_EQ(first->accelerate.p1[1], 0.0);
        ASSERT_EQ(first->accelerate.v0[0], 0.0);
        ASSERT_EQ(first->accelerate.v0[1], 0.0);
        ASSERT_NEAR(first->accelerate.v1[0], 1.0, 0.01);
        ASSERT_EQ(first->accelerate.v1[1], 0.0);
        ASSERT_EQ(first->accelerate.a[0], 1.0);
        ASSERT_EQ(first->accelerate.a[1], 0.0);

        ASSERT_NEAR(first->travel.duration, 0.0, 0.01);
        ASSERT_EQ(first->travel.p0[0], first->accelerate.p1[0]);
        ASSERT_EQ(first->travel.p0[1], 0.0);
        ASSERT_GT(first->travel.p1[0], 0.0);
        ASSERT_EQ(first->travel.p1[1], 0.0);
        ASSERT_NEAR(first->travel.v0[0], 1.0, 0.01);
        ASSERT_EQ(first->travel.v0[1], 0.0);
        ASSERT_NEAR(first->travel.v1[0], 1.0, 0.01);
        ASSERT_EQ(first->travel.v1[1], 0.0);
        ASSERT_EQ(first->travel.a[0], 0.0);
        ASSERT_EQ(first->travel.a[1], 0.0);

        ASSERT_GT(first->decelerate.duration, 0.0);
        ASSERT_NEAR(first->decelerate.p0[0], first->travel.p1[0], 0.0001);
        ASSERT_EQ(first->decelerate.p0[1], 0.0);
        // ASSERT_EQ(first->decelerate.p1[0], ???);
        ASSERT_EQ(first->decelerate.p1[1], 0.0);
        ASSERT_NEAR(first->decelerate.v0[0], 1.0, 0.01);
        ASSERT_EQ(first->decelerate.v0[1], 0.0);
        ASSERT_EQ(first->decelerate.a[0], -1.0);
        ASSERT_EQ(first->decelerate.a[1], 0.0);
        
        ASSERT_GT(first->curve.duration, 0.0);
        ASSERT_NEAR(first->curve.p0[0], first->decelerate.p1[0], 0.0001);
        ASSERT_EQ(first->curve.p0[1], 0.0);
        ASSERT_NEAR(first->curve.p1[0], first->curve.p0[0], 0.0001); // symmetric
        ASSERT_EQ(first->curve.p1[1], 0.0);
        ASSERT_EQ(first->curve.a[0], -1.0);
        ASSERT_EQ(first->curve.a[1], 0.0);

        ATDC *second = first->next;
        ASSERT_NE(second, (ATDC *) 0);
        
        ASSERT_GT(second->accelerate.duration, 0.0);
        ASSERT_NEAR(second->accelerate.p0[0], first->curve.p1[0], 0.0001);
        ASSERT_EQ(second->accelerate.p0[1], 0.0);
        ASSERT_LT(second->accelerate.p1[0], 1.0);
        ASSERT_EQ(second->accelerate.p1[1], 0.0);
        ASSERT_NEAR(second->accelerate.v0[0], first->curve.v1[0], 0.0001);
        ASSERT_EQ(second->accelerate.v0[1], 0.0);
        ASSERT_NEAR(second->accelerate.v1[0], -1.0, 0.01);
        ASSERT_EQ(second->accelerate.v1[1], 0.0);
        ASSERT_EQ(second->accelerate.a[0], -1.0);
        ASSERT_EQ(second->accelerate.a[1], 0.0);

        ASSERT_NEAR(first->travel.duration, 0.0, 0.01);
        ASSERT_NEAR(second->travel.p0[0], second->accelerate.p1[0], 0.0001);
        ASSERT_EQ(second->travel.p0[1], 0.0);
        ASSERT_GT(second->travel.p1[0], 0.0);
        ASSERT_EQ(second->travel.p1[1], 0.0);
        ASSERT_NEAR(second->travel.v0[0], second->accelerate.v1[0], 0.0001);
        ASSERT_EQ(second->travel.v0[1], 0.0);
        ASSERT_NEAR(second->travel.v1[0], second->travel.v0[0], 0.0001);
        ASSERT_EQ(second->travel.v1[1], 0.0);
        ASSERT_EQ(second->travel.a[0], 0.0);
        ASSERT_EQ(second->travel.a[1], 0.0);

        ASSERT_GT(second->decelerate.duration, 0.0);
        ASSERT_NEAR(second->decelerate.p0[0], second->travel.p1[0], 0.0001);
        ASSERT_EQ(second->decelerate.p0[1], 0.0);
        ASSERT_EQ(second->decelerate.p1[0], 0.0);
        ASSERT_EQ(second->decelerate.p1[1], 0.0);
        ASSERT_NEAR(second->decelerate.v0[0], second->travel.v1[0], 0.0001);
        ASSERT_EQ(second->decelerate.v0[1], 0.0);
        ASSERT_EQ(second->decelerate.v1[0], 0.0);
        ASSERT_EQ(second->decelerate.v1[1], 0.0);
        ASSERT_EQ(second->decelerate.a[0], 1.0);
        ASSERT_EQ(second->decelerate.a[1], 0.0);
        
        ASSERT_EQ(second->curve.duration, 0.0);
        ASSERT_EQ(second->curve.p0[0], 0.0);
        ASSERT_EQ(second->curve.p0[1], 0.0);
        ASSERT_EQ(second->curve.p1[0], 0.0);
        ASSERT_EQ(second->curve.p1[1], 0.0);
        ASSERT_EQ(second->curve.v0[0], 0.0);
        ASSERT_EQ(second->curve.v0[1], 0.0);
        ASSERT_EQ(second->curve.v1[0], 0.0);
        ASSERT_EQ(second->curve.v1[1], 0.0);
        ASSERT_EQ(second->curve.a[0], 0.0);
        ASSERT_EQ(second->curve.a[1], 0.0);

        ATDC *third = second->next;
        ASSERT_EQ(third, (ATDC *) 0);
}

TEST_F(script_tests, test_move_forward_twice)
{
        // Arrange
        double start_position[3] = {0, 0, 0};
        Script script(start_position);

        script.moveto(1.0, 0.0, 0.0, 1.0);
        script.moveto(2.0, 0.0, 0.0, 1.0);
        script.convert(vmax, amax, deviation, period, maxlen);
        
        ASSERT_EQ(true, is_valid(script, 120.0, range, vmax, amax));
}

TEST_F(script_tests, test_moves_at_90degrees)
{
        // Arrange
        double start_position[3] = {0, 0, 0};
        Script script(start_position);

        script.moveto(0.01, 0.00, 0.0, 1.0);
        script.moveto(0.01, 0.01, 0.0, 1.0);
        script.convert(vmax, amax, deviation, period, maxlen);
        
        ASSERT_EQ(true, is_valid(script, 120.0, range, vmax, amax));

        // ATDC *first = script.atdc;
        // ATDC *second = first->next;

        // printf("#0\n");
        // first->print();
        // printf("#1\n");
        // second->print();
}

TEST_F(script_tests, test_three_small_moves_in_u)
{
        // Arrange
        double start_position[3] = {0, 0, 0};
        Script script(start_position);

        // Three successive short moves are 90°. The first move will
        // only have an acceleration and a curve. The second only a
        // curve. The last one only a curve and a deceleration.
        script.moveto(0.01, 0.00, 0.0, 1.0);
        script.moveto(0.01, 0.01, 0.0, 1.0);
        script.moveto(0.00, 0.01, 0.0, 1.0);
        script.convert(vmax, amax, deviation, period, maxlen);

        //print(script);
        
        ASSERT_EQ(true, is_valid(script, 120.0, range, vmax, amax));

        // ATDC *first = script.atdc;
        // ATDC *second = first->next;
        // ATDC *third = second->next;

        // printf("#0\n");
        // first->print();
        // printf("#1\n");
        // second->print();
        // printf("#2\n");
        // third->print();
}

TEST_F(script_tests, test_reduce_exit_speed)
{
        // Arrange
        double start_position[3] = {0, 0, 0};
        Script script(start_position);

        double test_amax[3] = { 0.5, 0.5, 0.5};

        script.moveto(0.2, 0.00, 0.0, 1.0);
        script.moveto(0.4, 0.10, 0.0, 1.0);
        script.convert(vmax, test_amax, 0.04, period, maxlen);

        //print(script, false);

        ASSERT_EQ(true, is_valid(script, 120.0, range, vmax, test_amax));

        // ATDC *first = script.atdc;
        // ATDC *second = first->next;

        // printf("#0\n");
        // first->print();
        // printf("#1\n");
        // second->print();
}

TEST_F(script_tests, test_reduce_entry_speed)
{
        // Arrange
        double start_position[3] = {0, 0, 0};
        Script script(start_position);

        double test_amax[3] = { 0.5, 0.5, 0.5};

        script.moveto(0.4, 0.00, 0.0, 1.0);
        script.moveto(0.6, 0.10, 0.0, 1.0);
        script.convert(vmax, test_amax, 0.04, period, maxlen);

        // print(script, false);

        ASSERT_EQ(true, is_valid(script, 120.0, range, vmax, test_amax));

        // ATDC *first = script.atdc;
        // ATDC *second = first->next;

        // printf("#0\n");
        // first->print();
        // printf("#1\n");
        // second->print();
}

TEST_F(script_tests, moveto_throws_exception_if_negative_speed_1)
{
        // Arrange
        double start_position[3] = {0, 0, 0};
        Script script(start_position);

        try {
                script.moveto(0.4, 0.00, 0.0, -1.0);
                FAIL() << "Expected a runtime_error";
                
        } catch (std::runtime_error& e) {
                // OK
        } catch (...) {
                FAIL() << "Expected a runtime_error";
        }
}

TEST_F(script_tests, moveto_throws_exception_if_negative_speed_2)
{
        // Arrange
        double start_position[3] = {0, 0, 0};
        Script script(start_position);

        try {
                script.moveto(0.4, 0.00, 0.0, 1.0);
                script.moveto(0.0, 0.00, 0.0, -1.0);
                FAIL() << "Expected a runtime_error";
                
        } catch (std::runtime_error& e) {
                // OK
        } catch (...) {
                FAIL() << "Expected a runtime_error";
        }
}
