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
#include <stdlib.h>
#include <unistd.h>
#include "control_panel.h"

#define DISPLAY_LENGTH 16

messagehub_t *get_messagehub_watchdog();

enum {
        PANEL_STATE_ERROR = 0,
        PANEL_STATE_STARTING_UP,
        PANEL_STATE_POWERING_UP,
        PANEL_STATE_ON,
        PANEL_STATE_SHUTTING_DOWN,
        PANEL_STATE_OFF
};

const char *panel_state_str[] = {"error", "starting_up", "powering_up",
                                 "on", "shutting_down", "off"};

enum {
        APP_STATE_INITIALIZING = 0,
        APP_STATE_INITIALIZED,
        APP_STATE_POWERING_UP,
        APP_STATE_POWERED_UP,
        APP_STATE_ERROR
};

const char *app_state_str[] = {"initializing", "initialized",
                               "powering_up", "powered_up", "error"};

static char *device = NULL;
static serial_t *serial = NULL;
static membuf_t *state_buf = NULL;
static mutex_t *mutex = NULL;
static list_t *apps = NULL;
static double timeout = 10.0;
static int prev_apps_state = APP_STATE_ERROR;
static int prev_panel_state = PANEL_STATE_ERROR;

static void broadcast_log(void *userdata, const char* s)
{
        messagelink_t *get_messagelink_logger();
        static int _inside_log_writer = 0;
        messagelink_t *log = get_messagelink_logger();
        if (log != NULL && _inside_log_writer == 0) {
                _inside_log_writer = 1;
                messagelink_send_str(log, s);
                _inside_log_writer = 0;
        }
}

