#include <stdio.h>
#include "fake_camera.h"

static membuf_t *rgbbuf = NULL;

streamer_t *get_streamer_camera();

static void broadcast_log(void *userdata, const char* s)
{
        messagelink_t *get_messagelink_logger();
        messagelink_t *log = get_messagelink_logger();
        if (log)
                messagelink_send_str(log, s);
}

int fake_camera_init(int argc, char **argv)
{
        json_object_t path = json_null();
        const char *filename = NULL;

        r_log_set_writer(broadcast_log, NULL);
        
        if (argc >= 2) {
                filename = argv[1];
        } else {
                path = client_get("configuration", "fake_camera.image");
                if (!json_isstring(path)) {
                        r_err("The value of fake_camera.image is not a string");
                        json_unref(path);
                        return -1;
                }
                filename = json_string_value(path);
        }
        
        rgbbuf = new_membuf();
        if (rgbbuf == NULL) {
                json_unref(path);
                return -1;
        }

        r_err("Loading file: '%s'", filename);

        char buffer[1024];
        FILE *fp = fopen(filename, "rb");
        int err = 0;
        
        while (!feof(fp) && !ferror(fp)) {
                int n = fread(buffer, 1, sizeof(buffer), fp);
                membuf_append(rgbbuf, buffer, n);
        }

        if (ferror(fp)) {
                err = -1;
                json_unref(path);
                r_err("An error occured while loading '%s'", argv[1]);
        }
        
        fclose(fp);

        json_unref(path);
        
        return err;
}

void fake_camera_cleanup()
{
        if (rgbbuf)
                delete_membuf(rgbbuf);
}

void fake_camera_broadcast()
{
        streamer_t *streamer = get_streamer_camera();
        if (streamer_has_clients(streamer)) {
                double timestamp = clock_time();
                streamer_send_multipart(streamer, 
                                        membuf_data(rgbbuf),
                                        membuf_len(rgbbuf),
                                        "image/jpeg", timestamp);
        } 
        clock_sleep(0.5);
}

void fake_camera_still(void *data, request_t *request, response_t *response)
{
        response_append(response, membuf_data(rgbbuf), membuf_len(rgbbuf));        
}
