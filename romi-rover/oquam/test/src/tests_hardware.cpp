#include <string>
#include <iostream>
#include <thread>
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <RSerial.h>
#include <RomiSerialClient.h>

#include "oquam/Oquam.h"
#include "oquam/StepperController.h"

#include "Linux.h"
#include <ClockAccessor.h>
#include "data_provider/RomiDeviceData.h"
#include "data_provider/SoftwareVersion.h"
#include "weeder/Session.h"
#include "data_provider/Gps.h"
#include "data_provider/GpsLocationProvider.h"

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
        
	hardware_tests() : range(xmin, xmax), linux(), romiDeviceData(), softwareVersion(), gps(), locationPrivider(),
	session_directory("session-directory")
	{
                locationPrivider  = std::make_unique<GpsLocationProvider>(gps);
	}

	~hardware_tests() override = default;

	void SetUp() override {}

	void TearDown() override {}

    rpp::Linux linux;
    RomiDeviceData romiDeviceData;
    SoftwareVersion softwareVersion;
    romi::Gps gps;
    std::unique_ptr<ILocationProvider> locationPrivider;
    const std::string session_directory;

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
        auto romi_serial = romiserial::RomiSerialClient::create(device);
        StepperController stepper(romi_serial);
        //romi_serial.set_debug(debug_romi_serial);
        
        stepper.homing();
}

TEST_F(hardware_tests, test_move)
{
        auto romi_serial = romiserial::RomiSerialClient::create(device);
        StepperController stepper(romi_serial);
        //romi_serial.set_debug(debug_romi_serial);
        stepper.move(1000, 1000, 1000, 0);
        stepper.synchronize(120.0);
}

TEST_F(hardware_tests, test_get_position)
{
        auto romi_serial = romiserial::RomiSerialClient::create(device);
        StepperController stepper(romi_serial);
        //romi_serial.set_debug(debug_romi_serial);

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
        auto romi_serial = romiserial::RomiSerialClient::create(device);
        StepperController stepper(romi_serial);
        
        //romi_serial.set_debug(debug_romi_serial);
        romi::Session session(linux, session_directory, romiDeviceData, softwareVersion, std::move(locationPrivider));
        session.start("hw_observation_id");
        Oquam oquam(stepper, range, vmax, amax, scale, 0.01, slice_interval, session);
}

TEST_F(hardware_tests, test_oquam_moveto_1)
{
        auto romi_serial = romiserial::RomiSerialClient::create(device);
        StepperController stepper(romi_serial);

        //romi_serial.set_debug(debug_romi_serial);

        romi::Session session(linux, session_directory, romiDeviceData, softwareVersion, std::move(locationPrivider));
        session.start("hw_observation_id");
        Oquam oquam(stepper, range, vmax, amax, scale, 0.01, slice_interval, session);

        bool success = oquam.moveto(0.1, 0.0, 0.0, 0.3);
        ASSERT_EQ(success, true);
}

TEST_F(hardware_tests, test_oquam_moveto_2)
{
        auto romi_serial = romiserial::RomiSerialClient::create(device);
        StepperController stepper(romi_serial);

        //romi_serial.set_debug(debug_romi_serial);

        romi::Session session(linux, session_directory, romiDeviceData, softwareVersion, std::move(locationPrivider));
        session.start("hw_observation_id");
        Oquam oquam(stepper, range, vmax, amax, scale, 0.01, slice_interval, session);

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
        auto romi_serial = romiserial::RomiSerialClient::create(device);
        StepperController stepper(romi_serial);

        //romi_serial.set_debug(debug_romi_serial);

        romi::Session session(linux, session_directory, romiDeviceData, softwareVersion, std::move(locationPrivider));
        session.start("test_travel_square");
        Oquam oquam(stepper, range, vmax, amax, scale, 0.03, slice_interval, session);

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
        auto romi_serial = romiserial::RomiSerialClient::create(device);
        StepperController stepper(romi_serial);
        
        //romi_serial.set_debug(debug_romi_serial);

        romi::Session session(linux, session_directory, romiDeviceData, softwareVersion, std::move(locationPrivider));
        session.start("test_travel_square_fast");
        Oquam oquam(stepper, range, vmax, amax, scale, 0.03, slice_interval, session);

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
        auto romi_serial = romiserial::RomiSerialClient::create(device);
        StepperController stepper(romi_serial);
        
        //romi_serial.set_debug(debug_romi_serial);

        romi::Session session(linux, session_directory, romiDeviceData, softwareVersion, std::move(locationPrivider));
        session.start("test_travel_snake");
        Oquam oquam(stepper, range, vmax, amax, scale, 0.005, slice_interval, session);

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
        auto romi_serial = romiserial::RomiSerialClient::create(device);
        StepperController stepper(romi_serial);
        
        //romi_serial.set_debug(debug_romi_serial);

        romi::Session session(linux, session_directory, romiDeviceData, softwareVersion, std::move(locationPrivider));
        session.start("test_travel_snake_2");
        Oquam oquam(stepper, range, vmax, amax, scale, 0.005, slice_interval, session);

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
        auto romi_serial = romiserial::RomiSerialClient::create(device);
        StepperController stepper(romi_serial);
        
        //romi_serial.set_debug(debug_romi_serial);

        romi::Session session(linux, session_directory, romiDeviceData, softwareVersion, std::move(locationPrivider));
        session.start("test_travel_round_trip");
        Oquam oquam(stepper, range, vmax, amax, scale, 0.005, slice_interval, session);

        Path path;
        path.push_back(v3(0.0, 0.0, 0.0));
        path.push_back(v3(0.1, 0.0, 0.0));
        path.push_back(v3(0.0, 0.0, 0.0));
        
        bool success = oquam.travel(path, 1.0);
        ASSERT_EQ(success, true);
}

