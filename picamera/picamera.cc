#include <stdio.h>
#include <raspicam/raspicam.h>
#include <romi.h>
#include "picamera.h"
#include "convert.h"

static int width = 640;
static int height = 480;
static unsigned char *data = NULL;
static membuf_t *jpegbuf = NULL;
static raspicam::RaspiCam camera;

int picamera_init(int argc, char **argv)
{
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

  data = (unsigned char*) mem_alloc(width * height * 3);
  jpegbuf = new_membuf();
  if (data == NULL || jpegbuf == NULL)
    return -1;
  
  if (!camera.open()) {
    log_err("Failed to ope the camera");
    return -1;
  }
  return 0;
}

void picamera_cleanup()
{
  camera.release();
        if (data)
                mem_free(data);
        if (jpegbuf)
                delete_membuf(jpegbuf);
}

void picamera_broadcast()
{
        streamer_t *streamer = get_streamer_camera();
        if (streamer_has_clients(streamer)) {
	  log_debug("grab");
	  camera.grab();
	  camera.retrieve(data);
	  log_debug("convert");
	  convert_to_jpeg(data, width, height, 90, jpegbuf);
	  double timestamp = clock_time();
	  log_debug("send");
	  streamer_send_multipart(streamer, 
                                        membuf_data(jpegbuf),
                                        membuf_len(jpegbuf),
                                        "image/jpeg", timestamp);
	  log_debug("sleep");
	  clock_sleep(1);
        } 
}

int picamera_still(void *data, request_t *request)
{
        return 0;
}


