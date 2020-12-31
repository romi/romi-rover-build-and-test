#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../mock/mock_controller.h"
#include "Oquam.h"
#include "DebugWeedingSession.h"

using namespace std;
using namespace testing;
using namespace romi;

class oquam_tests : public ::testing::Test
{
protected:
        
        int32_t position[3] =  {0, 0, 0};
        
        const double xmin[3] =  {0, 0, 0};
        const double xmax[3] =  {0.5, 0.5, 0};
        const double vmax[3] =  {0.1, 0.1, 0.01};
        const double amax[3] =  {0.2, 0.2, 0.2};
        const double scale[3] = {40000, 40000, 100000};
        const double slice_interval = 0.020;
        CNCRange range;
        MockController controller;
        DebugWeedingSession debug;
        
	oquam_tests() : range(xmin, xmax), debug(".", "oquam_tests") {}

	~oquam_tests() override = default;

	void SetUp() override {
                position[0] = 0;
                position[1] = 0;
                position[2] = 0;
        }

	void TearDown() override {}

        void DefaultSetUp() {
                EXPECT_CALL(controller, get_position(NotNull()))
                        .WillRepeatedly(DoAll(SetArrayArgument<0>(position, position+3),
                                              Return(true)));
                EXPECT_CALL(controller, homing())
                        .WillRepeatedly(Return(true));
                EXPECT_CALL(controller, synchronize(_))
                        .WillRepeatedly(Return(true));
                EXPECT_CALL(controller, move(_,_,_,_))
                        .WillRepeatedly(Return(true));
        }
        
public:
        
        bool get_position(int32_t *p) {
                for (int i = 0; i < 3; i++)
                        p[i] = position[i];
                return true;
        }
        
        bool move(int16_t millis, int16_t x, int16_t y, int16_t z) {
                r_debug("move(%d, %d, %d)", x, y, z);
                position[0] += x;
                position[1] += y;
                position[2] += z;
                return true;
        }
};

TEST_F(oquam_tests, constructor_calls_homing)
{
        EXPECT_CALL(controller, homing())
                .Times(1)
                .WillOnce(Return(true));
        
        Oquam oquam(controller, range, vmax, amax, scale, 0.01, slice_interval);
}

TEST_F(oquam_tests, constructor_throws_exception_when_homing_fails)
{
        EXPECT_CALL(controller, homing())
                .Times(1)
                .WillOnce(Return(false));
        
        try {
                Oquam oquam(controller, range, vmax, amax, scale, 0.01, slice_interval);
                FAIL() << "Excpected a runtime error";
                
        } catch (std::runtime_error& e) {
                // OK
        }
}

TEST_F(oquam_tests, stop_execution_calls_controller_1)
{
        DefaultSetUp();
        EXPECT_CALL(controller, stop_execution())
                .Times(1)
                .WillOnce(Return(true));
        
        Oquam oquam(controller, range, vmax, amax, scale, 0.01, slice_interval);
        bool success = oquam.stop_execution();
        ASSERT_EQ(success, true);
}

TEST_F(oquam_tests, stop_execution_calls_controller_2)
{
        DefaultSetUp();
        EXPECT_CALL(controller, stop_execution())
                .Times(1)
                .WillOnce(Return(false));
        
        Oquam oquam(controller, range, vmax, amax, scale, 0.01, slice_interval);
        bool success = oquam.stop_execution();
        ASSERT_EQ(success, false);
}

TEST_F(oquam_tests, continue_execution_calls_controller_1)
{
        DefaultSetUp();
        EXPECT_CALL(controller, continue_execution())
                .Times(1)
                .WillOnce(Return(true));
        
        Oquam oquam(controller, range, vmax, amax, scale, 0.01, slice_interval);
        bool success = oquam.continue_execution();
        ASSERT_EQ(success, true);
}

TEST_F(oquam_tests, continue_execution_calls_controller_2)
{
        DefaultSetUp();
        EXPECT_CALL(controller, continue_execution())
                .Times(1)
                .WillOnce(Return(false));
        
        Oquam oquam(controller, range, vmax, amax, scale, 0.01, slice_interval);
        bool success = oquam.continue_execution();
        ASSERT_EQ(success, false);
}

