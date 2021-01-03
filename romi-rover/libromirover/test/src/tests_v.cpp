#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "v.h"

using namespace std;
using namespace testing;
using namespace romi;

class v_tests : public ::testing::Test
{
protected:
        
	v_tests() {
        }

	~v_tests() override = default;

	void SetUp() override {
        }

	void TearDown() override {}
};

TEST_F(v_tests, test_operators)
{
        double a[] = {1, 1, 1};
        double b[] = {2, 2, 2};
        double r[3];

        smul(r, a, 2.0);
        ASSERT_EQ(r[0], 2.0);
        ASSERT_EQ(r[1], 2.0);
        ASSERT_EQ(r[2], 2.0);

        sdiv(r, a, 2.0);
        ASSERT_EQ(r[0], 0.5);
        ASSERT_EQ(r[1], 0.5);
        ASSERT_EQ(r[2], 0.5);

        sdiv(r, b, 1.0);
        ASSERT_EQ(r[0], 2.0);
        ASSERT_EQ(r[1], 2.0);
        ASSERT_EQ(r[2], 2.0);

        vadd(r, a, b);
        ASSERT_EQ(r[0], 3.0);
        ASSERT_EQ(r[1], 3.0);
        ASSERT_EQ(r[2], 3.0);

        vsub(r, a, b);
        ASSERT_EQ(r[0], -1.0);
        ASSERT_EQ(r[1], -1.0);
        ASSERT_EQ(r[2], -1.0);

        vmul(r, a, b);
        ASSERT_EQ(r[0], 2.0);
        ASSERT_EQ(r[1], 2.0);
        ASSERT_EQ(r[2], 2.0);

        vdiv(r, a, b);
        ASSERT_EQ(r[0], 0.5);
        ASSERT_EQ(r[1], 0.5);
        ASSERT_EQ(r[2], 0.5);

        ASSERT_EQ(vdot(a, b), 6.0);

        vcross(r, a, b);
        ASSERT_NEAR(r[0], 0.0, 0.0000001);
        ASSERT_NEAR(r[1], 0.0, 0.0000001);
        ASSERT_NEAR(r[2], 0.0, 0.0000001);

        ASSERT_EQ(vmax(a), 1.0);
        ASSERT_EQ(vmax(b), 2.0);
        r[0] = 0.0; r[1] = 1.0; r[2] = 0.0;
        ASSERT_EQ(vmax(r), 1.0);
        r[0] = 0.0; r[1] = 0.0; r[2] = 1.0;
        ASSERT_EQ(vmax(r), 1.0);

        ASSERT_EQ(vmin(a), 1.0);
        ASSERT_EQ(vmin(b), 2.0);
        r[0] = 0.0; r[1] = -1.0; r[2] = 0.0;
        ASSERT_EQ(vmin(r), -1.0);
        r[0] = 0.0; r[1] = 0.0; r[2] = -1.0;
        ASSERT_EQ(vmin(r), -1.0);

        r[0] = 0.0; r[1] = -1.0; r[2] = 0.0;
        vabs(r, r);
        ASSERT_EQ(r[1], 1.0);

        vcopy(r, a);
        ASSERT_EQ(r[0], a[0]);
        ASSERT_EQ(r[1], a[1]);
        ASSERT_EQ(r[2], a[2]);

        vzero(r);
        ASSERT_EQ(r[0], 0.0);
        ASSERT_EQ(r[1], 0.0);
        ASSERT_EQ(r[2], 0.0);

        vset(r, -1.0);
        ASSERT_EQ(r[0], -1.0);
        ASSERT_EQ(r[1], -1.0);
        ASSERT_EQ(r[2], -1.0);

        r[0] = 1.0; r[1] = 0.0; r[2] = 0.0;
        ASSERT_NEAR(vnorm(r), 1.0, 0.000001);
        r[0] = 0.0; r[1] = 1.0; r[2] = 0.0;
        ASSERT_NEAR(vnorm(r), 1.0, 0.000001);
        r[0] = 0.0; r[1] = 0.0; r[2] = 1.0;
        ASSERT_NEAR(vnorm(r), 1.0, 0.000001);
        r[0] = 1.0; r[1] = 1.0; r[2] = 0.0;
        ASSERT_NEAR(vnorm(r), sqrt(2.0), 0.000001);
        r[0] = 1.0; r[1] = 1.0; r[2] = 1.0;
        ASSERT_NEAR(vnorm(r), sqrt(3.0), 0.000001);

        r[0] = 1.0; r[1] = 0.0; r[2] = 0.0;
        normalize(r, r);
        ASSERT_NEAR(r[0], 1.0, 0.000001);
        ASSERT_EQ(r[1], 0.0);
        ASSERT_EQ(r[2], 0.0);

        r[0] = 1.0; r[1] = 1.0; r[2] = 0.0;
        normalize(r, r);
        ASSERT_NEAR(r[0], sqrt(2)/2.0, 0.000001);
        ASSERT_NEAR(r[1], sqrt(2)/2.0, 0.000001);
        ASSERT_EQ(r[2], 0.0);

        double d = vdist(a, b);
        ASSERT_NEAR(d, sqrt(3), 0.000001);

        ASSERT_EQ(veq(a, b), false);
        vcopy(r, a);
        ASSERT_EQ(veq(a, r), true);

        sadd(r, a, 0.1);
        ASSERT_EQ(vnear(a, r, 0.3), true);
        ASSERT_EQ(vnear(a, r, 0.1), false);

        sadd(r, a, -0.1);
        vclamp(r, r, a, b);
        ASSERT_EQ(r[0], a[0]);
        ASSERT_EQ(r[1], a[1]);
        ASSERT_EQ(r[2], a[2]);
        
        sadd(r, b, 0.1);
        vclamp(r, r, a, b);
        ASSERT_EQ(r[0], b[0]);
        ASSERT_EQ(r[1], b[1]);
        ASSERT_EQ(r[2], b[2]);
}

