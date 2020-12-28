#include <string>
#include <iostream>
#include <thread>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "Section.h"

using namespace std;
using namespace testing;
using namespace romi;

class section_tests : public ::testing::Test
{
protected:
        
        const bool debug_romi_serial = true;
        
        const double xmin[3] =  {0, 0, 0};
        const double xmax[3] =  {1.0, 2.0, 3.0};
        const double vmax[3] =  {3.0, 2.0, 1.0};
        const double amax[3] =  {1.0, 1.0, 1.0};
        const double slice_interval = 0.020;

        double p0[3] = {0.0, 0.0, 0.0};
        double p1[3] = {1.0, 0.0, 0.0};
        double v0[3] = {0.5, 0.0, 0.0};
        double v1[3] = {1.5, 0.0, 0.0};
        double a[3] =  {1.0, 0.0, 0.0};
        
	section_tests() {
        }

	~section_tests() override = default;

	void SetUp() override {}

	void TearDown() override {}
};

TEST_F(section_tests, test_constructor)
{
        Section section(1.0, 2.0, p0, p1, v0, v1, a);

        ASSERT_EQ(section.duration, 1.0);
        ASSERT_EQ(section.at, 2.0);

        for (int i = 0; i < 3; i++) {
                ASSERT_EQ(section.p0[i], p0[i]);
                ASSERT_EQ(section.p1[i], p1[i]);
                ASSERT_EQ(section.v0[i], v0[i]);
                ASSERT_EQ(section.v1[i], v1[i]);
                ASSERT_EQ(section.a[i], a[i]);
        }
}

TEST_F(section_tests, test_get_speed_at_1)
{
        Section section(1.0, 2.0, p0, p1, v0, v1, a);

        double v[3];
        section.get_speed_at(1.0, v);

        ASSERT_EQ(v[0], 1.5);
        ASSERT_EQ(v[1], 0.0);
        ASSERT_EQ(v[2], 0.0);
}

TEST_F(section_tests, test_is_valid_1)
{
        Section section(1.0, 2.0, p0, p1, v0, v1, a);
        bool valid = section.is_valid("test", 120.0, xmin, xmax, vmax,  amax);
        ASSERT_EQ(valid, true);
}

TEST_F(section_tests, test_is_valid_fails_1)
{
        Section section(1.0, 2.0, p0, p1, v0, v1, a);

        section.p0[0] = xmax[0] + 0.1;
        ASSERT_EQ(false, section.is_valid("test", 120.0, xmin, xmax, vmax,  amax));
        section.p0[0] = p0[0];

        section.p0[0] = xmin[0] - 1.0;
        ASSERT_EQ(false, section.is_valid("test", 120.0, xmin, xmax, vmax,  amax));
        section.p0[0] = p0[0];
        
        section.p0[1] = xmax[1] + 0.1;
        ASSERT_EQ(false, section.is_valid("test", 120.0, xmin, xmax, vmax,  amax));
        section.p0[1] = p0[1];

        section.p0[1] = xmin[1] - 1.0;
        ASSERT_EQ(false, section.is_valid("test", 120.0, xmin, xmax, vmax,  amax));
        section.p0[1] = p0[1];

        section.p0[2] = xmax[2] + 0.1;
        ASSERT_EQ(false, section.is_valid("test", 120.0, xmin, xmax, vmax,  amax));
        section.p0[2] = p0[2];

        section.p0[2] = xmin[2] - 1.0;
        ASSERT_EQ(false, section.is_valid("test", 120.0, xmin, xmax, vmax,  amax));
        section.p0[2] = p0[2];


        section.p1[0] = xmax[0] + 0.1;
        ASSERT_EQ(false, section.is_valid("test", 120.0, xmin, xmax, vmax,  amax));
        section.p1[0] = p1[0];

        section.p1[0] = xmin[0] - 1.0;
        ASSERT_EQ(false, section.is_valid("test", 120.0, xmin, xmax, vmax,  amax));
        section.p1[0] = p1[0];
        
        section.p1[1] = xmax[1] + 0.1;
        ASSERT_EQ(false, section.is_valid("test", 120.0, xmin, xmax, vmax,  amax));
        section.p1[1] = p1[1];

        section.p1[1] = xmin[1] - 1.0;
        ASSERT_EQ(false, section.is_valid("test", 120.0, xmin, xmax, vmax,  amax));
        section.p1[1] = p1[1];

        section.p1[2] = xmax[2] + 0.1;
        ASSERT_EQ(false, section.is_valid("test", 120.0, xmin, xmax, vmax,  amax));
        section.p1[2] = p1[2];

        section.p1[2] = xmin[2] - 1.0;
        ASSERT_EQ(false, section.is_valid("test", 120.0, xmin, xmax, vmax,  amax));
        section.p1[2] = p1[2];
}

