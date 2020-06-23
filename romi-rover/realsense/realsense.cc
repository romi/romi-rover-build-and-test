#include <librealsense2/rs.hpp>
#include <rcom.h>
#include <romi.h>
#include "convert.h"
#include "realsense.h"

using namespace rs2;

static pipeline pipe;
static mutex_t *mutex = NULL;
static membuf_t *rgbbuf = NULL;
static membuf_t *pngbuf = NULL;
static int rgb_width = 1920;
static int rgb_height = 1080;
static int depth_width = 640;
static int depth_height = 480;
static int error_count = 0;
static bool want_image = false;
static unsigned long frame_count = 0;

static void broadcast_log(void *userdata, const char* s)
{
        messagelink_t *get_messagelink_logger();
        messagelink_t *log = get_messagelink_logger();
        if (log)
                messagelink_send_str(log, s);
}

int realsense_init(int argc, char **argv)
{
        r_log_set_writer(broadcast_log, NULL);
        
        try {
                config cfg;
                cfg.enable_stream(RS2_STREAM_COLOR,
                                  rgb_width, rgb_height,
                                  RS2_FORMAT_RGB8, 15);
                cfg.enable_stream(RS2_STREAM_DEPTH,
                                  depth_width, depth_height,
                                  RS2_FORMAT_Z16, 15);
                pipe.start(cfg);
                
        } catch (const rs2::error& e) {
                r_err("realsense_init: an error occurred: %s", e.what());
                return -1;
        }

        mutex = new_mutex();
        if (mutex == NULL)
                return -1;
        
        rgbbuf = new_membuf();
        if (rgbbuf == NULL || membuf_assure(rgbbuf, 5 * 1024 * 1024) != 0)
                return -1;
        
        pngbuf = new_membuf();
        if (pngbuf == NULL || membuf_assure(pngbuf, 5 * 1024 * 1024) != 0) 
                return -1;

        return 0;
}

void realsense_cleanup()
{
        if (pngbuf)
                delete_membuf(pngbuf);
        if (rgbbuf)
                delete_membuf(rgbbuf);
        if (mutex)
                delete_mutex(mutex);
}

void realsense_broadcast()
{
        frameset frames;
        double timestamp = clock_time();
        bool convert_rgb = false;
        bool convert_depth = false;
        
        mutex_lock(mutex);
        if (want_image)
                convert_rgb = true;
        mutex_unlock(mutex);

        if (streamer_has_clients(get_streamer_camera()))
                convert_rgb = true;
        if ((frame_count % 5) == 0
            && streamer_has_clients(get_streamer_depthsensor()))
                convert_depth = true;

        if (!convert_rgb && !convert_depth) {
                membuf_lock(rgbbuf);
                membuf_clear(rgbbuf);
                membuf_unlock(rgbbuf);
                clock_sleep(0.200);
                return;
        }
        
        try {
                frames = pipe.wait_for_frames();
                        
        } catch (const camera_disconnected_error& e) {
                r_err("Camera was disconnected! Quitting."); // TODO: use health monitor
                app_set_quit();
                
        } catch (const recoverable_error& e) {
                r_warn("Operation failed. Trying again. (error: %s)", e.what());
                return;
                
        } catch (const error& e) {
                r_err("An error occurred: %s", e.what());
                error_count++;
                if (error_count > 20) {
                        r_err("Too many errors. Quitting");
                        app_set_quit();
                }
                return;
        }
        
        membuf_lock(rgbbuf);
        membuf_clear(rgbbuf);
        membuf_unlock(rgbbuf);
                
        if (convert_rgb) {
                frame color_frame = frames.get_color_frame();

                r_debug("convert_rgb");

                membuf_lock(rgbbuf);
                convert_to_jpeg((uint8_t*)color_frame.get_data(), rgb_width, rgb_height, 85, rgbbuf);
                membuf_unlock(rgbbuf);
                
                streamer_send_multipart(get_streamer_camera(), 
                                        membuf_data(rgbbuf),
                                        membuf_len(rgbbuf),
                                        "image/jpeg", timestamp);
        }
        
        if (convert_depth) {
                frame depth_frame = frames.get_depth_frame();
                        
                r_debug("convert_depth");
                
                membuf_lock(pngbuf);
                membuf_clear(pngbuf);
                convert_to_png_16bit_grayscale((uint16_t*)depth_frame.get_data(),
                                               depth_width, depth_height, pngbuf);
                membuf_unlock(pngbuf);
                
                streamer_send_multipart(get_streamer_depthsensor(),
                                        membuf_data(pngbuf),
                                        membuf_len(pngbuf),
                                        "image/png", timestamp);
        }

        frame_count++;
}

void realsense_still(void *data, request_t *request, response_t *response)
{
        bool done = false;
        while (!done) {
                
                r_debug("send_still_image: want_image=true");
                
                mutex_lock(mutex);
                want_image = true;
                mutex_unlock(mutex);

                membuf_lock(rgbbuf);
                r_debug("realsense_still: image len=%d", membuf_len(rgbbuf));
                if (membuf_len(rgbbuf) > 0) {
                        r_debug("send_still_image: have_image=true");
                        response_append(response, membuf_data(rgbbuf), membuf_len(rgbbuf));
                        done = true;
                }
                membuf_unlock(rgbbuf);

                if (!done) clock_sleep(0.1);
        }

        mutex_lock(mutex);
        r_debug("send_still_image: want_image=false");
        want_image = false;
        mutex_unlock(mutex);
}
