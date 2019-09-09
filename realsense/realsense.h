#ifndef _ROMI_REALSENSE_H_
#define _ROMI_REALSENSE_H_

#ifdef __cplusplus
extern "C" {
#endif

int realsense_init(int argc, char **argv);
void realsense_cleanup();
void realsense_broadcast();
void realsense_still(void *data, request_t *request, response_t *response);

streamer_t *get_streamer_camera();
streamer_t *get_streamer_depthsensor();

#ifdef __cplusplus
}
#endif

#endif // _ROMI_REALSENSE_H_
