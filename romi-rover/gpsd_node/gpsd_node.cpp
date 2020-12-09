/*
  romi-rover

  Copyright (C) 2019 Sony Computer Science Laboratories
  Author(s) Peter Hanappe

  romi-rover is collection of applications for the Romi Rover.

  romi-rover is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see
  <http://www.gnu.org/licenses/>.

 */
#include <libgpsmm.h>
#include <unistd.h>
#include "gpsd_node.h"

datahub_t *get_datahub_gps();

gpsmm *gpsd = 0;

void gpsd_connect(const char *port)
{
        if (gpsd == 0) {
                gpsd = new gpsmm("localhost", port);
                if (gpsd->stream(WATCH_ENABLE|WATCH_JSON) == NULL) {
                        r_err("No GPSD running");
                        delete gpsd;
                        gpsd = 0;
    }        }
}

void gpsd_disconnect()
{
        if (gpsd) {
                delete gpsd;
                gpsd = 0;
        }
}

int gpsd_node_init(int argc, char **argv)
{
        const char *port = DEFAULT_GPSD_PORT;
        if (argc == 2)
                port = argv[1];
        gpsd_connect(port);
        return 0;
}

void gpsd_node_cleanup()
{
        gpsd_disconnect();
}

static void send_gps(struct gps_data_t* data) 
{
        if ((data->set & LATLON_SET)
            && (data->set & ALTITUDE_SET)
            && (data->set & TIME_SET)) {

                // FIXME: in the following online example
                // (https://github.com/rnorris/gpsd/blob/master/test_gpsmm.cpp),
                // data->dev.path is read after a test to (data->set &
                // DEVICE_SET).  This file seems to indicate the same
                // although it's not clear:
                // https://gpsd.gitlab.io/gpsd/protocol-transition.html
                // This does not seem to work however. 
                
                // const char *device = "?";
                // if (data->set & DEVICE_SET)
                //         device = data->dev.path;
                
                const char *device = "?";
                if (data->dev.path != NULL)
                        device = data->dev.path;

                datahub_broadcast_f(get_datahub_gps(), NULL,
                                    "{\"position\":[%f,%f,%f],"
                                    "\"orientation\":[1,0,0,0],"
                                    "\"timestamp\":%f,"
                                    "\"device\":\"%s\"}",
                                    data->fix.longitude,
                                    data->fix.latitude,
                                    data->fix.altitude,
                                    data->fix.time,
                                    device);
                r_debug("{\"position\":[%f,%f,%f],"
                        "\"orientation\":[1,0,0,0],"
                        "\"timestamp\":%f,"
                        "\"device\":\"%s\"}",
                        data->fix.longitude,
                        data->fix.latitude,
                        data->fix.altitude,
                        data->fix.time,
                        device);
        } else {
                datahub_broadcast_f(get_datahub_gps(), NULL, "{\"error\":\"no fix\"}");
                r_debug("no fix");
        }
}

static void read_gps() 
{
        struct gps_data_t* data;

        if (gpsd->waiting(3000000)) {
                data = gpsd->read();
                if (data != NULL && get_datahub_gps() != NULL) {
                        send_gps(data); 
                } else {
                        r_err("gpsd_node: read error.");
                }
        } else {
                r_debug("gpsd_node: waiting timed-out");
                clock_sleep(0.5);
        }
}

int gpsd_node_broadcast_position(void *userdata, datahub_t* hub)
{
        if (gpsd)
                read_gps();
        return 0;
}


