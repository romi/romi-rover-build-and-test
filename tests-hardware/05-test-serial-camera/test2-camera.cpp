#include <stdlib.h>
#include <r.h>
#include <RomiSerial.h>
#include <RSerial.h>
#include <memory>
#include <iostream>

static void send_info(RomiSerial *romiSerial, int16_t *args, const char *string_arg);
static void handle_grab(RomiSerial *romiSerial, int16_t *args, const char *string_arg);

const static MessageHandler handlers[] = {
        { '?', 0, false, send_info },
        { 'G', 1, false, handle_grab }
};

int main()
{
        RSerial serial("/dev/serial0", 115200, 0);
        RomiSerial romi_serial(serial, serial, handlers,
                               sizeof(handlers) / sizeof(MessageHandler));
        
        while (true) {
                romi_serial.handle_input();
                clock_sleep(0.010);
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
        char buffer[128];
        snprintf(buffer, 128, "[0,%d]", (int) args[0]+1);
        romiSerial->send(buffer);
}
