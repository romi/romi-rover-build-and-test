#include <exception>
#include <stdexcept>
#include <string.h>
#include <rcom.h>

#include "ControllerServer.h"
#include "CNCClient.h"
#include "Oquam.h"
#include "OquamStepper.h"
#include "RomiSerialClient.h"
#include "RSerial.h"
#include "ConfigurationFile.h"

using namespace romi;


static inline double sign(double v)
{
        return (v < 0)? -1.0 : 1.0;
}

int main(int argc, char** argv)
{
        app_init(&argc, argv);
        app_set_name("oquam");
        
        try {
                CNCRange range;
                ConfigurationFile config("config.json");                
                range.init(config.get("cnc").get("range"));
                
                double xmin[3];
                double xmax[3];
                double vm[3];
                double steps[3] = { 200.0, 200.0, 200.0 };
                double gears[3] = { 1.0, 1.0, 1.0 };
                double rpm[3] = { 300.0, 300.0, 300.0 };
                double displacement[3] = { 0.04, 0.04, -0.002 };
                double microsteps[3] = { 8.0, 8.0, 1.0 };
                double scale[3];
                double period = 0.014;
                
                // XCarve: values taken from grbl/defaults.h.  
                // 
                // XY: 500.0*60*60 mm/min² = 500 mm/s² = 0.5 m/s²
                // Z:  50.0*60*60 mm/min² = 500 mm/s² = 0.05 m/s²
                //double amax[3] = { 0.5, 0.5, 0.05 };
                double amax[3] = { 0.3, 0.3, 0.03 };

                xmin[0] = range._x[0];
                xmax[0] = range._x[1];
                xmin[1] = range._y[0];
                xmax[1] = range._y[1];
                xmin[2] = range._z[0];
                xmax[2] = range._z[1];

                for (int i = 0; i < 3; i++) {
                        vm[i] = sign(displacement[i]) * displacement[i] * rpm[i] / 60.0 / gears[i];
                        scale[i] = gears[i] * microsteps[i] * steps[i] / displacement[i];
                }
                
                RSerial serial("/dev/ttyACM0", 115200, 1);        
                RomiSerialClient romi_serial(&serial, &serial);
                OquamStepper stepper(romi_serial);
                Oquam oquam(stepper, xmin, xmax, vm, amax, scale, 0.01, period);
                CNCClient client(oquam);
                ControllerServer server(&client, "oquam", "cnc");
                
                while (!app_quit())
                        clock_sleep(0.1);

                
        } catch (std::runtime_error e) {
                r_err("main: std::runtime_error: %s", e.what());
                
        } catch (std::exception e) {
                r_err("main: std::exception: %s", e.what());
        }

        return 0;
}

