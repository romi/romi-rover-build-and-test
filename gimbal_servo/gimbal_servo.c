#include "gimbal_servo.h"

static char *device = NULL;
static serial_t *serial = NULL;
static membuf_t *encoders = NULL;
static mutex_t *mutex = NULL;
static int initialized = 0;
static int encL = 0;
static int encR = 0;
static double timestamp = 0.0;
static double max_cps = 0.0;
static double encoder_steps = 0.0;

static void broadcast_log(void *userdata, const char* s)
{
        messagelink_t *get_messagelink_logger();
        messagelink_t *log = get_messagelink_logger();
        if (log)
                messagelink_send_str(log, s);
}

static int open_serial(const char *dev)
{
        r_info("Trying to open the serial connection on %s.", dev);
	
        mutex_lock(mutex);        
        serial = new_serial(dev, 115200);
        mutex_unlock(mutex);        
	
        if (serial == NULL)
                r_info("Open failed.");
        else 
                r_info("Serial connection opened.");
        
        return (serial == NULL);
}

static void close_serial()
{
        mutex_lock(mutex);        
        if (serial) delete_serial(serial);
        serial = NULL;
        mutex_unlock(mutex);        
}

static int set_device(const char *dev)
{
        close_serial();
        if (device != NULL) {
                r_free(device);
                device = NULL;
        }
        device = r_strdup(dev);
        if (device == NULL) {
                r_err("Out of memory");
                return -1;
        }
        return 0;
}

static int get_gimbal_configuration()
{
        json_object_t gimbal = client_get("configuration", "gimbal_servo");
        if (!json_isobject(gimbal)) {
                r_err("unexpected value for 'gimbal_servo'");
                json_unref(gimbal);
                return -1;
        }

        double steps = json_object_getnum(gimbal, "encoder_steps");
        double cps = json_object_getnum(gimbal, "max_cps");
        if (isnan(cps) || isnan(steps)) {
                r_err("invalid encoder steps or maximum cycles-per-second");
                json_unref(gimbal);
                return -1;
        }
        if (steps < 100 || steps > 1000000) {
                r_err("invalid encoder steps: %f !in [100,1000000]", steps);
                json_unref(gimbal);
                return -1;
        }
        if (cps < 0.01 || cps > 5000.0) {
                r_err("invalid cycles per seconds: %f !in [0.0&,5000.00]", cps);
                json_unref(gimbal);
                return -1;
        }
        encoder_steps = steps;
        max_cps = cps;

        r_info("encoder_steps  %f", encoder_steps);
        r_info("max_cps %f", max_cps);
        json_unref(gimbal);
        return 0;
}

static int send_command(const char *cmd, membuf_t *message)
{
        int err = 0;
        const char *r;
        
        mutex_lock(mutex);        
        
        if (serial == NULL) {
                membuf_printf(message, "No serial");
                err = -1;
                goto unlock;
        }        
        
        r = serial_command_send(serial, message, cmd);
        if (r == NULL) {
                err = -1;
                membuf_printf(message, "Unknown error");
        } else if (strncmp(r, "ERR", 3) == 0) {
                err = -1;
        } 
        
unlock:
        mutex_unlock(mutex);        
        return err;
}

static int reset_controller(serial_t *serial)
{
        char cmd[256];
        
        r_info("Resetting the motor controller.");

        membuf_t *reply = new_membuf();
        if (reply == NULL)
                return -1;
        
        if (send_command("X", reply) != 0) {
                r_err("Reset failed: %s", membuf_data(reply));
                return -1;
        }

        rprintf(cmd, sizeof(cmd), "I[%d,%d,90,0,0,0]",
                (int) encoder_steps,
                (int) (max_cps * 100.0));
        if (send_command(cmd, reply) != 0) {
                r_err("Initialization failed: %s", membuf_data(reply));
                return -1;
        }
        
        if (send_command("E1", reply) != 0) {
                r_err("Enable failed: %s", membuf_data(reply));
                return -1;
        }

        delete_membuf(reply);
        return 0;
}

int gimbal_init()
{
        if (initialized)
                return 0;
        
        if (get_gimbal_configuration() != 0)
                return -1;

        if (open_serial(device) != 0)
                return -1;
        
        if (reset_controller(serial) != 0) {
                close_serial();
                return -1;
        }
        
        initialized = 1;
        return 0;
}

int gimbal_servo_init(int argc, char **argv)
{
        r_log_set_writer(broadcast_log, NULL);
        
        if (argc > 2) {
                r_debug("using serial device '%s'", device);
                if (set_device(argv[1]) != 0)
                        return -1;
        }

        mutex = new_mutex();
        if (mutex == NULL)
                return -1;

        encoders = new_membuf();
        if (encoders == NULL)
                return -1;
        
        for (int i = 0; i < 10; i++) {
                if (gimbal_init() == 0)
                        return 0;
                r_err("gimbal_init failed: attempt %d/10", i);
                clock_sleep(0.2);
        }

        r_err("failed to initialize the gimbal");
        return -1;
}

void gimbal_servo_cleanup()
{
        close_serial();
        if (mutex)
                delete_mutex(mutex);
        if (encoders)
                delete_membuf(encoders);}


int gimbal_onmoveat(void *userdata,
                    messagelink_t *link,
                    json_object_t command,
                    membuf_t *message)
{
}

int gimbal_onmoveto(void *userdata,
                    messagelink_t *link,
                    json_object_t command,
                    membuf_t *message)
{
}

int gimbal_onhoming(void *userdata,
                    messagelink_t *link,
                    json_object_t command,
                    membuf_t *message)
{
        r_warn("homing not implemented");
        return 0;
}

void update_encoders()
{
        static double offset = 0.0;
        
        if (gimbal_init() != 0) {
                clock_sleep(0.2);
                return;
        }
        
        if (send_command("e", encoders) != 0) {
                r_err("brush_motors: 'e' command returned: %s",
                        membuf_data(encoders));
                return;
        }
                
        mutex_lock(mutex);        
        unsigned int millis;
        const char *r = membuf_data(encoders);
        sscanf(r, "e[%d,%d,%u]", &encL, &encR, &millis);

        double boottime = millis / 1000.0;
        if (offset == 0.0) {
                double now = clock_time();
                offset = now - boottime;
        }

        timestamp = offset + boottime;
        mutex_unlock(mutex);        
        
        clock_sleep(0.100);
}
