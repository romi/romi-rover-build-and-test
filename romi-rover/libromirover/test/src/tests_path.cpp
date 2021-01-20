#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "api/Path.h"

using namespace std;
using namespace testing;
using namespace romi;

class path_tests : public ::testing::Test
{
protected:
        
	path_tests() {
        }

	~path_tests() override = default;

	void SetUp() override {
        }

	void TearDown() override {}
};

TEST_F(path_tests, test_constructor)
{
        Path path;
        EXPECT_EQ(path.size(), 0);
}


TEST_F(path_tests, test_set_z)
{
        Path path;
        path.push_back(v3(1, 2, 3));
        path.set_z(4);
        
        EXPECT_EQ(path.size(), 1);
        EXPECT_EQ(path[0].x(), 1.0);
        EXPECT_EQ(path[0].y(), 2.0);
        EXPECT_EQ(path[0].z(), 4.0);
}

TEST_F(path_tests, test_invert_y)
{
        Path path;
        path.push_back(v3(0.1, 0.2, 0.3));
        path.invert_y();
        
        EXPECT_EQ(path.size(), 1);
        EXPECT_EQ(path[0].x(), 0.1);
        EXPECT_EQ(path[0].y(), 0.8);
        EXPECT_EQ(path[0].z(), 0.3);
}

TEST_F(path_tests, test_closest_point)
{
        Path path;
        path.push_back(v3(0.0, 0.0, 0.0));
        path.push_back(v3(0.1, 0.1, 0.0));
        path.push_back(v3(0.2, 0.2, 0.0));

        v3 p0(0.0, 0.0, 0.0);
        v3 p1(0.01, 0.01, 0.0);
        v3 p2(0.11, 0.11, 0.0);
        v3 p3(0.19, 0.19, 0.0);
        
        EXPECT_EQ(path.size(), 3);
        EXPECT_EQ(path.closest_point(p0), 0);
        EXPECT_EQ(path.closest_point(p1), 0);
        EXPECT_EQ(path.closest_point(p2), 1);
        EXPECT_EQ(path.closest_point(p3), 2);
}


TEST_F(path_tests, test_scale)
{
        Path path;
        path.push_back(v3(1, 2, 3));
        path.scale(2);
        
        EXPECT_EQ(path.size(), 1);
        EXPECT_EQ(path[0].x(), 2.0);
        EXPECT_EQ(path[0].y(), 4.0);
        EXPECT_EQ(path[0].z(), 6.0);
}

TEST_F(path_tests, test_translate)
{
        Path path;
        path.push_back(v3(1, 2, 3));
        path.translate(v3(1, 2, 3));
        
        EXPECT_EQ(path.size(), 1);
        EXPECT_EQ(path[0].x(), 2.0);
        EXPECT_EQ(path[0].y(), 4.0);
        EXPECT_EQ(path[0].z(), 6.0);
}

TEST_F(path_tests, test_rotate_0)
{
        Path path;
        path.push_back(v3(0, 0, 0));
        path.push_back(v3(1, 1, 1));
        path.push_back(v3(2, 2, 2));

        Path out;
        path.rotate(out, 0);
        
        EXPECT_EQ(path.size(), 3);
        
        EXPECT_EQ(path[0].x(), 0);
        EXPECT_EQ(path[0].y(), 0);
        EXPECT_EQ(path[0].z(), 0);

        EXPECT_EQ(path[1].x(), 1);
        EXPECT_EQ(path[1].y(), 1);
        EXPECT_EQ(path[1].z(), 1);

        EXPECT_EQ(path[2].x(), 2);
        EXPECT_EQ(path[2].y(), 2);
        EXPECT_EQ(path[2].z(), 2);

        
        EXPECT_EQ(out.size(), 3);

        EXPECT_EQ(out[0].x(), 0);
        EXPECT_EQ(out[0].y(), 0);
        EXPECT_EQ(out[0].z(), 0);

        EXPECT_EQ(out[1].x(), 1);
        EXPECT_EQ(out[1].y(), 1);
        EXPECT_EQ(out[1].z(), 1);

        EXPECT_EQ(out[2].x(), 2);
        EXPECT_EQ(out[2].y(), 2);
        EXPECT_EQ(out[2].z(), 2);
}

