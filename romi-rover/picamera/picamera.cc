#include <stdio.h>
#include <raspicam/raspicam.h>
#include <romi.h>
#include "picamera.h"

static int width = 640;
static int height = 480;
static unsigned char *data = NULL;
static membuf_t *jpegbuf = NULL;
static raspicam::RaspiCam camera;

typedef struct _image_request_t {
        response_t *response;
        condition_t *condition;
        double timestamp;
} image_request_t;

#define REQUEST_BUFFER_SIZE 8

static mutex_t *request_mutex = NULL;
static image_request_t *request_buffer[REQUEST_BUFFER_SIZE];
static int request_buffer_index = 0;

static void broadcast_log(void *userdata, const char* s)
{
        messagelink_t *get_messagelink_logger();
        messagelink_t *log = get_messagelink_logger();
        if (log)
                messagelink_send_str(log, s);
}

image_request_t *new_image_request(response_t *response)
{
        image_request_t *r = r_new(image_request_t);
        if (r == NULL)
                return NULL;
        r->timestamp = 0.0;
        r->response = response;
        r->condition = new_condition();
        if (r->condition == NULL) {
                r_delete(r);
                return NULL;
        }
        return r;
}

void delete_image_request(image_request_t *r)
{
        if (r) {
                if (r->condition) 
                        delete_condition(r->condition);
                r_delete(r);
        }
}

int add_image_request(image_request_t *r)
{
        int err = 0;
        
        mutex_lock(request_mutex);
        if (request_buffer_index < REQUEST_BUFFER_SIZE) 
                request_buffer[request_buffer_index++] = r;
        else
                err = -1;
        mutex_unlock(request_mutex);

        return err;
}

int wait_image_request(image_request_t *r)
{
        membuf_t *buf = response_body(r->response);
        membuf_lock(buf);
        while (r->timestamp == 0)
                condition_wait(r->condition, membuf_mutex(buf));
        membuf_unlock(buf);
        return 0;
}

int picamera_init(int argc, char **argv)
{
        r_log_set_writer(broadcast_log, NULL);
        camera.setFormat(raspicam::RASPICAM_FORMAT_BGR);
        camera.setWidth(width);
        camera.setHeight(height);
        camera.setBrightness(50);
        camera.setSharpness(0);
        camera.setContrast(0);
        camera.setSaturation(0);
        camera.setShutterSpeed(0);
        camera.setISO(400);
        camera.setVideoStabilization(true);
        camera.setExposureCompensation(0);
        //camera.setExposure(xxx);
        //camera.setAWB(xxx);
        //camera.setAWB_RB(xxx);
        
        request_mutex = new_mutex();
        if (request_mutex == NULL)
                return -1;

        data = (unsigned char*) r_alloc(width * height * 3);
        jpegbuf = new_membuf();
        if (data == NULL || jpegbuf == NULL)
                return -1;
  
        if (!camera.open()) {
                r_err("Failed to ope the camera");
                return -1;
        }
        return 0;
}

void picamera_cleanup()
{
        camera.release();
        if (data)
                r_free(data);
        if (jpegbuf)
                delete_membuf(jpegbuf);
        if (request_mutex)
                delete_mutex(request_mutex);
}

void picamera_broadcast()
{
        streamer_t *streamer = get_streamer_camera();
        int image_requested = 0;
        int stream_requested = 0;

        // Always grab images to keep the camera alive
        camera.grab();

        double timestamp = clock_time();
        
        mutex_lock(request_mutex);
        image_requested = (request_buffer_index > 0);
        mutex_unlock(request_mutex);
        
        streamer_lock_clients(streamer);
        stream_requested = streamer_has_clients(streamer);
        streamer_unlock_clients(streamer);

        if (image_requested || stream_requested) {
                camera.retrieve(data);
                convert_to_jpeg(data, width, height, 90, jpegbuf);
        } else {
                clock_sleep(1);
        }
        
        if (stream_requested) {
                //r_debug("timestamp %f", timestamp);
                //r_debug("image length %d", membuf_len(jpegbuf));
                streamer_send_multipart(streamer, 
                                        membuf_data(jpegbuf),
                                        membuf_len(jpegbuf),
                                        "image/jpeg", timestamp);
                //r_debug("sleep");

                // FIXME: Reduce the framerate
                clock_sleep(1);
        }
        
        if (image_requested) {
                mutex_lock(request_mutex);
                for (int i = 0; i < request_buffer_index; i++) {
                        image_request_t *r = request_buffer[i];
                        membuf_t *buf = response_body(r->response);
                        
                        membuf_lock(buf);
                        membuf_clear(buf);
                        membuf_append(buf, membuf_data(jpegbuf), membuf_len(jpegbuf));
                        r->timestamp = timestamp;
                        condition_signal(r->condition);
                        membuf_unlock(buf);
                }
                request_buffer_index = 0;
                mutex_unlock(request_mutex);
        }
}

void picamera_still(void *data, request_t *request, response_t *response)
{
        image_request_t *r = new_image_request(response);
        if (r == NULL) {
                response_set_status(response, HTTP_Status_Internal_Server_Error);
                return;
        }
        
        int err = add_image_request(r);
        if (err != 0) {
                response_set_status(response, HTTP_Status_Too_Many_Requests);
                goto cleanup;
        }

        err = wait_image_request(r);
        if (r == NULL) {
                response_set_status(response, HTTP_Status_Internal_Server_Error);
                goto cleanup;
        }

cleanup:
        delete_image_request(r);
}


