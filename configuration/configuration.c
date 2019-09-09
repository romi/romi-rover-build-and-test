#include "configuration.h"

static json_object_t config;

int configuration_init(int argc, char **argv)
{
        int err;
        char errmsg[256];
        const char *config_file = NULL;
        
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

void configuration_get(void *data, request_t *request, response_t *response)
{
        const char *uri = request_uri(request);
        
        if (rstreq(uri, "/")) {
                response_json(response, config);
                return;
        } else {
                if (uri[0] == '/')
                        uri++;
                response_json(response, json_object_get(config, uri));
        }
}

