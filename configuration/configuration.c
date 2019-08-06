#include "configuration.h"

static json_object_t config;

int configuration_init(int argc, char **argv)
{
        int err;
        char errmsg[256];
        const char *config_file = NULL;
        
        if (argc == 2) {
                config_file = argv[1];
                log_info("using configuration file passed on command line: %s",
                        config_file);
        } else if (app_get_config() != NULL) {
                config_file = app_get_config();
                log_info("using configuration file from -C option: %s",
                        config_file);
        } else {
                log_err("no configuration file given");
                return -1;
        }

        config = json_load(config_file, &err, errmsg, sizeof(errmsg));
        if (err != 0) {
                log_err("loading configuration file failed: %s", errmsg);
                return -1;
        }
        if (json_isnull(config)) {
                log_err("empty configuration?!");
                return -1;
        }
        return 0;
}

void configuration_cleanup()
{
        if (config)
                json_unref(config);
}

int configuration_get(void *data, request_t *request)
{
        const char *uri = request_uri(request);

        if (rstreq(uri, "/")) {
                request_reply_json(request, config);
                request_set_status(request, 200);
                return 0;
        }

        if (uri[0] == '/')
                uri++;
        
        log_debug("get '%s'", uri);

        json_object_t value = json_object_get(config, uri);
        request_reply_json(request, value);
        request_set_status(request, 200);
        return 0;
}