TEST_F(hardware_tests, test_oquam_travel_collinear)
{
        auto romi_serial = romiserial::RomiSerialClient::create(device);
        StepperController stepper(romi_serial);
        
        //romi_serial.set_debug(debug_romi_serial);

        romi::Session session(linux, session_directory, romiDeviceData, softwareVersion, std::move(locationPrivider));
        session.start("test_travel_collinear");
        Oquam oquam(stepper, range, vmax, amax, scale, 0.005, slice_interval, session);

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
        auto romi_serial = romiserial::RomiSerialClient::create(device);
        StepperController stepper(romi_serial);
        
        //romi_serial.set_debug(debug_romi_serial);

        romi::Session session(linux, session_directory, romiDeviceData, softwareVersion, std::move(locationPrivider));
        session.start("test_travel_displacement");
        Oquam oquam(stepper, range, vmax, amax, scale, 0.005, slice_interval, session);

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
        auto romi_serial = romiserial::RomiSerialClient::create(device);
        StepperController stepper(romi_serial);
        
        //romi_serial.set_debug(debug_romi_serial);

        romi::Session session(linux, session_directory, romiDeviceData, softwareVersion, std::move(locationPrivider));
        session.start("test_small_displacement");
        Oquam oquam(stepper, range, vmax, amax, scale, 0.005, slice_interval, session);

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
        auto romi_serial = romiserial::RomiSerialClient::create(device);
        StepperController stepper(romi_serial);
        
        //romi_serial.set_debug(debug_romi_serial);

        romi::Session session(linux, session_directory, romiDeviceData, softwareVersion, std::move(locationPrivider));
        session.start("test_tiny_displacement");
        Oquam oquam(stepper, range, vmax, amax, scale, 0.005, slice_interval, session);

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
        auto romi_serial = romiserial::RomiSerialClient::create(device);
        StepperController stepper(romi_serial);
        
        //romi_serial.set_debug(debug_romi_serial);

        romi::Session session(linux, session_directory, romiDeviceData, softwareVersion, std::move(locationPrivider));
        session.start("test_travel_zigzag");
        Oquam oquam(stepper, range, vmax, amax, scale, 0.005, slice_interval, session);

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
        auto clock = rpp::ClockAccessor::GetInstance();
        for (int i = 0; i < 4; i++) {
                clock->sleep(1.0);
                oquam->pause_activity();
                clock->sleep(1.0);
                oquam->continue_activity();
        }
} 

TEST_F(hardware_tests, test_oquam_stop_and_continue)
{
        auto romi_serial = romiserial::RomiSerialClient::create(device);
        StepperController stepper(romi_serial);
        
        //romi_serial.set_debug(debug_romi_serial);

        romi::Session session(linux, session_directory, romiDeviceData, softwareVersion, std::move(locationPrivider));
        session.start("test_stop_and_continue");
        Oquam oquam(stepper, range, vmax, amax, scale, 0.005, slice_interval, session);

        
        std::thread th(stop_and_continue, &oquam);

        oquam.moveto(0.2, 0.2, 0.0, 0.2);

        th.join();
}

void stop_and_reset(Oquam *oquam) 
{
        auto clock = rpp::ClockAccessor::GetInstance();
        clock->sleep(1.0);
        oquam->pause_activity();
        clock->sleep(1.0);
        oquam->reset_activity();
} 

TEST_F(hardware_tests, test_oquam_stop_and_reset)
{
        auto romi_serial = romiserial::RomiSerialClient::create(device);
        StepperController stepper(romi_serial);
        
        //romi_serial.set_debug(debug_romi_serial);

        romi::Session session(linux, session_directory, romiDeviceData, softwareVersion, std::move(locationPrivider));
        session.start("test_stop_and_reset");
        Oquam oquam(stepper, range, vmax, amax, scale, 0.005, slice_interval, session);
        
        std::thread th(stop_and_reset, &oquam);

        oquam.moveto(0.2, 0.2, 0.0, 0.2);

        th.join();
}

