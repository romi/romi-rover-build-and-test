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
#include "ControlPanelLcdKeypad/States.h"

#define DISPLAY_LENGTH 16

static char *device = NULL;
static serial_t *serial = NULL;
static membuf_t *state_buf = NULL;
static mutex_t *mutex = NULL;
static json_object_t scripts;

static int execute_script(int id);

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
        if (serial)
                delete_serial(serial);
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
        }
        membuf_append_zero(message);
        
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

/* static int display_message(const char *s) */
/* { */
/*         char cmd[20]; */
/*         if (strlen(s) < DISPLAY_LENGTH) */
/*                 rprintf(cmd, sizeof(cmd), "D\"%s\"", s); */
/*         else { */
/*                 char m[DISPLAY_LENGTH+1]; */
/*                 memcpy(m, s, DISPLAY_LENGTH); */
/*                 m[DISPLAY_LENGTH] = 0; */
/*                 rprintf(cmd, sizeof(cmd), "D\"%s\"", m);                 */
/*         } */
/*         return send_command(cmd, state_buf); */
/* } */

static int set_menu_item(const char *name, int id)
{
        char cmd[64];
        rprintf(cmd, sizeof(cmd), "M[\"%s\",%d]", name, id);
        return send_command(cmd, state_buf);
}

static int start_shutdown()
{
        int r = -1;
	r_info("Shutting down");
        FILE *fp = popen("/sbin/poweroff", "r");
        if (fp != NULL) {
                while (!feof(fp) && !ferror(fp)) {
                        char c;
                        fread(&c, 1, 1, fp);
                }
                fclose(fp);
                r = 0;
        }
        return r;
}

int control_panel_shutdown(void *userdata,
                           messagelink_t *link,
                           json_object_t command,
                           membuf_t *message)
{
        int r = -1;        
        if (send_command("0", state_buf) == 0) {
                const char* t = membuf_data(state_buf);
                if (strncmp(t, "OK", 2) == 0) {
                        r = start_shutdown();
                } else {
                        r_err("control_panel_shutdown: start_shutdown failed: %s", t);
                        membuf_printf(message, "start_shutdown failed: %s", t);
                }
        } else {
                r_err("control_panel_shutdown: send_command failed");
                membuf_printf(message, "send_command failed");
        }
        return r;
}

static int handle_state(const char *state, int menu)
{
        int r = -1;
        r_debug("State %s", state);
        if (rstreq(state, STATE_SENDING_STR)) {
                r_debug("Execute script %d", menu);
                r = execute_script(menu);
        } else if (rstreq(state, STATE_SHUTTING_DOWN_STR)) {
                r = start_shutdown();
        }
        return r;
}

static int analyse_state(json_object_t state)
{
        int r = -1;
        if (json_isarray(state)) {
                const char *s = json_array_getstr(state, 0);
                double menu = json_array_getnum(state, 1);
                if (s != NULL && !isnan(menu)) {
                        r = handle_state(s, (int) menu);
                } else {
                        r_warn("handle_state_str: invalid state");
                }
        } else {
                r_warn("handle_state_str: state is not a valid json array");
        }
        return r;
}

static json_object_t get_state()
{
        json_object_t r = json_null();
        
        mutex_lock(mutex);
        int s = send_command_unlocked("S", state_buf);
        if (s == 0) {
                const char* t = membuf_data(state_buf);
                if (t[0] == 'S') 
                        r = json_parse(t+1);
        }
        
        mutex_unlock(mutex);
        return r;
}

static int get_device()
{
        int r = -1;
        if (device == NULL) {
                const char *dev = NULL;
                json_object_t port = client_get("configuration",
                                                "ports/control_panel/port");
                if (!json_isstring(port)) {
                        r_err("missing value for 'ports' in the configuration");
                } else {
                        dev = json_string_value(port);
                }

                if (dev != NULL && set_device(dev) == 0) {
                        r = 0;
                }
                json_unref(port);
        }

        return r;
}

static int get_configuration()
{
        if (get_device() != 0)
                return -1;

        return 0;
}

static int execute_script(int id)
{
        json_object_t script = json_array_get(scripts, id);
        const char *name = json_object_getstr(script, "name");
        if (name) {
                char uri[1024];
                r_info("execute_script: %s(%d)", name, id);
                rprintf(uri, sizeof(uri), "/execute?%s", name);
                //json_object_t r =
                client_get("script_engine", uri);
                // FIXME: script_engine_execute() returns no body?
        } else {
                r_warn("execute_script: script %d has no name", id);
        }
        return 0;
}

static int init_menus(json_object_t obj)
{
        if (json_isarray(obj)) {
                scripts = obj;
                for (int i = 0; i < json_array_length(scripts); i++) {
                        json_object_t script = json_array_get(scripts, i);
                        const char *name = json_object_getstr(script, "display_name");
                        if (name) {
                                r_debug("create_menus: menu[%d]: %s", i, name);
                                set_menu_item(name, i);
                        } else {
                                r_warn("create_menus: script %d has no display name", i);
                        }
                }
        } else {
                r_warn("create_menus: invalid scripts array");
        }
        return 0;
}

static int power_up()
{
        int r = -1;        
        if (send_command("1", state_buf) == 0) {
                const char* t = membuf_data(state_buf);
                if (strncmp(t, "OK", 2) == 0) {
                        r = 0;
                } else {
                        r_err("power_up: failed: %s", t);
                }
        } else {
                r_err("power_up: send_command failed");
        }
        return r;
}

static int download_scripts()
{
        scripts = json_null();
        
        int err = -1;
        for (int i = 0; i < 10; i++) {
                json_object_t obj = client_get("script_engine", "/scripts.json");
                if (!json_isnull(obj)) {
                        err = init_menus(obj);
                        if (err == 0)
                                break;
                } else {
                        r_err("Failed to download the scripts (%d/10)", i+1);
                }
                clock_sleep(1.0);
        }
        
        return err;
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

        if (download_scripts() != 0) {
                r_err("Failed to download the scripts: no menus");
        }
        
        if (power_up() != 0) {
                r_err("Power up failed");
                return -1;
        }
        
        
        /* if (display_message("Watchdog started")) { */
        /*         close_serial(); */
        /*         return -1;                 */
        /* } */

        return 0;
}

void control_panel_cleanup()
{
        close_serial();
        if (mutex)
                delete_mutex(mutex);
        if (state_buf)
                delete_membuf(state_buf);
        json_unref(scripts);
}

int control_panel_display(void *userdata,
                          messagelink_t *link,
                          json_object_t command,
                          membuf_t *message)
{
	r_info("Display");
        membuf_printf(message, "Not implemented yet");
        return -1;
}

void control_panel_onidle()
{
        json_object_t state = get_state();
        if (!json_isnull(state)) {
                analyse_state(state);
                json_unref(state);
        }
        clock_sleep(1.0);
}
