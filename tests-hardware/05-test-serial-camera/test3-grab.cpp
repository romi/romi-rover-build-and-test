#include <stdlib.h>
#include <memory>
#include <iostream>
#include <r.h>
#include <USBCamera.h>
#include <ImageIO.h>


int main()
{
        romi::USBCamera camera("/dev/video0", 1920, 1080);
        romi::Image image;

        clock_sleep(2.0);
        
        bool success = camera.grab(image);
        if (success) {
                r_info("Grabbed the image");
                romi::bytevector jpeg;
                double start = clock_time();

                success = romi::ImageIO::store_jpg(image, "test.jpg");

                if (success) {
                        r_info("Compressed the image: %f", clock_time() - start);
                } else {
                        r_err("Compression failed");
                }
        } else {
                r_err("Grab failed");
        }
        return 0;
}
