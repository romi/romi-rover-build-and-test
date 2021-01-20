#include <string>
#include <iostream>
#include <thread>
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <RSerial.h>
#include <RomiSerialClient.h>

#include "DebugWeedingSession.h"

#include "oquam/Oquam.h"
#include "oquam/StepperController.h"

using namespace std;
using namespace testing;
using namespace romi;

//const char *device = "/dev/ttyACM0";
const char *device = "/dev/ttyUSB0";

class hardware_tests : public ::testing::Test
{
protected:
        
        const bool debug_romi_serial = false;
        
        const double xmin[3] =  {0, 0, 0};
        const double xmax[3] =  {0.5, 0.5, 0};
        const double vmax[3] =  {0.1, 0.1, 0.01};
        const double amax[3] =  {0.2, 0.2, 0.2};
        const double scale[3] = {40000, 40000, 100000};
        const double slice_interval = 0.020;
        CNCRange range;
        
	hardware_tests() : range(xmin, xmax) {}

	~hardware_tests() override = default;

	void SetUp() override {}

	void TearDown() override {}
};

// TEST_F(hardware_tests, test_homing_romiserial)
// {
//         RSerial serial(device, 115200, 1);        
//         RomiSerialClient romi_serial(&serial, &serial);
//         romi_serial.set_debug(debug_romi_serial);

//         JsonCpp reply;
//         romi_serial.send("H", reply);
        
//         //json_print(reply, k_json_pretty);

//         ASSERT_EQ(true, reply.isarray());
//         ASSERT_EQ(1, reply.length());
//         ASSERT_EQ(0.0, reply.num(0));

//         while (true) {
//                 romi_serial.send("I", reply);
//                 //json_print(reply, k_json_pretty);
//                 if (1 == (int) reply.num(1)) {
//                         break;
//                 }
//                 clock_sleep(1.0);
//         }
// }

TEST_F(hardware_tests, test_homing_stepper)
{
        RSerial serial(device, 115200, 1);        
        RomiSerialClient romi_serial(&serial, &serial);
        StepperController stepper(romi_serial);
        romi_serial.set_debug(debug_romi_serial);
        
        stepper.homing();
}

TEST_F(hardware_tests, test_move)
{
        RSerial serial(device, 115200, 1);        
        RomiSerialClient romi_serial(&serial, &serial);
        StepperController stepper(romi_serial);
        romi_serial.set_debug(debug_romi_serial);
        stepper.move(1000, 1000, 1000, 0);
        stepper.synchronize(120.0);
}

