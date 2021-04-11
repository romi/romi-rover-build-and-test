#include <stdlib.h>
#include <memory>
#include <iostream>
#include <thread>
#include <chrono>
#include <r.h>
#include <RomiSerial.h>
#include <RSerial.h>
#include <USBCamera.h>
#include <ImageIO.h>

using namespace std::chrono_literals;

static void send_info(RomiSerial *romiSerial, int16_t *args, const char *string_arg);
static void handle_grab(RomiSerial *romiSerial, int16_t *args, const char *string_arg);

const static MessageHandler handlers[] = {
        { '?', 0, false, send_info },
        { 'G', 1, false, handle_grab }
};

romi::USBCamera camera_("/dev/video0", 1920, 1080);

int main()
{
        RSerial serial("/dev/serial0", 115200, 0);
        RomiSerial romi_serial(serial, serial, handlers,
                               sizeof(handlers) / sizeof(MessageHandler));
        
        while (true) {
                romi_serial.handle_input();
                std::this_thread::sleep_for(10ms);
        }

        return 0;
}

static void send_info(RomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        (void) args;
        (void) string_arg;
        romiSerial->send("[0,\"test-camera\",\"0.1\"]");
}

static void handle_grab(RomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        (void) string_arg;
        (void) args;
        romi::Image image;

        bool success = camera_.grab(image);
        if (success) {
                romi::bytevector jpeg;
                success = romi::ImageIO::store_jpg(image, jpeg);

                if (success) {
                        char buffer[128];
                        snprintf(buffer, 128, "[0,%d]", (int) jpeg.size());
                        romiSerial->send(buffer);
                        r_info("Compressed the image");
                } else {
                        romiSerial->send_error(1, "Compression failed");
                }
        } else {
                romiSerial->send_error(1, "Grab failed");
        }
}
