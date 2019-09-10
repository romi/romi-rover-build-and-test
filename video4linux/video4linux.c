#include "camera_v4l.h"
#include "video4linux.h"

static mutex_t *mutex = NULL;
static const char *device = "/dev/video0";
static camera_t* camera = NULL;
static int want_image = 0;
static membuf_t *rgbbuf = NULL;
static int width = 1920;
static int height = 1080;

streamer_t *get_streamer_camera();

int video4linux_init(int argc, char **argv)
{
        if (argc == 2)
                device = argv[1];
        
        mutex = new_mutex();
        if (mutex == NULL)
                return -1;
        
        rgbbuf = new_membuf();
        if (rgbbuf == NULL || membuf_assure(rgbbuf, 5 * 1024 * 1024) != 0)
                return -1;
        
        camera = new_camera(device, IO_METHOD_MMAP, width, height, 90);
        if (camera == NULL) {
                r_err("Failed to open the camera");
                return -1;
        }
        return 0;
}

void video4linux_cleanup()
{
        if (camera)
                delete_camera(camera);
        if (mutex)
                delete_mutex(mutex);
        if (rgbbuf)
                delete_membuf(rgbbuf);
}

void video4linux_broadcast()
{
        streamer_t *streamer = get_streamer_camera();
        if (want_image || streamer_has_clients(streamer)) {
                int error = camera_capture(camera);
                if (error) {
                        r_err("Failed to grab the image");
                        clock_sleep(0.04);
                        return;
                }

                double timestamp = clock_time();
        
                streamer_send_multipart(streamer, 
                                        camera_getimagebuffer(camera),
                                        camera_getimagesize(camera),
                                        "image/jpeg", timestamp);

                membuf_lock(rgbbuf);
                membuf_clear(rgbbuf);
                membuf_append(rgbbuf,
                              camera_getimagebuffer(camera),
                              camera_getimagesize(camera));
                membuf_unlock(rgbbuf);
                
        } else {
                clock_sleep(0.1);
        }
}

void video4linux_still(void *data, request_t *request, response_t *response)
{
        int done = 0;
        while (!done) {
                
                r_debug("send_still_image: want_image=true");
                
                mutex_lock(mutex);
                want_image = 1;
                mutex_unlock(mutex);

                membuf_lock(rgbbuf);
                r_debug("realsense_still: image len=%d", membuf_len(rgbbuf));
                if (membuf_len(rgbbuf) > 0) {
                        r_debug("send_still_image: have_image=true");
                        response_append(response, membuf_data(rgbbuf), membuf_len(rgbbuf));
                        membuf_clear(rgbbuf); // consume the image
                        done = 1;
                }
                membuf_unlock(rgbbuf);

                if (!done) clock_sleep(0.1);
        }

        mutex_lock(mutex);
        r_debug("send_still_image: want_image=false");
        want_image = 0;
        mutex_unlock(mutex);
}