TEST_F(section_tests, test_is_valid_fails_2)
{
        Section section(1.0, 2.0, p0, p1, v0, v1, a);

        section.v0[0] = vmax[0] + 1.0;
        ASSERT_EQ(false, section.is_valid("test", 120.0, xmin, xmax, vmax,  amax));
        section.v0[0] = v0[0];

        section.v0[1] = vmax[1] + 1.0;
        ASSERT_EQ(false, section.is_valid("test", 120.0, xmin, xmax, vmax,  amax));
        section.v0[1] = v0[1];

        section.v0[2] = vmax[2] + 1.0;
        ASSERT_EQ(false, section.is_valid("test", 120.0, xmin, xmax, vmax,  amax));
        section.v0[2] = v0[2];

        
        section.v1[0] = vmax[0] + 1.0;
        ASSERT_EQ(false, section.is_valid("test", 120.0, xmin, xmax, vmax,  amax));
        section.v1[0] = v1[0];

        section.v1[1] = vmax[1] + 1.0;
        ASSERT_EQ(false, section.is_valid("test", 120.0, xmin, xmax, vmax,  amax));
        section.v1[1] = v1[1];

        section.v1[2] = vmax[2] + 1.0;
        ASSERT_EQ(false, section.is_valid("test", 120.0, xmin, xmax, vmax,  amax));
        section.v1[2] = v1[2];

        
        section.v0[0] = -vmax[0] - 1.0;
        ASSERT_EQ(false, section.is_valid("test", 120.0, xmin, xmax, vmax,  amax));
        section.v0[0] = v0[0];

        section.v0[1] = -vmax[1] - 1.0;
        ASSERT_EQ(false, section.is_valid("test", 120.0, xmin, xmax, vmax,  amax));
        section.v0[1] = v0[1];

        section.v0[2] = -vmax[2] - 1.0;
        ASSERT_EQ(false, section.is_valid("test", 120.0, xmin, xmax, vmax,  amax));
        section.v0[2] = v0[2];

        
        section.v1[0] = -vmax[0] - 1.0;
        ASSERT_EQ(false, section.is_valid("test", 120.0, xmin, xmax, vmax,  amax));
        section.v1[0] = v1[0];

        section.v1[1] = -vmax[1] - 1.0;
        ASSERT_EQ(false, section.is_valid("test", 120.0, xmin, xmax, vmax,  amax));
        section.v1[1] = v1[1];

        section.v1[2] = -vmax[2] - 1.0;
        ASSERT_EQ(false, section.is_valid("test", 120.0, xmin, xmax, vmax,  amax));
        section.v1[2] = v1[2];
}

TEST_F(section_tests, test_is_valid_fails_3)
{
        Section section(1.0, 2.0, p0, p1, v0, v1, a);

        section.a[0] = amax[0] + 1.0;
        ASSERT_EQ(false, section.is_valid("test", 120.0, xmin, xmax, vmax,  amax));
        section.a[0] = a[0];

        section.a[1] = amax[1] + 1.0;
        ASSERT_EQ(false, section.is_valid("test", 120.0, xmin, xmax, vmax,  amax));
        section.a[1] = a[1];

        section.a[2] = amax[2] + 1.0;
        ASSERT_EQ(false, section.is_valid("test", 120.0, xmin, xmax, vmax,  amax));
        section.a[2] = a[2];

        
        section.a[0] = -amax[0] - 1.0;
        ASSERT_EQ(false, section.is_valid("test", 120.0, xmin, xmax, vmax,  amax));
        section.a[0] = a[0];

        section.a[1] = -amax[1] - 1.0;
        ASSERT_EQ(false, section.is_valid("test", 120.0, xmin, xmax, vmax,  amax));
        section.a[1] = a[1];

        section.a[2] = -amax[2] - 1.0;
        ASSERT_EQ(false, section.is_valid("test", 120.0, xmin, xmax, vmax,  amax));
        section.a[2] = a[2];
}

TEST_F(section_tests, test_is_valid_fails_4)
{
        Section section(1.0, 2.0, p0, p1, v0, v1, a);

        section.a[0] = a[0] / 2.0;
        ASSERT_EQ(false, section.is_valid("test", 120.0, xmin, xmax, vmax,  amax));
}
