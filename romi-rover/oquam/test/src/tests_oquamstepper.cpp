#include <string>
#include <iostream>
#include <thread>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "StepperController.h"
#include "RSerial.h"
#include "RomiSerialClient.h"
#include "Oquam.h"
#include "DebugWeedingSession.h"

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
        
	oquamstepper_tests() {}

	~oquamstepper_tests() override = default;

	void SetUp() override {}

	void TearDown() override {}
};

// TEST_F(oquamstepper_tests, test_homing_serial)
// {
//         RSerial serial("/dev/ttyACM0", 115200, 1);
        
//         serial.println("#H\r");

//         char reply[256];
//         while (true) {
//                 serial.readline(reply, sizeof(reply));
//                 //std::cout << reply << std::endl;
//                 if (reply[0] == '#')
//                         break;
//         }
        
//         ASSERT_STREQ(reply, "#H[0]:0073\r\n");        

//         while (true) {
//                 memset(reply, 0, sizeof(reply));
//                 serial.println("#I\r");
//                 serial.readline(reply, sizeof(reply));
//                 //std::cout << reply << std::endl;
//                 if (reply[5] == '1')
//                         break;
//         }
// }

// TEST_F(oquamstepper_tests, test_homing_romiserial)
// {
//         RSerial serial("/dev/ttyACM0", 115200, 1);        
//         RomiSerialClient romi_serial(&serial, &serial);

//         json_object_t reply = romi_serial.send("H");
        
//         //json_print(reply, k_json_pretty);

//         ASSERT_EQ(true, json_isarray(reply));
//         ASSERT_EQ(1, json_array_length(reply));
//         ASSERT_EQ(0.0, json_array_getnum(reply, 0));
//         json_unref(reply);

//         while (true) {
//                 reply = romi_serial.send("S");
//                 //json_print(reply, k_json_pretty);
//                 if (rstreq("r", json_array_getstr(reply, 1))) {
//                         json_unref(reply);
//                         break;
//                 }
//                 clock_sleep(1.0);
//                 json_unref(reply);
//         }
// }

// TEST_F(oquamstepper_tests, test_homing_stepper)
// {
//         RSerial serial("/dev/ttyACM0", 115200, 1);        
//         RomiSerialClient romi_serial(&serial, &serial);
//         StepperController stepper(romi_serial);
        
//         stepper.homing();
// }

// TEST_F(oquamstepper_tests, test_move)
// {
//         RSerial serial("/dev/ttyACM0", 115200, 1);        
//         RomiSerialClient romi_serial(&serial, &serial);
//         StepperController stepper(romi_serial);
//         stepper.move(1000, 1000, 1000, 0);
//         stepper.synchronize(120.0);
// }

// TEST_F(oquamstepper_tests, test_get_position)
// {
//         RSerial serial("/dev/ttyACM0", 115200, 1);        
//         RomiSerialClient romi_serial(&serial, &serial);
//         StepperController stepper(romi_serial);

//         stepper.homing();
        
//         int32_t position0[3];
//         stepper.get_position(position0);
        
//         stepper.move(1000, 1000, 1000, 0);
//         stepper.synchronize(10.0);

//         int32_t position1[3];
//         stepper.get_position(position1);

//         stepper.move(1000, 2000, 2000, 0);
//         stepper.synchronize(10.0);

//         int32_t position2[3];
//         stepper.get_position(position2);

//         for (int i = 0; i < 2; i++) {
//                 ASSERT_EQ(0, position0[i]);
//                 ASSERT_EQ(1000, position1[i]);
//                 ASSERT_EQ(3000, position2[i]);
//         }
//         ASSERT_EQ(0, position0[2]);
//         ASSERT_EQ(0, position1[2]);
//         ASSERT_EQ(0, position2[2]);
// }

// TEST_F(oquamstepper_tests, test_oquam_homing)
// {
//         RSerial serial("/dev/ttyACM0", 115200, 1);        
//         RomiSerialClient romi_serial(&serial, &serial);
//         StepperController stepper(romi_serial);
        