TEST_F(oquam_tests, reset_calls_controller_1)
{
        DefaultSetUp();
        EXPECT_CALL(controller, reset())
                .Times(1)
                .WillOnce(Return(true));
        
        Oquam oquam(controller, range, vmax, amax, scale, 0.01, slice_interval);
        bool success = oquam.reset();
        ASSERT_EQ(success, true);
}

TEST_F(oquam_tests, reset_calls_controller_2)
{
        DefaultSetUp();
        EXPECT_CALL(controller, reset())
                .Times(1)
                .WillOnce(Return(false));
        
        Oquam oquam(controller, range, vmax, amax, scale, 0.01, slice_interval);
        bool success = oquam.reset();
        ASSERT_EQ(success, false);
}

TEST_F(oquam_tests, constructor_copies_range)
{
        EXPECT_CALL(controller, homing())
                .Times(1)
                .WillOnce(Return(true));
        
        Oquam oquam(controller, range, vmax, amax, scale, 0.01, slice_interval);

        CNCRange range;
        oquam.get_range(range);
        ASSERT_EQ(range._x[0], xmin[0]);
        ASSERT_EQ(range._x[1], xmax[0]);
        ASSERT_EQ(range._y[0], xmin[1]);
        ASSERT_EQ(range._y[1], xmax[1]);
        ASSERT_EQ(range._z[0], xmin[2]);
        ASSERT_EQ(range._z[1], xmax[2]);
}

TEST_F(oquam_tests, moveto_returns_error_when_speed_is_invalid)
{
        DefaultSetUp();
        
        Oquam oquam(controller, range, vmax, amax, scale, 0.01, slice_interval);
        ASSERT_EQ(oquam.moveto(0.1, 0.0, 0.0, 1.1), false);
        ASSERT_EQ(oquam.moveto(0.1, 0.0, 0.0, -0.1), false);
}

TEST_F(oquam_tests, moveto_returns_error_when_position_is_invalid)
{
        DefaultSetUp();
        
        Oquam oquam(controller, range, vmax, amax, scale, 0.01, slice_interval);
        ASSERT_EQ(oquam.moveto(range._x[0]-0.1, 0.0, 0.0, 0.1), false);
        ASSERT_EQ(oquam.moveto(range._x[1]+0.1, 0.0, 0.0, 0.1), false);
        ASSERT_EQ(oquam.moveto(0.0, range._y[0]-0.1, 0.0, 0.1), false);
        ASSERT_EQ(oquam.moveto(0.0, range._y[1]+0.1, 0.0, 0.1), false);
        ASSERT_EQ(oquam.moveto(0.0, 0.0, range._z[0]-0.1, 0.1), false);
        ASSERT_EQ(oquam.moveto(0.0, 0.0, range._z[1]+0.1, 0.1), false);
}

TEST_F(oquam_tests, returns_false_when_get_position_fails)
{
        EXPECT_CALL(controller, homing())
                .Times(1)
                .WillOnce(Return(true));
        EXPECT_CALL(controller, get_position(_))
                .Times(1)
                .WillOnce(Return(false));
        
        Oquam oquam(controller, range, vmax, amax, scale, 0.01, slice_interval);
        bool success = oquam.moveto(0.1, 0.0, 0.0, 0.3);
        ASSERT_EQ(success, false);        
}

TEST_F(oquam_tests, returns_false_when_moveto_fails)
{
        EXPECT_CALL(controller, homing())
                .Times(1)
                .WillOnce(Return(true));
        EXPECT_CALL(controller, get_position(_))
                .Times(1)
                .WillOnce(Return(true));
        EXPECT_CALL(controller, move(_,_,_,_))
                .Times(1)
                .WillOnce(Return(false));
        
        Oquam oquam(controller, range, vmax, amax, scale, 0.01, slice_interval);
        bool success = oquam.moveto(0.1, 0.0, 0.0, 0.3);
        ASSERT_EQ(success, false);        
}

