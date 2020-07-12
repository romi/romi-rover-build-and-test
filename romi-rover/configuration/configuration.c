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
#include "configuration.h"

static json_object_t config;

static void broadcast_log(void *userdata, const char* s)
{
        messagelink_t *get_messagelink_logger();
        messagelink_t *log = get_messagelink_logger();
        if (log)
                messagelink_send_str(log, s);
}

int configuration_init(int argc, char **argv)
{
        int err;
        char errmsg[256];
        const char *config_file = NULL;

        r_log_set_writer(broadcast_log, NULL);
        
        if (argc == 2) {
                config_file = argv[1];
                r_info("using configuration file passed on command line: %s",
                        config_file);
        } else if (app_get_config() != NULL) {
                config_file = app_get_config();
                r_info("using configuration file from -C option: %s",
                        config_file);
        } else {
                r_err("no configuration file given");
                return -1;
        }

        config = json_load(config_file, &err, errmsg, sizeof(errmsg));
        if (err != 0) {
                r_err("loading configuration file failed: %s", errmsg);
                return -1;
        }
        if (json_isnull(config)) {
                r_err("empty configuration?!");
                return -1;
        }
        return 0;
}

void configuration_cleanup()
{
        if (config)
                json_unref(config);
}

static json_object_t _eval_recursive(json_object_t obj, list_t *expr)
{
        if (expr == NULL)
                return obj;
        else if (!json_isobject(obj))
                return json_undefined();
        else {
                const char *key = list_get(expr, const char);
                list_t *next = list_next(expr);
                return _eval_recursive(json_object_get(obj, key), next);
        }
}

static json_object_t _eval(const char *path)
{
        list_t *expr = path_break(path);
        json_object_t r = _eval_recursive(config, expr);
        path_delete(expr);
        return r;
}

void configuration_get(void *data, request_t *request, response_t *response)
{
        const char *uri = request_uri(request);
        r_debug("Request: %s", uri);
        if (rstreq(uri, "/")) {
                response_json(response, config);
                return;
        } else {
                if (uri[0] == '/')
                        uri++;
                response_json(response, _eval(uri));
        }
}