//         romi_serial.set_debug(debug_romi_serial);
        
//         Oquam oquam(stepper, xmin, xmax, vmax, amax, scale, 0.01, 0.014);

//         oquam.homing();
// }

// TEST_F(oquamstepper_tests, test_oquam_moveto)
// {
//         RSerial serial("/dev/ttyACM0", 115200, 1);        
//         RomiSerialClient romi_serial(&serial, &serial);
//         StepperController stepper(romi_serial);

//         romi_serial.set_debug(debug_romi_serial);
        
//         Oquam oquam(stepper, xmin, xmax, vmax, amax, scale, 0.01, 0.014);

//         oquam.homing();
//         oquam.moveto(0.1, 0.0, 0.0, 0.3);
//         oquam.moveto(0.1, 0.1, 0.0, 0.3);
//         oquam.moveto(0.0, 0.1, 0.0, 0.3);
//         oquam.moveto(0.0, 0.0, 0.0, 0.3);
// }

// TEST_F(oquamstepper_tests, test_oquam_travel_square)
// {
//         RSerial serial("/dev/ttyACM0", 115200, 1);        
//         RomiSerialClient romi_serial(&serial, &serial);
//         StepperController stepper(romi_serial);
//         DebugWeedingSession debug(".", "test_travel_square");

//         romi_serial.set_debug(debug_romi_serial);
        
//         Oquam oquam(stepper, xmin, xmax, vmax, amax, scale, 0.03, 0.014);
//         oquam.set_file_cabinet(&debug);

//         Path path;
//         Waypoint p0(0.1, 0.0);
//         Waypoint p1(0.1, 0.1);
//         Waypoint p2(0.0, 0.1);
//         Waypoint p3(0.0, 0.0);
//         path.push_back(p0);
//         path.push_back(p1);
//         path.push_back(p2);
//         path.push_back(p3);

//         oquam.homing();
//         oquam.travel(path, 0.3);
// }

// TEST_F(oquamstepper_tests, test_oquam_travel_square_fast)
// {
//         RSerial serial("/dev/ttyACM0", 115200, 1);        
//         RomiSerialClient romi_serial(&serial, &serial);
//         StepperController stepper(romi_serial);
//         DebugWeedingSession debug(".", "test_travel_square_fast");
        
//         romi_serial.set_debug(debug_romi_serial);
        
//         Oquam oquam(stepper, xmin, xmax, vmax, amax, scale, 0.03, 0.014);
//         oquam.set_file_cabinet(&debug);

//         Path path;
//         Waypoint p0(0.1, 0.0);
//         Waypoint p1(0.1, 0.1);
//         Waypoint p2(0.0, 0.1);
//         Waypoint p3(0.0, 0.0);
//         path.push_back(p0);
//         path.push_back(p1);
//         path.push_back(p2);
//         path.push_back(p3);

//         oquam.homing();
//         oquam.travel(path, 1.0);
// }

// TEST_F(oquamstepper_tests, test_oquam_travel_zigzag)
// {
//         RSerial serial("/dev/ttyACM0", 115200, 1);        
//         RomiSerialClient romi_serial(&serial, &serial);
//         StepperController stepper(romi_serial);
//         DebugWeedingSession debug(".", "test_travel_zigzag");
        
//         romi_serial.set_debug(debug_romi_serial);
        
//         Oquam oquam(stepper, xmin, xmax, vmax, amax, scale, 0.005, 0.014);
//         oquam.set_file_cabinet(&debug);

//         Path path;
//         for (int i = 1; i <= 10; i++) {
//                 Waypoint p0(i * 0.01, (i-1) * 0.01);
//                 path.push_back(p0);
//                 Waypoint p1(i * 0.01, i * 0.01);
//                 path.push_back(p1);
//         }
        
//         Waypoint p(0.0, 0.0);
//         path.push_back(p);

