#include <romi.h>
#include "navigation.h"

static mutex_t *position_mutex;
static vector_t position = {0};
static int moving = 0;

messagelink_t *get_messagelink_motorcontroller();

int navigation_init(int argc, char **argv)
{
        position_mutex = new_mutex();
        return 0;
}

void navigation_cleanup()
{
        if (position_mutex)
                delete_mutex(position_mutex);
}

void navigation_onpose(void *userdata,
                       datalink_t *link,
                       data_t *input,
                       data_t *output)
{
        json_object_t data = datalink_parse(link, input);
        if (json_falsy(data)) {
                log_warn("camera_recorder_onpose: failed to parse data");
                json_unref(data);
                return;
        }

        json_object_t a = json_object_get(data, "position");
        if (!json_isarray(a)) {
                log_warn("camera_recorder_onpose: 'position' is not an array");
                json_unref(data);
                return;
        }
        
        double px, py, pz;
        px = json_array_getnum(a, 0);
        py = json_array_getnum(a, 1);
        pz = json_array_getnum(a, 2);
        if (isnan(px) || isnan(py) || isnan(pz)) {
                log_warn("camera_recorder_onpose: invalid position values"); 
                json_unref(data);
                return;
        }
        
        mutex_lock(position_mutex);
        position.x = px;
        position.y = py;
        position.z = pz;
        mutex_unlock(position_mutex);

        json_unref(data);
}

int status_error(json_object_t reply, membuf_t *message)
{
        const char *status = json_object_getstr(reply, "status");
        if (status == NULL) {
                log_err("invalid status");
                membuf_printf(message, "invalid status");
                return -1;
                
        } else if (rstreq(status, "ok")) {
                return 0;
                
        } else if (rstreq(status, "error")) {
                const char *s = json_object_getstr(reply, "message");
                log_err("message error: %s", s);
                membuf_printf(message, "%s", s);
                return -1;
        }
        return -1; // ??
}

static int move(double distance, double speed, membuf_t *message)
{
        json_object_t reply;
        double pos;
        messagelink_t *motors = get_messagelink_motorcontroller();

        log_info("getting current position");

        mutex_lock(position_mutex);
        pos = position.x;
        mutex_unlock(position_mutex);

        log_info("current position: %f", pos);
                
        double target = pos + distance;
        if (distance * speed < 0)
                speed = -speed;
        
        log_info("starting to move");
        
        reply = messagelink_send_command_f(motors, "{'command':'moveat','speed':[%d,%d]}",
                                           (int) speed, (int) speed);
        if (status_error(reply, message))
                return -1;

        
        while (1) {
                mutex_lock(position_mutex);
                pos = position.x;
                mutex_unlock(position_mutex);

                log_debug("Distance to go: %f", target - pos);

                if ((speed < 0 && pos <= target)
                    || (speed > 0 && pos >= target)) {
                        break;
                }
                
                clock_sleep(0.010);
        }

        reply = messagelink_send_command_f(motors, "{'command':'moveat','speed':[0,0]}");
        if (status_error(reply, message)) {
                
                // FIXME, TODO: tell watchdog to cut the current!
                
                return -1;
        }
        
        return 0;
}

int navigation_onmove(void *userdata,
                      messagelink_t *link,
                      json_object_t command,
                      membuf_t *message)
{
	log_debug("navigation_onmove");

        double distance = json_object_getnum(command, "distance");
        if (isnan(distance)) {
                log_warn("Invalid distance");
                membuf_printf(message, "Invalid distance");
                return -1;
        }

        double speed = 100.0;
        if (json_object_has(command, "speed")) {
                speed = json_object_getnum(command, "speed");
                if (isnan(speed)) {
                        log_warn("Invalid speed");
                        membuf_printf(message, "Invalid speed");
                        return -1;
                }
        }

        return move(distance, speed, message);
}

