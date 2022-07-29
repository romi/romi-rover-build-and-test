#include <stdlib.h>
#include <memory>
#include <iostream>
#include <Logger.h>
#include <camera/USBCamera.h>
#include <cv/ImageIO.h>
#include <ClockAccessor.h>

int main()
{
        romi::USBCamera camera("/dev/video0", 1920, 1080);
        romi::Image image;
        auto clock = rpp::ClockAccessor::GetInstance();

        clock->sleep(2.0);
        
        bool success = camera.grab(image);
        if (success) {
                r_info("Grabbed the image");
                romi::bytevector jpeg;
                double start = clock->time();

                success = romi::ImageIO::store_jpg(image, "test.jpg");

                if (success) {
                        r_info("Compressed the image: %f", clock->time() - start);
                } else {
                        r_err("Compression failed");
                }
        } else {
                r_err("Grab failed");
        }
        return 0;
}