//         oquam.homing();
//         oquam.travel(path, 1.0);
// }

// TEST_F(oquamstepper_tests, test_oquam_travel_round_trip)
// {
//         RSerial serial("/dev/ttyACM0", 115200, 1);        
//         RomiSerialClient romi_serial(&serial, &serial);
//         StepperController stepper(romi_serial);
//         DebugWeedingSession debug(".", "test_travel_round_trip");
        
//         romi_serial.set_debug(debug_romi_serial);
        
//         Oquam oquam(stepper, xmin, xmax, vmax, amax, scale, 0.005, 0.014);
//         oquam.set_file_cabinet(&debug);

//         Path path;
//         path.push_back(Waypoint(0.0, 0.0));
//         path.push_back(Waypoint(0.1, 0.0));
//         path.push_back(Waypoint(0.0, 0.0));
        
//         oquam.homing();
//         oquam.travel(path, 1.0);
// }

// TEST_F(oquamstepper_tests, test_oquam_travel_collinear)
// {
//         RSerial serial("/dev/ttyACM0", 115200, 1);        
//         RomiSerialClient romi_serial(&serial, &serial);
//         StepperController stepper(romi_serial);
//         DebugWeedingSession debug(".", "test_travel_collinear");
        
//         romi_serial.set_debug(debug_romi_serial);
        
//         Oquam oquam(stepper, xmin, xmax, vmax, amax, scale, 0.005, 0.014);
//         oquam.set_file_cabinet(&debug);

//         Path path;
//         path.push_back(Waypoint(0.0, 0.0));
//         path.push_back(Waypoint(0.1, 0.0));
//         path.push_back(Waypoint(0.2, 0.0));
//         path.push_back(Waypoint(0.0, 0.0));
        
//         oquam.homing();
//         oquam.travel(path, 1.0);
// }

// TEST_F(oquamstepper_tests, test_oquam_travel_large_displacement)
// {
//         RSerial serial("/dev/ttyACM0", 115200, 1);        
//         RomiSerialClient romi_serial(&serial, &serial);
//         StepperController stepper(romi_serial);
//         DebugWeedingSession debug(".", "test_travel_displacement");
        
//         romi_serial.set_debug(debug_romi_serial);
        
//         Oquam oquam(stepper, xmin, xmax, vmax, amax, scale, 0.005, 0.014);
//         oquam.set_file_cabinet(&debug);

//         Path path;
//         path.push_back(Waypoint(0.0, 0.0));
//         path.push_back(Waypoint(0.1, 0.0));
//         path.push_back(Waypoint(0.1, 0.07));
//         path.push_back(Waypoint(0.2, 0.07));
//         path.push_back(Waypoint(0.0, 0.0));
        
//         oquam.homing();
//         oquam.travel(path, 1.0);
// }

TEST_F(oquamstepper_tests, test_oquam_travel_small_displacement)
{
        RSerial serial("/dev/ttyACM0", 115200, 1);        
        RomiSerialClient romi_serial(&serial, &serial);
        StepperController stepper(romi_serial);
        DebugWeedingSession debug(".", "test_small_displacement");
        
        romi_serial.set_debug(debug_romi_serial);
        //romi_serial.set_debug(false);
        
        Oquam oquam(stepper, xmin, xmax, vmax, amax, scale, 0.005, 0.014);
        oquam.set_file_cabinet(&debug);

        Path path;
        path.push_back(Waypoint(0.0, 0.0));
        path.push_back(Waypoint(0.1, 0.0));
        path.push_back(Waypoint(0.1, 0.04));
        path.push_back(Waypoint(0.2, 0.04));
        path.push_back(Waypoint(0.0, 0.0));
        
        oquam.homing();
        oquam.travel(path, 1.0);

        int32_t position[3];
        stepper.get_position(position);
        printf("Position: %d %d", position[0], position[1]);
}

