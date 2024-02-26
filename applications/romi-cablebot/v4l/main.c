#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "camera_v4l.h"

const char *default_device = "/dev/video0";

int main(int argc, char **argv)
{
        const char *device = default_device;
        int width = 640;
        int height = 480;
        int jpeg_quality = 90;
        char *output = "test.jpg";
        
        camera_t* camera = new_camera(default_device, width, height, jpeg_quality);
        if (camera == NULL) {
                fprintf(stderr, "Failed to open camera %s\n", device);
                exit(1);
        }

        for (int i = 0; i < 10; i++) {
                camera_capture(camera);
                usleep(300000);
        }

        ssize_t size = (ssize_t) camera_getimagesize(camera);
        unsigned char* data = camera_getimagebuffer(camera);
        
        int fp = open(output, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG);
        if (fp >= 0) {
                ssize_t written = 0;

                while (written < size) {
                        ssize_t n = write(fp, data + written, size - written);
                        if (n < 0) {
                                fprintf(stderr, "Failed to write\n");
                                exit(1);
                        }
                        written += n;
                }

                close(fp);
                
        } else {
                fprintf(stderr, "Failed to open file %s\n", output);
                exit(1);
        }

        delete_camera(camera);
}