TEST_F(path_tests, test_rotate_1)
{
        Path path;
        path.push_back(v3(0, 0, 0));
        path.push_back(v3(1, 1, 1));
        path.push_back(v3(2, 2, 2));

        Path out;
        path.rotate(out, 1);
        
        EXPECT_EQ(path.size(), 3);
        
        EXPECT_EQ(path[0].x(), 0);
        EXPECT_EQ(path[0].y(), 0);
        EXPECT_EQ(path[0].z(), 0);

        EXPECT_EQ(path[1].x(), 1);
        EXPECT_EQ(path[1].y(), 1);
        EXPECT_EQ(path[1].z(), 1);

        EXPECT_EQ(path[2].x(), 2);
        EXPECT_EQ(path[2].y(), 2);
        EXPECT_EQ(path[2].z(), 2);

        
        EXPECT_EQ(out.size(), 3);

        EXPECT_EQ(out[0].x(), 1);
        EXPECT_EQ(out[0].y(), 1);
        EXPECT_EQ(out[0].z(), 1);

        EXPECT_EQ(out[1].x(), 2);
        EXPECT_EQ(out[1].y(), 2);
        EXPECT_EQ(out[1].z(), 2);

        EXPECT_EQ(out[2].x(), 0);
        EXPECT_EQ(out[2].y(), 0);
        EXPECT_EQ(out[2].z(), 0);
}

TEST_F(path_tests, test_rotate_2)
{
        Path path;
        path.push_back(v3(0, 0, 0));
        path.push_back(v3(1, 1, 1));
        path.push_back(v3(2, 2, 2));

        Path out;
        path.rotate(out, 2);
        
        EXPECT_EQ(path.size(), 3);
        
        EXPECT_EQ(path[0].x(), 0);
        EXPECT_EQ(path[0].y(), 0);
        EXPECT_EQ(path[0].z(), 0);

        EXPECT_EQ(path[1].x(), 1);
        EXPECT_EQ(path[1].y(), 1);
        EXPECT_EQ(path[1].z(), 1);

        EXPECT_EQ(path[2].x(), 2);
        EXPECT_EQ(path[2].y(), 2);
        EXPECT_EQ(path[2].z(), 2);

        
        EXPECT_EQ(out.size(), 3);

        EXPECT_EQ(out[0].x(), 2);
        EXPECT_EQ(out[0].y(), 2);
        EXPECT_EQ(out[0].z(), 2);

        EXPECT_EQ(out[1].x(), 0);
        EXPECT_EQ(out[1].y(), 0);
        EXPECT_EQ(out[1].z(), 0);

        EXPECT_EQ(out[2].x(), 1);
        EXPECT_EQ(out[2].y(), 1);
        EXPECT_EQ(out[2].z(), 1);
}

TEST_F(path_tests, test_clamp_1)
{
        double min[3] = {0, 0, 0};
        double max[3] = {1, 1, 1};
        CNCRange range(min, max);
        
        Path path;
        path.push_back(v3(-0.1, 0.0, 0.0));

        EXPECT_EQ(path.clamp(range, 0.1), true);
        EXPECT_EQ(path.size(), 1);
        EXPECT_EQ(path[0].x(), 0.0);
}

TEST_F(path_tests, test_clamp_2)
{
        double min[3] = {0, 0, 0};
        double max[3] = {1, 1, 1};
        CNCRange range(min, max);
        
        Path path;
        path.push_back(v3(-0.1, 0.0, 0.0));

        EXPECT_EQ(path.clamp(range, 0.099), false);
}
