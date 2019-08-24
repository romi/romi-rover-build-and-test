#ifndef _ROMI_PICAMERA_H_
#define _ROMI_PICAMERA_H_

#include <rcom.h>

#ifdef __cplusplus
extern "C" {
#endif

streamer_t *get_streamer_camera();

int picamera_init(int argc, char **argv);
void picamera_cleanup();
void picamera_broadcast();
int picamera_still(void *data, request_t *request);  

#ifdef __cplusplus
}
#endif

#endif // _ROMI_PICAMERA_H_