TEST_F(oquam_tests, returns_false_when_synchronize_fails)
{
        EXPECT_CALL(controller, homing())
                .Times(1)
                .WillOnce(Return(true));
        EXPECT_CALL(controller, get_position(_))
                .Times(1)
                .WillOnce(Return(true));
        EXPECT_CALL(controller, move(_,_,_,_))
                .Times(1)
                .WillOnce(Return(true));
        EXPECT_CALL(controller, synchronize(_))
                .Times(1)
                .WillOnce(Return(false));
        
        Oquam oquam(controller, range, vmax, amax, scale, 0.01, slice_interval);
        bool success = oquam.moveto(0.1, 0.0, 0.0, 0.3);
        ASSERT_EQ(success, false);        
}

TEST_F(oquam_tests, test_oquam_moveto)
{
        InSequence seq;

        EXPECT_CALL(controller, homing())
                .Times(1)
                .WillOnce(Return(true));
        EXPECT_CALL(controller, get_position(_))
                .Times(1)
                .WillOnce(Invoke(this, &oquam_tests::get_position));
        EXPECT_CALL(controller, move(_,_,_,_))
                .Times(1)
                .WillOnce(Invoke(this, &oquam_tests::move));
        EXPECT_CALL(controller, synchronize(_))
                .Times(1)
                .WillOnce(Return(true));
        
        Oquam oquam(controller, range, vmax, amax, scale, 0.01, slice_interval);
        bool success = oquam.moveto(0.1, 0.0, 0.0, 0.3);
        ASSERT_EQ(success, true);
        ASSERT_EQ(position[0], 4000);        
}

TEST_F(oquam_tests, test_oquam_moveto_2)
{
        InSequence seq;

        EXPECT_CALL(controller, homing())
                .Times(1)
                .WillOnce(Return(true));
        for (int i = 0; i < 2; i++) {
                EXPECT_CALL(controller, get_position(_))
                        .Times(1)
                        .WillOnce(Invoke(this, &oquam_tests::get_position));
                EXPECT_CALL(controller, move(_,_,_,_))
                        .Times(1)
                        .WillOnce(Invoke(this, &oquam_tests::move));
                EXPECT_CALL(controller, synchronize(_))
                        .Times(1)
                        .WillOnce(Return(true));
        }
        
        Oquam oquam(controller, range, vmax, amax, scale, 0.01, slice_interval);

        oquam.moveto(0.1, 0.0, 0.0, 0.3);
        bool success = oquam.moveto(0.0, 0.0, 0.0, 0.3);

        ASSERT_EQ(success, true);
        ASSERT_EQ(position[0], 0);        
}

TEST_F(oquam_tests, test_oquam_travel_square)
{
        DefaultSetUp();

        Oquam oquam(controller, range, vmax, amax, scale, 0.03, slice_interval);
        oquam.set_file_cabinet(&debug);

        Path path;
        Waypoint p0(0.1, 0.0);
        Waypoint p1(0.1, 0.1);
        Waypoint p2(0.0, 0.1);
        Waypoint p3(0.0, 0.0);
        path.push_back(p0);
        path.push_back(p1);
        path.push_back(p2);
        path.push_back(p3);

        bool success = oquam.travel(path, 0.3);
        ASSERT_EQ(success, true);
}

TEST_F(oquam_tests, test_oquam_travel_square_fast)
{
        DefaultSetUp();

        Oquam oquam(controller, range, vmax, amax, scale, 0.03, slice_interval);
        oquam.set_file_cabinet(&debug);

        Path path;
        Waypoint p0(0.1, 0.0);
        Waypoint p1(0.1, 0.1);
        Waypoint p2(0.0, 0.1);
        Waypoint p3(0.0, 0.0);
        path.push_back(p0);
        path.push_back(p1);
        path.push_back(p2);
        path.push_back(p3);

        bool success = oquam.travel(path, 1.0);
        ASSERT_EQ(success, true);
}

TEST_F(oquam_tests, test_oquam_travel_snake)
{
        DefaultSetUp();

        Oquam oquam(controller, range, vmax, amax, scale, 0.005, slice_interval);
        oquam.set_file_cabinet(&debug);

        Path path;
        for (int i = 1; i <= 1; i++) {
                Waypoint p0(i * 0.01, (i-1) * 0.01);
                path.push_back(p0);
                Waypoint p1(i * 0.01, i * 0.01);
                path.push_back(p1);
        }
        
        Waypoint p(0.0, 0.0);
        path.push_back(p);

        bool success = oquam.travel(path, 1.0);
        ASSERT_EQ(success, true);
}

