#include <string>
#include <iostream>
#include <thread>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "StepperController.h"
#include "RSerial.h"
#include "RomiSerialClient.h"

using namespace std;
using namespace testing;
using namespace romi;

class oquamstepper_tests : public ::testing::Test
{
protected:
        
        const bool debug_romi_serial = true;
        
        const double xmin[3] =  {0, 0, 0};
        const double xmax[3] =  {0.5, 0.5, 0};
        const double vmax[3] =  {0.1, 0.1, 0.01};
        const double amax[3] =  {0.2, 0.2, 0.2};
        const double scale[3] = {40000, 40000, 100000};
        const double slice_interval = 0.020;
        CNCRange range;
        
	oquamstepper_tests() : range(xmin, xmax) {}

	~oquamstepper_tests() override = default;

	void SetUp() override {}

	void TearDown() override {}
};

TEST_F(oquamstepper_tests, test_homing_romiserial)
{
        RSerial serial("/dev/ttyACM0", 115200, 1);        
        RomiSerialClient romi_serial(&serial, &serial);
        romi_serial.set_debug(debug_romi_serial);

        JsonCpp reply;
        romi_serial.send("H", reply);
        
        //json_print(reply, k_json_pretty);

        ASSERT_EQ(true, reply.isarray());
        ASSERT_EQ(1, reply.length());
        ASSERT_EQ(0.0, reply.num(0));

        while (true) {
                romi_serial.send("I", reply);
                //json_print(reply, k_json_pretty);
                if (1 == (int) reply.num(1)) {
                        break;
                }
                clock_sleep(1.0);
        }
}

TEST_F(oquamstepper_tests, test_homing_stepper)
{
        RSerial serial("/dev/ttyACM0", 115200, 1);        
        RomiSerialClient romi_serial(&serial, &serial);
        StepperController stepper(romi_serial);
        romi_serial.set_debug(debug_romi_serial);
        
        stepper.homing();
}

TEST_F(oquamstepper_tests, test_move)
{
        RSerial serial("/dev/ttyACM0", 115200, 1);        
        RomiSerialClient romi_serial(&serial, &serial);
        StepperController stepper(romi_serial);
        romi_serial.set_debug(debug_romi_serial);
        stepper.move(1000, 1000, 1000, 0);
        stepper.synchronize(120.0);
}

TEST_F(oquamstepper_tests, test_get_position)
{
        RSerial serial("/dev/ttyACM0", 115200, 1);        
        RomiSerialClient romi_serial(&serial, &serial);
        StepperController stepper(romi_serial);
        romi_serial.set_debug(debug_romi_serial);

        stepper.homing();
        
        int32_t position0[3];
        stepper.get_position(position0);
        
        stepper.move(1000, 1000, 1000, 0);
        stepper.synchronize(10.0);

        int32_t position1[3];
        stepper.get_position(position1);

        stepper.move(1000, 2000, 2000, 0);
        stepper.synchronize(10.0);

        int32_t position2[3];
        stepper.get_position(position2);

        for (int i = 0; i < 2; i++) {
                ASSERT_EQ(0, position0[i]);
                ASSERT_EQ(1000, position1[i]);
                ASSERT_EQ(3000, position2[i]);
        }
        ASSERT_EQ(0, position0[2]);
        ASSERT_EQ(0, position1[2]);
        ASSERT_EQ(0, position2[2]);
}
