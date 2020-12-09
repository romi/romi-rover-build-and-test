#include <exception>
#include <string.h>
#include <rcom.h>

#include "ConfigurationFile.h"
#include "CameraProxy.h"
#include "CameraFile.h"
#include "Weeder.h"
#include "ControllerClient.h"
#include "ControllerServer.h"
#include "CNCProxy.h"
#include "Pipeline.h"

using namespace romi;

int main(int argc, char** argv)
{
        app_init(&argc, argv);

        try {
                IConfiguration *config = new ConfigurationFile("config.json");
                //ICamera *camera = new CameraProxy("camera", "camera.jpg");
                ICamera *camera = new CameraFile("camera.jpg");

                
                IController *controller = new ControllerClient("weeder", "cnc");
                CNCProxy *cnc = new CNCProxy(controller);

                CNCRange range;
                //cnc->get_range(range);
                range.init(config->get("cnc").get("range"));
                
                r_debug("main: range: %f, %f", range._x[0], range._x[1]);
                               
                IPipeline *pipeline = new Pipeline(range, config->get());
                
                Weeder *weeder =  new Weeder(config, camera, pipeline, cnc, range);
                ControllerServer *server = new ControllerServer(weeder, "weeder", "weeder");
                
                while (!app_quit())
                        clock_sleep(0.1);

                delete server;
                delete weeder;
                delete cnc;
                delete pipeline;
                delete controller;
                delete camera;
                delete config;
                
        } catch (std::exception& e) {
                r_err(e.what());
        }

        return 0;
}