TEST_F(oquam_tests, test_oquam_travel_round_trip)
{
        DefaultSetUp();

        Oquam oquam(controller, range, vmax, amax, scale, 0.005, slice_interval);
        oquam.set_file_cabinet(&debug);

        Path path;
        path.push_back(Waypoint(0.0, 0.0));
        path.push_back(Waypoint(0.1, 0.0));
        path.push_back(Waypoint(0.0, 0.0));
        
        bool success = oquam.travel(path, 1.0);
        ASSERT_EQ(success, true);
}

TEST_F(oquam_tests, test_oquam_travel_collinear)
{
        DefaultSetUp();

        Oquam oquam(controller, range, vmax, amax, scale, 0.005, slice_interval);
        oquam.set_file_cabinet(&debug);

        Path path;
        path.push_back(Waypoint(0.0, 0.0));
        path.push_back(Waypoint(0.1, 0.0));
        path.push_back(Waypoint(0.2, 0.0));
        path.push_back(Waypoint(0.0, 0.0));
        
        bool success = oquam.travel(path, 1.0);
        ASSERT_EQ(success, true);
}

TEST_F(oquam_tests, test_oquam_travel_large_displacement)
{
        DefaultSetUp();

        Oquam oquam(controller, range, vmax, amax, scale, 0.005, slice_interval);
        oquam.set_file_cabinet(&debug);

        Path path;
        path.push_back(Waypoint(0.0, 0.0));
        path.push_back(Waypoint(0.1, 0.0));
        path.push_back(Waypoint(0.1, 0.07));
        path.push_back(Waypoint(0.2, 0.07));
        path.push_back(Waypoint(0.0, 0.0));
        
        bool success = oquam.travel(path, 1.0);
        ASSERT_EQ(success, true);
}

TEST_F(oquam_tests, test_oquam_travel_small_displacement)
{
        DefaultSetUp();

        Oquam oquam(controller, range, vmax, amax, scale, 0.005, slice_interval);
        oquam.set_file_cabinet(&debug);

        Path path;
        path.push_back(Waypoint(0.0, 0.0));
        path.push_back(Waypoint(0.1, 0.0));
        path.push_back(Waypoint(0.1, 0.04));
        path.push_back(Waypoint(0.2, 0.04));
        path.push_back(Waypoint(0.0, 0.0));
        
        bool success = oquam.travel(path, 1.0);
        ASSERT_EQ(success, true);
}

TEST_F(oquam_tests, test_oquam_travel_tiny_displacement)
{
        DefaultSetUp();

        Oquam oquam(controller, range, vmax, amax, scale, 0.005, slice_interval);
        oquam.set_file_cabinet(&debug);

        Path path;
        path.push_back(Waypoint(0.0, 0.0));
        path.push_back(Waypoint(0.1, 0.0));
        path.push_back(Waypoint(0.1, 0.005));
        path.push_back(Waypoint(0.2, 0.005));
        path.push_back(Waypoint(0.0, 0.0));
        
        bool success = oquam.travel(path, 1.0);
        ASSERT_EQ(success, true);
}

TEST_F(oquam_tests, test_oquam_travel_zigzag)
{
        DefaultSetUp();

        Oquam oquam(controller, range, vmax, amax, scale, 0.005, slice_interval);
        oquam.set_file_cabinet(&debug);

        Path path;
        Waypoint p(0.0, 0.0);
        
        for (int i = 1; i <= 3; i++) {
                p.y += 0.01;
                path.push_back(p);
                
                p.x += 0.1;
                path.push_back(p);
                
                p.y += 0.01;
                path.push_back(p);

                p.x -= 0.1;
                path.push_back(p);
        }
        
        path.push_back(Waypoint(0.0, 0.0));

        bool success = oquam.travel(path, 1.0);
        ASSERT_EQ(success, true);
}