TEST_F(hardware_tests, test_get_position)
{
        RSerial serial(device, 115200, 1);        
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

TEST_F(hardware_tests, test_oquam_homing)
{
        RSerial serial(device, 115200, 1);        
        RomiSerialClient romi_serial(&serial, &serial);
        StepperController stepper(romi_serial);
        
        romi_serial.set_debug(debug_romi_serial);
        
        Oquam oquam(stepper, range, vmax, amax, scale, 0.01, slice_interval);
}

TEST_F(hardware_tests, test_oquam_moveto_1)
{
        RSerial serial(device, 115200, 1);        
        RomiSerialClient romi_serial(&serial, &serial);
        StepperController stepper(romi_serial);

        romi_serial.set_debug(debug_romi_serial);
        
        Oquam oquam(stepper, range, vmax, amax, scale, 0.01, slice_interval);

        bool success = oquam.moveto(0.1, 0.0, 0.0, 0.3);
        ASSERT_EQ(success, true);
}

TEST_F(hardware_tests, test_oquam_moveto_2)
{
        RSerial serial(device, 115200, 1);        
        RomiSerialClient romi_serial(&serial, &serial);
        StepperController stepper(romi_serial);

        romi_serial.set_debug(debug_romi_serial);
        
        Oquam oquam(stepper, range, vmax, amax, scale, 0.01, slice_interval);

        bool success = oquam.moveto(0.1, 0.0, 0.0, 0.3);
        ASSERT_EQ(success, true);
        success = oquam.moveto(0.1, 0.1, 0.0, 0.3);
        ASSERT_EQ(success, true);
        success = oquam.moveto(0.0, 0.1, 0.0, 0.3);
        ASSERT_EQ(success, true);
        success = oquam.moveto(0.0, 0.0, 0.0, 0.3);
        ASSERT_EQ(success, true);
}

TEST_F(hardware_tests, test_oquam_travel_square)
{
        RSerial serial(device, 115200, 1);        
        RomiSerialClient romi_serial(&serial, &serial);
        StepperController stepper(romi_serial);
        DebugWeedingSession debug(".", "test_travel_square");

        romi_serial.set_debug(debug_romi_serial);
        
        Oquam oquam(stepper, range, vmax, amax, scale, 0.03, slice_interval);
        oquam.set_file_cabinet(&debug);

        Path path;
        v3 p0(0.1, 0.0, 0.0);
        v3 p1(0.1, 0.1, 0.0);
        v3 p2(0.0, 0.1, 0.0);
        v3 p3(0.0, 0.0, 0.0);
        path.push_back(p0);
        path.push_back(p1);
        path.push_back(p2);
        path.push_back(p3);

        bool success = oquam.travel(path, 0.3);
        ASSERT_EQ(success, true);
}

TEST_F(hardware_tests, test_oquam_travel_square_fast)
{
        RSerial serial(device, 115200, 1);        
        RomiSerialClient romi_serial(&serial, &serial);
        StepperController stepper(romi_serial);
        DebugWeedingSession debug(".", "test_travel_square_fast");
        
        romi_serial.set_debug(debug_romi_serial);
        
        Oquam oquam(stepper, range, vmax, amax, scale, 0.03, slice_interval);
        oquam.set_file_cabinet(&debug);

        Path path;
        v3 p0(0.1, 0.0, 0.0);
        v3 p1(0.1, 0.1, 0.0);
        v3 p2(0.0, 0.1, 0.0);
        v3 p3(0.0, 0.0, 0.0);
        path.push_back(p0);
        path.push_back(p1);
        path.push_back(p2);
        path.push_back(p3);

        bool success = oquam.travel(path, 1.0);
        ASSERT_EQ(success, true);
}

TEST_F(hardware_tests, test_oquam_travel_snake)
{
        RSerial serial(device, 115200, 1);        
        RomiSerialClient romi_serial(&serial, &serial);
        StepperController stepper(romi_serial);
        DebugWeedingSession debug(".", "test_travel_snake");
        
        romi_serial.set_debug(debug_romi_serial);
        
        Oquam oquam(stepper, range, vmax, amax, scale, 0.005, slice_interval);
        oquam.set_file_cabinet(&debug);

        Path path;
        int N = 10;
        for (int i = 1; i <= N; i++) {
                v3 p0(i * 0.01, (i-1) * 0.01, 0.0);
                path.push_back(p0);
                v3 p1(i * 0.01, i * 0.01, 0.0);
                path.push_back(p1);
        }
        
        v3 p(0.0, 0.0, 0.0);
        path.push_back(p);

        bool success = oquam.travel(path, 1.0);
        ASSERT_EQ(success, true);
}

TEST_F(hardware_tests, test_oquam_travel_snake_2)
{
        RSerial serial(device, 115200, 1);        
        RomiSerialClient romi_serial(&serial, &serial);
        StepperController stepper(romi_serial);
        DebugWeedingSession debug(".", "test_travel_snake_2");
        
        romi_serial.set_debug(debug_romi_serial);
        
        Oquam oquam(stepper, range, vmax, amax, scale, 0.005, slice_interval);
        oquam.set_file_cabinet(&debug);

        int N = 11;
        Path path;
        double x = 0.0;
        double y = 0.0;
        for (int i = 1; i <= N; i++) {
                int n = N + 1 - i;
                double len = 0.001 * n;
                v3 p0(x + len, y, 0.0);
                path.push_back(p0);
                v3 p1(x + len, y + len, 0.0);
                path.push_back(p1);
                x += len;
                y += len;
        }
        
        v3 p(0.0, 0.0, 0.0);
        path.push_back(p);

        bool success = oquam.travel(path, 1.0);
        ASSERT_EQ(success, true);
}

TEST_F(hardware_tests, test_oquam_travel_round_trip)
{
        RSerial serial(device, 115200, 1);        
        RomiSerialClient romi_serial(&serial, &serial);
        StepperController stepper(romi_serial);
        DebugWeedingSession debug(".", "test_travel_round_trip");
        
        romi_serial.set_debug(debug_romi_serial);
        
        Oquam oquam(stepper, range, vmax, amax, scale, 0.005, slice_interval);
        oquam.set_file_cabinet(&debug);

        Path path;
        path.push_back(v3(0.0, 0.0, 0.0));
        path.push_back(v3(0.1, 0.0, 0.0));
        path.push_back(v3(0.0, 0.0, 0.0));
        
        bool success = oquam.travel(path, 1.0);
        ASSERT_EQ(success, true);
}

TEST_F(hardware_tests, test_oquam_travel_collinear)
{
        RSerial serial(device, 115200, 1);        
        RomiSerialClient romi_serial(&serial, &serial);
        StepperController stepper(romi_serial);
        DebugWeedingSession debug(".", "test_travel_collinear");
        
        romi_serial.set_debug(debug_romi_serial);
        
        Oquam oquam(stepper, range, vmax, amax, scale, 0.005, slice_interval);
        oquam.set_file_cabinet(&debug);

        Path path;
        path.push_back(v3(0.0, 0.0, 0.0));
        path.push_back(v3(0.1, 0.0, 0.0));
        path.push_back(v3(0.2, 0.0, 0.0));
        path.push_back(v3(0.0, 0.0, 0.0));
        
        bool success = oquam.travel(path, 1.0);
        ASSERT_EQ(success, true);
}

TEST_F(hardware_tests, test_oquam_travel_large_displacement)
{
        RSerial serial(device, 115200, 1);        
        RomiSerialClient romi_serial(&serial, &serial);
        StepperController stepper(romi_serial);
        DebugWeedingSession debug(".", "test_travel_displacement");
        
        romi_serial.set_debug(debug_romi_serial);
        
        Oquam oquam(stepper, range, vmax, amax, scale, 0.005, slice_interval);
        oquam.set_file_cabinet(&debug);

        Path path;
        path.push_back(v3(0.0, 0.0, 0.0));
        path.push_back(v3(0.1, 0.0, 0.0));
        path.push_back(v3(0.1, 0.07, 0.0));
        path.push_back(v3(0.2, 0.07, 0.0));
        path.push_back(v3(0.0, 0.0, 0.0));
        
        bool success = oquam.travel(path, 1.0);
        ASSERT_EQ(success, true);
}

TEST_F(hardware_tests, test_oquam_travel_small_displacement)
{
        RSerial serial(device, 115200, 1);        
        RomiSerialClient romi_serial(&serial, &serial);
        StepperController stepper(romi_serial);
        DebugWeedingSession debug(".", "test_small_displacement");
        
        romi_serial.set_debug(debug_romi_serial);
        
        Oquam oquam(stepper, range, vmax, amax, scale, 0.005, slice_interval);
        oquam.set_file_cabinet(&debug);

        Path path;
        path.push_back(v3(0.0, 0.0, 0.0));
        path.push_back(v3(0.1, 0.0, 0.0));
        path.push_back(v3(0.1, 0.04, 0.0));
        path.push_back(v3(0.2, 0.04, 0.0));
        path.push_back(v3(0.0, 0.0, 0.0));
        
        bool success = oquam.travel(path, 1.0);
        ASSERT_EQ(success, true);
}

TEST_F(hardware_tests, test_oquam_travel_tiny_displacement)
{
        RSerial serial(device, 115200, 1);        
        RomiSerialClient romi_serial(&serial, &serial);
        StepperController stepper(romi_serial);
        DebugWeedingSession debug(".", "test_tiny_displacement");
        
        romi_serial.set_debug(debug_romi_serial);
        
        Oquam oquam(stepper, range, vmax, amax, scale, 0.005, slice_interval);
        oquam.set_file_cabinet(&debug);

        Path path;
        path.push_back(v3(0.0, 0.0, 0.0));
        path.push_back(v3(0.1, 0.0, 0.0));
        path.push_back(v3(0.1, 0.005, 0.0));
        path.push_back(v3(0.2, 0.005, 0.0));
        path.push_back(v3(0.0, 0.0, 0.0));
        
        bool success = oquam.travel(path, 1.0);
        ASSERT_EQ(success, true);
}

TEST_F(hardware_tests, test_oquam_travel_zigzag)
{
        RSerial serial(device, 115200, 1);        
        RomiSerialClient romi_serial(&serial, &serial);
        StepperController stepper(romi_serial);
        DebugWeedingSession debug(".", "test_travel_zigzag");
        
        romi_serial.set_debug(debug_romi_serial);
        
        Oquam oquam(stepper, range, vmax, amax, scale, 0.005, slice_interval);
        oquam.set_file_cabinet(&debug);

        Path path;
        v3 p(0.0, 0.0, 0.0);
        
        for (int i = 1; i <= 3; i++) {
                p.y() += 0.01;
                path.push_back(p);
                
                p.x() += 0.1;
                path.push_back(p);
                
                p.y() += 0.01;
                path.push_back(p);

                p.x() -= 0.1;
                path.push_back(p);
        }
        
        path.push_back(v3(0.0, 0.0, 0.0));

        bool success = oquam.travel(path, 1.0);
        ASSERT_EQ(success, true);
}


void stop_and_continue(Oquam *oquam) 
{
        for (int i = 0; i < 4; i++) {
                clock_sleep(1.0);
                oquam->pause_activity();
                clock_sleep(1.0);
                oquam->continue_activity();
        }
} 

TEST_F(hardware_tests, test_oquam_stop_and_continue)
{
        RSerial serial(device, 115200, 1);        
        RomiSerialClient romi_serial(&serial, &serial);
        StepperController stepper(romi_serial);
        DebugWeedingSession debug(".", "test_stop_and_continue");
        
        romi_serial.set_debug(debug_romi_serial);
        
        Oquam oquam(stepper, range, vmax, amax, scale, 0.005, slice_interval);
        oquam.set_file_cabinet(&debug);

        
        std::thread th(stop_and_continue, &oquam);

        oquam.moveto(0.2, 0.2, 0.0, 0.2);

        th.join();
}

void stop_and_reset(Oquam *oquam) 
{
        clock_sleep(1.0);
        oquam->pause_activity();
        clock_sleep(1.0);
        oquam->reset_activity();
} 

TEST_F(hardware_tests, test_oquam_stop_and_reset)
{
        RSerial serial(device, 115200, 1);        
        RomiSerialClient romi_serial(&serial, &serial);
        StepperController stepper(romi_serial);
        DebugWeedingSession debug(".", "test_stop_and_reset");
        
        romi_serial.set_debug(debug_romi_serial);
        
        Oquam oquam(stepper, range, vmax, amax, scale, 0.005, slice_interval);
        oquam.set_file_cabinet(&debug);
        
        std::thread th(stop_and_reset, &oquam);

        oquam.moveto(0.2, 0.2, 0.0, 0.2);

        th.join();
}

