#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "Planner.h"

using namespace std;
using namespace testing;

class planner_tests : public ::testing::Test
{
protected:
	planner_tests() {
	}

	~planner_tests() override = default;

	void SetUp() override {
	}

	void TearDown() override {
	}
};

TEST_F(planner_tests, test_single_moveto_actions)
{
        // Arrange
        oquam::Script script;
        script.moveto(1.0, 0.0, 0.0, 1.0, 0);
        double pos[3] = {0, 0, 0};
        
        // Act
        std::vector<oquam::ConsecutiveSections> paths = split(&script, pos);
        
        //Assert
        ASSERT_EQ(paths.size(), 1);
        oquam::ConsecutiveSections path = paths[0];
        ASSERT_EQ(path.size(), 1);
        
        oquam::Section section1 = path[0];
        ASSERT_EQ(section1.id, 0);
        ASSERT_EQ(section1.p0[0], 0.0);
        ASSERT_EQ(section1.p1[0], 1.0);
        ASSERT_EQ(section1.d[0], 1.0);
        ASSERT_EQ(section1.v0[0], 1.0);
        ASSERT_EQ(section1.v1[0], 1.0);        
}

TEST_F(planner_tests, test_two_moveto_actions)
{
        // Arrange
        oquam::Script script;
        script.moveto(1.0, 0.0, 0.0, 1.0, 0);
        script.moveto(2.0, 0.0, 0.0, 1.0, 1);
        double pos[3] = {0, 0, 0};
        
        // Act
        std::vector<oquam::ConsecutiveSections> paths = split(&script, pos);
        
        //Assert
        ASSERT_EQ(paths.size(), 1);
        oquam::ConsecutiveSections path = paths[0];
        ASSERT_EQ(path.size(), 2);
        
        oquam::Section section1 = path[0];
        ASSERT_EQ(section1.id, 0);
        ASSERT_EQ(section1.p0[0], 0.0);
        ASSERT_EQ(section1.p1[0], 1.0);
        ASSERT_EQ(section1.d[0], 1.0);
        ASSERT_EQ(section1.v0[0], 1.0);
        ASSERT_EQ(section1.v1[0], 1.0);
        
        oquam::Section section2 = path[1];
        ASSERT_EQ(section2.id, 1);
        ASSERT_EQ(section2.p0[0], 1.0);
        ASSERT_EQ(section2.p1[0], 2.0);
        ASSERT_EQ(section2.d[0], 1.0);
        ASSERT_EQ(section2.v0[0], 1.0);
        ASSERT_EQ(section2.v1[0], 1.0);
        
}

TEST_F(planner_tests, test_two_moveto_and_one_delay_actions)
{
        // Arrange
        oquam::Script script;
        script.moveto(1.0, 0.0, 0.0, 1.0, 0);
        script.delay(1.0, 1);
        script.moveto(2.0, 0.0, 0.0, 1.0, 2);
        double pos[3] = {0, 0, 0};
        
        // Act
        std::vector<oquam::ConsecutiveSections> paths = split(&script, pos);
        
        //Assert
        ASSERT_EQ(paths.size(), 2);
        oquam::ConsecutiveSections path;
        oquam::Section section;
        
        path = paths[0];
        ASSERT_EQ(path.size(), 2);
        
        section = path[0];
        ASSERT_EQ(section.id, 0);
        ASSERT_EQ(section.action.id, 0);
        ASSERT_EQ(section.p0[0], 0.0);
        ASSERT_EQ(section.p1[0], 1.0);
        ASSERT_EQ(section.d[0], 1.0);
        ASSERT_EQ(section.v0[0], 1.0);
        ASSERT_EQ(section.v1[0], 1.0);

        //section = path[1];
        
        path = paths[1];
        ASSERT_EQ(path.size(), 1);
        
        section = path[0];
        ASSERT_EQ(section.id, 2);
        ASSERT_EQ(section.p0[0], 1.0);
        ASSERT_EQ(section.p1[0], 2.0);
        ASSERT_EQ(section.d[0], 1.0);
        ASSERT_EQ(section.v0[0], 1.0);
        ASSERT_EQ(section.v1[0], 1.0);
        
}