// TEST_F(oquamstepper_tests, test_oquam_travel_tiny_displacement)
// {
//         RSerial serial("/dev/ttyACM0", 115200, 1);        
//         RomiSerialClient romi_serial(&serial, &serial);
//         StepperController stepper(romi_serial);
//         DebugWeedingSession debug(".", "test_tiny_displacement");
        
//         romi_serial.set_debug(debug_romi_serial);
        
//         Oquam oquam(stepper, xmin, xmax, vmax, amax, scale, 0.005, 0.014);
//         oquam.set_file_cabinet(&debug);

//         Path path;
//         path.push_back(Waypoint(0.0, 0.0));
//         path.push_back(Waypoint(0.1, 0.0));
//         path.push_back(Waypoint(0.1, 0.005));
//         path.push_back(Waypoint(0.2, 0.005));
//         path.push_back(Waypoint(0.0, 0.0));
        
//         oquam.homing();
//         oquam.travel(path, 1.0);
// }

// TEST_F(oquamstepper_tests, test_oquam_travel_snake)
// {
//         RSerial serial("/dev/ttyACM0", 115200, 1);        
//         RomiSerialClient romi_serial(&serial, &serial);
//         StepperController stepper(romi_serial);
//         DebugWeedingSession debug(".", "test_travel_snake");
        
//         romi_serial.set_debug(debug_romi_serial);
        
//         Oquam oquam(stepper, xmin, xmax, vmax, amax, scale, 0.005, 0.014);
//         oquam.set_file_cabinet(&debug);

//         Path path;
//         Waypoint p(0.0, 0.0);
        
//         for (int i = 1; i <= 3; i++) {
//                 p.y += 0.01;
//                 path.push_back(p);
                
//                 p.x += 0.1;
//                 path.push_back(p);
                
//                 p.y += 0.01;
//                 path.push_back(p);

//                 p.x -= 0.1;
//                 path.push_back(p);
//         }
        
//         path.push_back(Waypoint(0.0, 0.0));

//         oquam.homing();
//         oquam.travel(path, 1.0);
// }


// void stop_and_continue(Oquam *oquam) 
// {
//         for (int i = 0; i < 4; i++) {
//                 clock_sleep(1.0);
//                 oquam->stop_execution();
//                 clock_sleep(1.0);
//                 oquam->continue_execution();
//         }
// } 

// TEST_F(oquamstepper_tests, test_oquam_stop_and_continue)
// {
//         RSerial serial("/dev/ttyACM0", 115200, 1);        
//         RomiSerialClient romi_serial(&serial, &serial);
//         StepperController stepper(romi_serial);
//         DebugWeedingSession debug(".", "test_travel_snake");
        
//         romi_serial.set_debug(debug_romi_serial);
        
//         Oquam oquam(stepper, xmin, xmax, vmax, amax, scale, 0.005, 0.014);
//         oquam.set_file_cabinet(&debug);

//         oquam.homing();
        
//         std::thread th(stop_and_continue, &oquam);

//         oquam.moveto(0.2, 0.2, 0.0, 0.2);

//         th.join();
// }

// void stop_and_reset(Oquam *oquam) 
// {
//         clock_sleep(1.0);
//         oquam->stop_execution();
//         clock_sleep(1.0);
//         oquam->reset();
// } 

// TEST_F(oquamstepper_tests, test_oquam_stop_and_reset)
// {
//         RSerial serial("/dev/ttyACM0", 115200, 1);        
//         RomiSerialClient romi_serial(&serial, &serial);
//         StepperController stepper(romi_serial);
//         DebugWeedingSession debug(".", "test_travel_snake");
        
//         romi_serial.set_debug(debug_romi_serial);
        
//         Oquam oquam(stepper, xmin, xmax, vmax, amax, scale, 0.005, 0.014);
//         oquam.set_file_cabinet(&debug);

//         oquam.homing();
        
//         std::thread th(stop_and_reset, &oquam);

//         oquam.moveto(0.2, 0.2, 0.0, 0.2);

//         th.join();
// }

