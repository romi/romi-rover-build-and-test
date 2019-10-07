#include <romi.h>
#include "wheel_odometry.h"

datahub_t *get_datahub_pose();

static rover_t *rover = NULL;

static void broadcast_log(void *userdata, const char* s)
{
        messagelink_t *get_messagelink_logger();
        messagelink_t *log = get_messagelink_logger();
        if (log)
                messagelink_send_str(log, s);
}

int init_rover(double left, double right, double timestamp)
{
        static double last_attempt = 0.0;

        double now = clock_time();
        if (now - last_attempt < 1.0)
                return -1;
        last_attempt = now;
        
        r_debug("trying to configure the rover's dimensions");
        
        json_object_t config = client_get("configuration", "rover");
        if (json_isnull(config)) {
                r_err("failed to load the configuration");
                json_unref(config);
                return -1;
        }
        double diam = json_object_getnum(config, "wheel_diameter");
        double base = json_object_getnum(config, "wheel_base");
        double steps = json_object_getnum(config, "encoder_steps");
        if (isnan(diam) || isnan(base) || isnan(steps)) {
                r_err("invalid configuration values");
                json_unref(config);
                return -1;
        }

        json_unref(config);
        
        r_debug("initializing rover: diam %.3f, base %.3f, steps %d",
                  diam, base, (int) steps);
        
        rover = new_rover(diam, base, steps);
        if (rover == NULL)
                return -1;
        
        rover_init_encoders(rover, left, right, timestamp);
        
        return 0;
}

int wheel_odometry_init(int argc, char **argv)
{
        r_log_set_writer(broadcast_log, NULL);
        return 0;
}

void wheel_odometry_cleanup()
{
        if (rover) 
                delete_rover(rover);
}

void wheel_odometry_ondata(void *userdata,
                           datalink_t *link,
                           data_t *input,
                           data_t *output)
{
        json_object_t data = datalink_parse(link, input);
        if (json_falsy(data)) {
                r_warn("wheel_odometry_ondata: failed to parse data");
                json_unref(data);
                return;
        }
        
        json_object_t encoders = json_object_get(data, "encoders");
        if (json_isnull(encoders) || !json_isarray(encoders)) {
                r_warn("wheel_odometry_ondata: expected an array for the encoder values");
                json_unref(data);
                return;
        }

        double left = json_array_getnum(encoders, 0);
        double right = json_array_getnum(encoders, 1);
        double timestamp = json_object_getnum(data, "timestamp");
        
        if (isnan(left) || isnan(right) || isnan(timestamp)) {
                r_warn("wheel_odometry_ondata: invalid values");
                json_unref(data);
                return;
        }

        if (rover == NULL) {
                init_rover(left, right, timestamp);
                if (rover == NULL) return;
                
        } else {
                rover_set_encoders(rover, left, right, timestamp);
        }

        //r_debug("encoders: %f, %f", left, right);
        
        vector_t position;
        vector_t speed;
        quaternion_t orientation;
        rover_get_pose(rover, &position, &speed, &orientation);

        if (0) {
                static double last_print = 0.0;
                double now = clock_time();
                if (now - last_print > 3) {
                        //r_info("position: x %f, y %f", position.x, position.y);
                        last_print = now;
                }
        }
        
        datahub_broadcast_f(get_datahub_pose(),
                            NULL,
                            "{\"position\":[%f,%f,%f],"
                            "\"speed\":[%f,%f,%f],"
                            "\"orientation\":[%f,%f,%f,%f],"
                            "\"timestamp\":%f}",
                            position.x, position.y, position.z,
                            speed.x, speed.y, speed.z,
                            orientation.s, orientation.v.x,
                            orientation.v.y, orientation.v.z,
                            timestamp);
        json_unref(data);
}



