#include <string.h>
#include <math.h>
#include "fake_motors.h"

static double encoders[] = { 0.0, 0.0 };
static double speed[] = { 0.0, 0.0 };
static double max_speed = 0.5; // in meters per seconds
static double steps_per_meter; 
static int enabled = 0; 
static mutex_t *mutex = NULL;
static double wheel_circumference;
static double encoder_steps;
static int rover_initialized = 0;

enum {
        APP_STATE_INITIALIZING = 0,
        APP_STATE_INITIALIZED,
        APP_STATE_POWERING_UP,
        APP_STATE_POWERED_UP,
        APP_STATE_ERROR
};

static int state = APP_STATE_INITIALIZING;


datahub_t *get_datahub_encoders();
messagehub_t *get_messagehub_status();

static void broadcast_log(void *userdata, const char* s)
{
        messagelink_t *get_messagelink_logger();
        messagelink_t *log = get_messagelink_logger();
        if (log)
                messagelink_send_str(log, s);
}

int init_rover()
{
        static double last_attempt = 0.0;

        if (state >= APP_STATE_INITIALIZED)
                return 0;
        
        double now = clock_time();
        if (now - last_attempt < 1.0)
                return -1;
        last_attempt = now;
        
        r_debug("trying to configure the rover's dimensions");
        
        json_object_t config = client_get("configuration", "rover");
        if (json_falsy(config)) {
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

        r_debug("initializing rover: diam %.2f, base %.2f, steps %d",
                  diam, base, (int) steps);
        
        wheel_circumference = M_PI * diam;
        encoder_steps = steps;
        steps_per_meter = encoder_steps / wheel_circumference;
        state = APP_STATE_INITIALIZED;

        json_unref(config);
        return 0;
}

int fake_motors_init(int argc, char **argv)
{
        r_log_set_writer(broadcast_log, NULL);
        mutex = new_mutex();
        init_rover();
        return 0;
}

void fake_motors_cleanup()
{
        if (mutex)
                delete_mutex(mutex);
}

int motorcontroller_onmoveat(void *userdata,
                             messagelink_t *link,
                             json_object_t command,
                             membuf_t *message)
{
        r_debug("motorcontroller_onmoveat");

        json_object_t s = json_object_get(command, "speed");
        if (!json_isarray(s)) {
                r_warn("Speed value is not an array");
                membuf_printf(message, "Speed value is not an array");
                return -1;
        }
        
        double left = json_array_getnum(s, 0);
        double right = json_array_getnum(s, 1);
        if (isnan(left) || isnan(right)) {
                r_warn("invalid left|right values: %f, %f", left, right);
                membuf_printf(message, "Invalid left|right values");
                return -1;
        }

        if (left < -1000.0 || left > 1000.0
            || right < -1000.0 || right > 1000.0) {
                r_warn("Speed is out of bounds [-1000, 1000].");
                membuf_printf(message, "Speed is out of bounds [-1000, 1000]");
                return -1;
        }

        mutex_lock(mutex);
        speed[0] = left / 1000.0;
        speed[1] = right / 1000.0;
        mutex_unlock(mutex);
        
        return 0;
}

int motorcontroller_onenable(void *userdata,
                             messagelink_t *link,
                             json_object_t command,
                             membuf_t *message)
{
        r_debug("motorcontroller_onenable");

        double value = json_object_getnum(command, "value");
        if (isnan(value)) {
                r_warn("missing value for enable");
                membuf_printf(message, "missing value for enable");
                return -1;
        }

        mutex_lock(mutex);
        enabled = (value != 0);
        mutex_unlock(mutex);
        
        return 0;
}

int motorcontroller_onreset(void *userdata,
                            messagelink_t *link,
                            json_object_t command,
                            membuf_t *message)
{
        r_debug("motorcontroller_onreset");

        mutex_lock(mutex);
        enabled = 0;
        speed[0] = 0.0;
        speed[1] = 0.0;
        encoders[0] = 0.0;
        encoders[1] = 0.0;
        mutex_unlock(mutex);
        
        return 0;
}

int motorcontroller_onhoming(void *userdata,
                             messagelink_t *link,
                             json_object_t command,
                             membuf_t *message)
{
        r_debug("motorcontroller_onhoming");
        return 0;
}

void broadcast_encoders(void *userdata, datahub_t *hub)
{
        static double last_time = 0.0;
        
        double timestamp = clock_time();

        if (state == APP_STATE_INITIALIZING
            && init_rover() != 0) 
                return;
        
        mutex_lock(mutex);

        if (last_time == 0.0) {
                last_time = timestamp;
        } else {
                double dt = timestamp - last_time;
                encoders[0] += steps_per_meter * max_speed * speed[0] * dt;
                encoders[1] += steps_per_meter * max_speed * speed[1] * dt;
                last_time = timestamp;
        }
        
        datahub_broadcast_f(hub, NULL, 
                            "{\"encoders\":[%d,%d], \"timestamp\": %f}",
                            (int) encoders[0], (int) encoders[1], timestamp);

        mutex_unlock(mutex);
        clock_sleep(0.050);
}

void broadcast_status()
{
        static char buffer[200];
        const char *state_str;
        
        mutex_lock(mutex);

        if (!rover_initialized)
                state_str = "Initializing";
        else
                state_str = enabled? "Enabled" : "Disabled";

        snprintf(buffer, sizeof(buffer),
                 "S['%s','None','Serial','Direct',"
                 "%.3f,%.3f,%.3f,%.3f,"
                 "%.3f,%.3f,%.3f,%.3f,"
                 "1500,1500]",
                 state_str, 
                 speed[0], speed[1], max_speed * speed[0], max_speed * speed[1],
                 speed[0], speed[1], speed[0], speed[1]);
        mutex_unlock(mutex);
        clock_sleep(1);
}

static const char *state_str(int s)
{
        switch (s) {
        case APP_STATE_INITIALIZING:
                return "initializing";
        case APP_STATE_INITIALIZED:
                return "initialized";
        case APP_STATE_POWERING_UP:
                return "powering_up";
        case APP_STATE_POWERED_UP:
                return "powered_up";
        case APP_STATE_ERROR: 
        default:
                return "error";
        }
}

void watchdog_onmessage(void *userdata,
                        messagelink_t *link,
                        json_object_t message)
{
        //r_debug("watchdog_onmessage");
        const char *r = json_object_getstr(message, "request");
        if (r == NULL)
                r_warn("watchdog_onmessage: invalid message: empty request");
        else if (rstreq(r, "state?"))
                messagelink_send_f(link,
                                   "{\"request\": \"state\", "
                                   "\"name\": \"fake_motors\", "
                                   "\"state\": \"%s\"}", state_str(state));
        else if (rstreq(r, "power_up"))
                state = APP_STATE_POWERED_UP;
}
