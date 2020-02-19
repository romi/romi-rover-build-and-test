#include <stdio.h>
#include <raspicam/raspicam.h>
#include <romi.h>
#include "picamera.h"

static int width = 640;
static int height = 480;
static unsigned char *data = NULL;
static membuf_t *jpegbuf = NULL;
static raspicam::RaspiCam camera;

static void broadcast_log(void *userdata, const char* s)
{
        messagelink_t *get_messagelink_logger();
        messagelink_t *log = get_messagelink_logger();
        if (log)
                messagelink_send_str(log, s);
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
}

void picamera_broadcast()
{
        streamer_t *streamer = get_streamer_camera();
        if (streamer_has_clients(streamer)) {
                r_debug("grab");
                camera.grab();
                camera.retrieve(data);
                r_debug("convert");
                convert_to_jpeg(data, width, height, 90, jpegbuf);
                double timestamp = clock_time();
                r_debug("send");
                streamer_send_multipart(streamer, 
                                        membuf_data(jpegbuf),
                                        membuf_len(jpegbuf),
                                        "image/jpeg", timestamp);
                r_debug("sleep");
                clock_sleep(1);
        } 
}

int picamera_still(void *data, request_t *request)
{
        return 0;
}


