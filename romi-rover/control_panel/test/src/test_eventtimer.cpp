#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "EventTimer.h"

using namespace std;
using namespace testing;

class eventtimer_tests : public ::testing::Test
{
protected:
	eventtimer_tests() {
	}

	~eventtimer_tests() override = default;

	void SetUp() override {
	}

	void TearDown() override {
	}
};

TEST_F(eventtimer_tests, eventtimer_test_single_event)
{
        // Arrange
        EventTimer eventtimer;
        eventtimer.setTimeout(100, 1);

        // Act
        vector<int16_t> r;
        r.push_back(eventtimer.update(0));
        r.push_back(eventtimer.update(99));
        r.push_back(eventtimer.update(100));
        r.push_back(eventtimer.update(101));
        
        //Assert
        ASSERT_THAT(r, ElementsAre(EventTimerNoEvent, EventTimerNoEvent,
                                   1, EventTimerNoEvent));
}

TEST_F(eventtimer_tests, eventtimer_test_two_events)
{
        // Arrange
        EventTimer eventtimer;
        eventtimer.setTimeout(100, 1);

        // Act
        vector<int16_t> r;
        r.push_back(eventtimer.update(0));
        r.push_back(eventtimer.update(99));
        r.push_back(eventtimer.update(100));
        r.push_back(eventtimer.update(101));
        eventtimer.setTimeout(200, 1);
        r.push_back(eventtimer.update(102));
        r.push_back(eventtimer.update(200));
        r.push_back(eventtimer.update(201));
        
        //Assert
        ASSERT_THAT(r, ElementsAre(EventTimerNoEvent, EventTimerNoEvent, 1,
                                   EventTimerNoEvent, EventTimerNoEvent, 1,
                                   EventTimerNoEvent));
}