static int open_serial(const char *dev)
{
        r_info("Trying to open the serial connection on %s.", dev);
	
        mutex_lock(mutex);        
        serial = new_serial(dev, 115200, 0);
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

static int send_command_unlocked(const char *cmd, membuf_t *message)
{
        const char *r;

        membuf_clear(message);
        
        if (serial == NULL) {
                membuf_printf(message, "No serial");
                return -1;
        }        
        
        r = serial_command_send(serial, message, cmd);
        if (r == NULL) {
                membuf_printf(message, "Unknown error");
                return -1;
        } else if (strncmp(r, "ERR", 3) == 0) {
                return -1;
        } 
        
        return 0;
}

static int send_command(const char *cmd, membuf_t *message)
{
        int err;
        mutex_lock(mutex);        
        err = send_command_unlocked(cmd, message);
        mutex_unlock(mutex);        
        return err;
}

static int set_state(int state)
{
        char cmd[18];
        rprintf(cmd, sizeof(cmd), "S%d", state);
        return send_command(cmd, state_buf);
}

static int display_message(const char *s)
{
        char cmd[20];
        if (strlen(s) < DISPLAY_LENGTH)
                rprintf(cmd, sizeof(cmd), "D\"%s\"", s);
        else {
                char m[DISPLAY_LENGTH+1];
                memcpy(m, s, DISPLAY_LENGTH);
                m[DISPLAY_LENGTH] = 0;
                rprintf(cmd, sizeof(cmd), "D\"%s\"", m);                
        }
        return send_command(cmd, state_buf);
}

static void set_error(const char *s)
{
        display_message(s);
        set_state(PANEL_STATE_ERROR);
        r_err("%s", s);
}

static int get_state()
{
        int s = PANEL_STATE_ERROR;

        mutex_lock(mutex);
        s = send_command_unlocked("s", state_buf);
        if (s != 0) {
                s = -1;
                goto unlock;
        }
        
        membuf_append_zero(state_buf);
        const char* r = membuf_data(state_buf);
        if (r[0] != 's') {
                s = -2;
                goto unlock;
        }

        s = atoi(r + 1);
        
unlock:
        mutex_unlock(mutex);
        return s;
}

typedef struct _app_state_t {
        char *name;
        int state;
        double timestamp;
} app_state_t;

static app_state_t *new_app_state(const char *name)
{
        app_state_t *s = r_new(app_state_t);
        if (s == NULL)
                return NULL;
        s->name = r_strdup(name);
        if (s->name == NULL) {
                r_delete(s);
                return NULL;
        }
        s->state = APP_STATE_INITIALIZING; 
        s->timestamp = clock_time();
        return s;
}

__attribute_used__
static void delete_app_state(app_state_t *s)
{
        if (s) {
                if (s->name)
                        r_free(s->name);
                r_delete(s);
        }
}

static int add_app_state(const char *name)
{
        app_state_t *s = new_app_state(name);
        if (s == NULL)
                return -1;

        apps = list_append(apps, s);
        if (apps == NULL)
                return -1;
        
        return 0;
}

static app_state_t *get_app_state(const char *name)
{
        for (list_t *l = apps; l != NULL; l = list_next(l)) {
                app_state_t *s = list_get(l, app_state_t);
                if (rstreq(s->name, name))
                        return s;
        }
        return NULL;
}

static void app_set_state(json_object_t message)
{
        const char *name = json_object_getstr(message, "name");
        if (name == NULL) {
                r_warn("app_set_ready: missing name");
                return;
        }
        
        const char *state = json_object_getstr(message, "state");
        if (state == NULL) {
                r_warn("app_set_ready: missing state");
                return;
        }
        
        app_state_t *s = get_app_state(name);
        if (s == NULL) {
                r_warn("app_set_ready: can't find app named '%s'", name);
                return;
        }

        int last_state = s->state;
                
        if (rstreq(state, "initializing"))
                s->state = APP_STATE_INITIALIZING;
        else if (rstreq(state, "initialized"))
                s->state = APP_STATE_INITIALIZED;
        else if (rstreq(state, "powering_up"))
                s->state = APP_STATE_POWERING_UP;
        else if (rstreq(state, "powered_up"))
                s->state = APP_STATE_POWERED_UP;
        else if (rstreq(state, "error"))
                s->state = APP_STATE_ERROR;
        else {
                r_err("app '%s' returned unknown state '%s'", name, state);
                s->state = APP_STATE_ERROR;
        }
        s->timestamp = clock_time();

        if (last_state != s->state)
                r_debug("app_set_state: state(%s)=%s", name, state);
}

/*  Apps 0 to n   App n+1                               State
 *  ---           ---                                   ---
 *  error         ?                                     error
 *  ?             error                                 error
 *  -
 *  initializing  initializing, initialized             initializing
 *  initializing  powering_up, powered_up               error
 *  -
 *  initialized   initializing                          initializing
 *  initialized   initialized                           initialized
 *  initialized   powering_up, powered_up               powering_up
 *  -
 *  powering_up   initializing                          error
 *  powering_up   initialized, powering_up, powered_up  powering_up
 *  -
 *  powered_up    initializing                          error
 *  powered_up    initialized                           powering_up
 *  powered_up    powering_up                           powering_up
 *  powered_up    powered_up                            powered
 *
 *  If an app hasn't responded to an state update request for more
 *  than three seconds, the state switches to 'error'.
 *
 */
static int get_apps_state()
{
        double t = clock_time();
        
        list_t *l = apps;
        if (l == NULL)
                return APP_STATE_INITIALIZING;
        
        app_state_t *s = list_get(l, app_state_t);
        int state = s->state;
        if (t - s->timestamp > timeout) {
                r_err("app '%s' timed out", s->name);
                return APP_STATE_ERROR;
        }
                
        l = list_next(l);
        
        while (l != NULL) {
                s = list_get(l, app_state_t);

                if (t - s->timestamp > timeout) {
                        r_err("app '%s' timed out", s->name);
                        return APP_STATE_ERROR;
                }

                switch (state) {
                case APP_STATE_ERROR:
                        return APP_STATE_ERROR;
                        
                case APP_STATE_INITIALIZING:
                        if (s->state == APP_STATE_INITIALIZING
                            || s->state == APP_STATE_INITIALIZED)
                                state = APP_STATE_INITIALIZING;
                        else if (s->state == APP_STATE_POWERING_UP
                                 || s->state == APP_STATE_POWERED_UP)
                                return APP_STATE_ERROR;
                        break;
                        
                case APP_STATE_INITIALIZED:
                        if (s->state == APP_STATE_INITIALIZING)
                                state = APP_STATE_INITIALIZING;
                        else if (s->state == APP_STATE_INITIALIZED)
                                state = APP_STATE_INITIALIZED;
                        else if (s->state == APP_STATE_POWERING_UP
                                 || s->state == APP_STATE_POWERED_UP)
                                state = APP_STATE_POWERING_UP;
                        break;
                        
                case APP_STATE_POWERING_UP:
                        if (s->state == APP_STATE_INITIALIZING)
                                return APP_STATE_ERROR;
                        else if (s->state == APP_STATE_INITIALIZED
                                || s->state == APP_STATE_POWERING_UP
                                || s->state == APP_STATE_POWERED_UP)
                                state = APP_STATE_POWERING_UP;
                        break;

                case APP_STATE_POWERED_UP:
                        if (s->state == APP_STATE_INITIALIZING)
                                return APP_STATE_ERROR;
                        else if (s->state == APP_STATE_INITIALIZED
                                || s->state == APP_STATE_POWERING_UP)
                                state = APP_STATE_POWERING_UP;
                        else if (s->state == APP_STATE_POWERED_UP)
                                state = APP_STATE_POWERED_UP;
                        break;
                }
                
                l = list_next(l);
        }
        
        return state;
}

static int apps_ask_state()
{
        messagehub_t *hub = get_messagehub_watchdog();
        if (hub == NULL) {
                r_err("apps_ask_state: hub is NULL");
                return -1;
        }
                
        const char *s = "{\"request\": \"state?\"}";
        return messagehub_broadcast_text(hub, NULL, s, strlen(s));
}

static int apps_power_up()
{
        messagehub_t *hub = get_messagehub_watchdog();
        if (hub == NULL) {
                r_err("apps_ask_state: hub is NULL");
                return -1;
        }

        const char *s = "{\"request\": \"power_up\"}";
        return messagehub_broadcast_text(hub, NULL, s, strlen(s));
}

static int get_apps()
{
        json_object_t config = client_get("configuration", "watchdog");
        if (!json_isobject(config)) {
                r_err("unexpected value for 'watchdog'");
                json_unref(config);
                return -1;
        }

        json_object_t applist = json_object_get(config, "apps");
        if (!json_isarray(applist)) {
                r_err("invalid watchdog settings. Expected an array for apps.");
                json_unref(config);
                return -1;
        }

        for (int i = 0; i < json_array_length(applist); i++) {
                const char *name = json_array_getstr(applist, i);
                int err = add_app_state(name);
                if (err != 0) {
                        json_unref(config);
                        return -1;
                }
        }
        json_unref(config);
        return 0;
}

static int get_device()
{
        json_object_t config = client_get("configuration", "control_panel");
        if (!json_isobject(config)) {
                r_err("unexpected value for 'brush_motors'");
                json_unref(config);
                return -1;
        }

        const char *dev = json_object_getstr(config, "device");
        if (dev == NULL) {
                r_err("invalid control panel settings");
                json_unref(config);
                return -1;
        }
        
        if (set_device(dev) != 0) {
                json_unref(config);
                return -1;
        }
        
        json_unref(config);
        return 0;
}

static int get_configuration()
{
        if (get_device() != 0)
                return -1;

        if (get_apps() != 0)
                return -1;

        return 0;
}

int control_panel_init(int argc, char **argv)
{ 
        r_log_set_writer(broadcast_log, NULL);

        mutex = new_mutex();
        if (mutex == NULL)
                return -1;

        if (argc > 1) {
                r_debug("using serial device '%s'", argv[1]);
                if (set_device(argv[1]) != 0)
                        return -1;
        }

        state_buf = new_membuf();
        if (state_buf == NULL)
                return -1;

        if (device == NULL) {
                for (int i = 0; i < 10; i++) {
                        if (get_configuration() == 0)
                                break;
                        clock_sleep(0.2);
                }
        }
        
        if (device == NULL)
                return -1;

        if (open_serial(device) != 0)
                return -1;

        if (display_message("Watchdog started")) {
                close_serial();
                return -1;                
        }

        return 0;
}

void control_panel_cleanup()
{
        close_serial();
        if (mutex)
                delete_mutex(mutex);
        if (state_buf)
                delete_membuf(state_buf);
}

static void shutdown_system()
{
	r_info("Shutting down");
        FILE *fp = popen("/sbin/poweroff", "r");
        while (!feof(fp) && !ferror(fp)) {
                char c;
                fread(&c, 1, 1, fp);
        }
        fclose(fp);
}

int control_panel_shutdown(void *userdata,
                           messagelink_t *link,
                           json_object_t command,
                           membuf_t *message)
{
        shutdown_system();
        return 0;
}

int control_panel_display(void *userdata,
                          messagelink_t *link,
                          json_object_t command,
                          membuf_t *message)
{
	r_info("Display");
        return 0;
}

void watchdog_onmessage(void *userdata,
                        messagelink_t *link,
                        json_object_t message)
{
        //r_debug("watchdog_onmessage");
        const char *r = json_object_getstr(message, "request");
        if (r == NULL)
                r_warn("watchdog_onmessage: invalid message: empty request");
        else if (rstreq(r, "state"))
                app_set_state(message);
        else 
                r_warn("watchdog_onmessage: unhandled message: %s", r);
}

void watchdog_onidle()
{
        int err = apps_ask_state();
        if (err != 0) {
                set_error("ERR #000");
                return;
        }
        
        clock_sleep(1);

        int apps_state = get_apps_state();
        int panel_state = get_state();
        
        if (panel_state < 0) {
                set_error("ERR #002");
                clock_sleep(1);
                return;
        }

        if (apps_state != prev_apps_state)
                r_info("Apps: %s", app_state_str[apps_state]);
        if (panel_state != prev_panel_state)
                r_info("Panel: %s", panel_state_str[panel_state]);
        
        prev_apps_state = apps_state;
        prev_panel_state = panel_state;
        
        if (panel_state == PANEL_STATE_ERROR) {
                clock_sleep(1);
                return;
        }
        
        if (panel_state == PANEL_STATE_SHUTTING_DOWN) {
                shutdown_system();
                return;
        }
        
        
        if (apps_state == APP_STATE_ERROR) {
                set_error("ERR #001");
                clock_sleep(1);
                return;
        }

        if (panel_state == PANEL_STATE_STARTING_UP) {
                if (apps_state == APP_STATE_INITIALIZED) {
                        
                        if (set_state(PANEL_STATE_POWERING_UP) != 0)
                                set_error("ERR #003");
                        
                        if (apps_power_up() != 0)
                                set_error("ERR #005");
                        
                        // Reduce the timeout
                        timeout = 3.0;
                }
                
        } else if (panel_state == PANEL_STATE_POWERING_UP) {
                if (apps_state == APP_STATE_POWERED_UP) {
                        
                        if (set_state(PANEL_STATE_ON) != 0)
                                set_error("ERR #003");
                }
                
        }
}
