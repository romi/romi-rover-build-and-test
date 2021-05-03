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
#include "gimbal_bldc.h"

static char *device = NULL;
static serial_t *serial = NULL;
static mutex_t *mutex = NULL;
static int initialized = 0;
static membuf_t *message_buffer = NULL;

static int open_serial(const char *dev)
{
        r_info("Trying to open the serial connection on %s.", dev);
	
        mutex_lock(mutex);        
        serial = new_serial(dev, 115200, 1);
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

static int get_gimbal_configuration()
{
        return (device == NULL)? -1 : 0;
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

int gimbal_init()
{
        if (initialized)
                return 0;
        
        if (get_gimbal_configuration() != 0)
                return -1;

        if (open_serial(device) != 0)
                return -1;
        
        initialized = 1;
        return 0;
}

int gimbal_bldc_init(int argc, char **argv)
{
        message_buffer = new_membuf();
        mutex = new_mutex();
        
        if (argc >= 2) {
                r_debug("using serial device '%s'", argv[1]);
                if (set_device(argv[1]) != 0)
                        return -1;
        }
        
        for (int i = 0; i < 10; i++) {
                if (gimbal_init() == 0)
                        return 0;
                r_err("gimbal_init failed: attempt %d/10", i);
                clock_sleep(0.2);
        }

        r_err("failed to initialize the gimbal");
        return -1;
}

void gimbal_bldc_cleanup()
{
        close_serial();
        if (mutex)
                delete_mutex(mutex);
        if (message_buffer)
                delete_membuf(message_buffer);
}

int gimbal_onmoveto(void *userdata,
                    messagelink_t *link,
                    json_object_t command,
                    membuf_t *message)
{
        double angle = json_object_getnum(command, "angle");
        if (isnan(angle)) {
                membuf_printf(message, "Missing value for angle");
                return -1;
        }
        
        char cmd[64];
        rprintf(cmd, sizeof(cmd), "X%d", (int) angle);
        
        return send_command(cmd, message_buffer);
}
