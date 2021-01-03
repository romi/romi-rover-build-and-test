#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "CNCRange.h"

using namespace std;
using namespace testing;
using namespace romi;

class cncrange_tests : public ::testing::Test
{
protected:
        
	cncrange_tests() {
        }

	~cncrange_tests() override = default;

	void SetUp() override {
        }

	void TearDown() override {}
};

TEST_F(cncrange_tests, test_default_constructor)
{
        CNCRange range;

        EXPECT_EQ(range.min.x(), 0.0);
        EXPECT_EQ(range.max.x(), 0.0);
        EXPECT_EQ(range.min.y(), 0.0);
        EXPECT_EQ(range.max.y(), 0.0);
        EXPECT_EQ(range.min.z(), 0.0);
        EXPECT_EQ(range.max.z(), 0.0);
}

TEST_F(cncrange_tests, test_constructor_with_values)
{
        double min[3] = {1, 2, 3};
        double max[3] = {4, 5, 6};
        CNCRange range(min, max);

        EXPECT_EQ(range.min.x(), 1.0);
        EXPECT_EQ(range.max.x(), 4.0);
        EXPECT_EQ(range.min.y(), 2.0);
        EXPECT_EQ(range.max.y(), 5.0);
        EXPECT_EQ(range.min.z(), 3.0);
        EXPECT_EQ(range.max.z(), 6.0);
}

TEST_F(cncrange_tests, test_constructor_using_json)
{
        JsonCpp json = JsonCpp::parse("[[1,4],[2,5],[3,6]]");
        CNCRange range(json);

        EXPECT_EQ(range.min.x(), 1.0);
        EXPECT_EQ(range.max.x(), 4.0);
        EXPECT_EQ(range.min.y(), 2.0);
        EXPECT_EQ(range.max.y(), 5.0);
        EXPECT_EQ(range.min.z(), 3.0);
        EXPECT_EQ(range.max.z(), 6.0);
}

TEST_F(cncrange_tests, constructor_throw_error_when_json_invalid)
{
        JsonCpp json = JsonCpp::parse("[[1,4],[2,5]]");

        try {
                CNCRange range(json);
                FAIL() << "Expected an exception";
        } catch (JSONError& e) {
                // OK
        }
}

TEST_F(cncrange_tests, test_is_inside)
{
        double min[3] = {1, 2, 3};
        double max[3] = {4, 5, 6};
        CNCRange range(min, max);

        v3 p0(min);
        v3 p1(max);
        
        EXPECT_EQ(range.is_inside(p0), true);
        EXPECT_EQ(range.is_inside(p1), true);
        EXPECT_EQ(range.is_inside((p0 + p1) / 2.0), true);
        EXPECT_EQ(range.is_inside(p0 - 0.1), false);
        EXPECT_EQ(range.is_inside(p1 + 0.1), false);
}

TEST_F(cncrange_tests, test_error)
{
        v3 min(1, 2, 3);
        v3 max(4, 5, 6);
        CNCRange range(min, max);

        v3 p0 = max;
        EXPECT_EQ(range.error(p0), 0.0);

        v3 p1 = max + v3(1, 0, 0);        
        EXPECT_EQ(range.error(p1), 1.0);
        p1 = max + v3(0, 1, 0);        
        EXPECT_EQ(range.error(p1), 1.0);
        p1 = max + v3(0, 0, 1);        
        EXPECT_EQ(range.error(p1), 1.0);

        v3 p2 = min;        
        EXPECT_EQ(range.error(p2), 0.0);

        v3 p3 = min - v3(1, 0, 0);        
        EXPECT_EQ(range.error(p3), 1.0);
        p3 = min - v3(0, 1, 0);        
        EXPECT_EQ(range.error(p3), 1.0);
        p3 = min - v3(0, 0, 1);        
        EXPECT_EQ(range.error(p3), 1.0);
}

TEST_F(cncrange_tests, test_clamp)
{
        v3 min(1, 2, 3);
        v3 max(4, 5, 6);
        CNCRange range(min, max);

        v3 p0 = range.clamp(min); 
        EXPECT_EQ(p0 == min, true);

        v3 p1 = range.clamp(max); 
        EXPECT_EQ(p1 == max, true);

        v3 p2 = range.clamp(min - v3(0.01, 0, 0));
        EXPECT_EQ(p2 == min, true);
        p2 = range.clamp(min - v3(0, 0.01, 0));
        EXPECT_EQ(p2 == min, true);
        p2 = range.clamp(min - v3(0, 0, 0.01));
        EXPECT_EQ(p2 == min, true);

        v3 p3 = range.clamp(max + v3(0.01, 0, 0));
        EXPECT_EQ(p3 == max, true);
        p3 = range.clamp(max + v3(0, 0.01, 0));
        EXPECT_EQ(p3 == max, true);
        p3 = range.clamp(max + v3(0, 0, 0.01));
        EXPECT_EQ(p3 == max, true);
}

TEST_F(cncrange_tests, test_dimensions)
{
        v3 min(1, 2, 3);
        v3 max(4, 6, 8);
        CNCRange range(min, max);

        v3 dimensions = range.dimensions();
        EXPECT_EQ(dimensions.x(), 3.0);
        EXPECT_EQ(dimensions.y(), 4.0);
        EXPECT_EQ(dimensions.z(), 5.0);
}
