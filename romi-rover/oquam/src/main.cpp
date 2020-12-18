#include <exception>
#include <stdexcept>
#include <string.h>
#include <rcom.h>
#include <getopt.h>

#include "Oquam.h"
#include "StepperController.h"
#include "RomiSerialClient.h"
#include "RSerial.h"
#include "ConfigurationFile.h"
#include "DebugWeedingSession.h"
#include "RPCServer.h"
#include "RPCCNCServerAdaptor.h"
#include "FakeCNCController.h"

using namespace romi;


static inline double sign(double v)
{
        return (v < 0)? -1.0 : 1.0;
}

struct Options {
        
        const char *config_file;
        const char *serial_device;
        const char *server_name;
        const char *cnc_controller;
        const char *output_directory;

        Options() {
                config_file = "config.json";
                serial_device = "/dev/ttyACM0";
                server_name = "oquam";
                cnc_controller = "oquam";
                output_directory = ".";
        }

        void parse(int argc, char** argv) {
                int option_index;
                static const char *optchars = "C:N:T:D:";
                static struct option long_options[] = {
                        {"config", required_argument, 0, 'C'},
                        {"device", required_argument, 0, 'D'},
                        {"navigation-server-name", required_argument, 0, 'N'},
                        {"cnc-controller", required_argument, 0, 'c'},
                        {"output-directory", required_argument, 0, 'd'},
                        {0, 0, 0, 0}
                };
        
                while (1) {
                        int c = getopt_long(argc, argv, optchars,
                                            long_options, &option_index);
                        if (c == -1) break;
                        switch (c) {
                        case 'C':
                                config_file = optarg;
                                break;
                        case 'N':
                                server_name = optarg;
                                break;
                        case 'D':
                                serial_device = optarg;
                                break;
                        case 'c':
                                cnc_controller = optarg;
                                break;
                        case 'd':
                                output_directory = optarg;
                                break;
                        }
                }
        }
};
        

int main(int argc, char** argv)
{
        int retval = 1;
        Options options;
        options.parse(argc, argv);
        
        app_init(&argc, argv);
        app_set_name(options.server_name);

        ICNCController *cnc_controller = 0;
        
        try {
                ConfigurationFile config(options.config_file);
                
                CNCRange range;
                JsonCpp r = config.get("cnc").get("range");
                range.init(r);
                
                double xmin[3];
                double xmax[3];
                double vm[3];
                double steps[3] = { 200.0, 200.0, 200.0 };
                double gears[3] = { 1.0, 1.0, 1.0 };
                double rpm[3] = { 300.0, 300.0, 300.0 };
                double displacement[3] = { 0.04, 0.04, -0.002 };
                double microsteps[3] = { 8.0, 8.0, 1.0 };
                double scale[3];
                double period = 0.020;
                
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
                
                if (rstreq(options.cnc_controller, FakeCNCController::ClassName)) {
                        cnc_controller = new FakeCNCController();
                } else {
                        RSerial serial(options.serial_device, 115200, 1);        
                        RomiSerialClient romi_serial(&serial, &serial);
                        cnc_controller = new StepperController(romi_serial);
                }

                Oquam oquam(cnc_controller, xmin, xmax, vm, amax, scale, 0.01, period);

                DebugWeedingSession debug(options.output_directory, "oquam");
                oquam.set_file_cabinet(&debug);
                
                RPCCNCServerAdaptor adaptor(oquam);
                rcom::RPCServer server(adaptor, "oquam", "cnc");

                
                while (!app_quit())
                        clock_sleep(0.1);

                retval = 0;
                
        } catch (std::exception &e) {
                r_err("main: std::exception: %s", e.what());
        }

        if (cnc_controller)
                delete cnc_controller;

        return retval;
}

