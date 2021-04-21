#include <ArduinoSerial.h>
#include <RomiSerial.h>

void send_info(RomiSerial *romiSerial, int16_t *args, const char *string_arg);
void handle_grab(RomiSerial *romiSerial, int16_t *args, const char *string_arg);

const static MessageHandler handlers[] = {
        { 'G', 0, false, handle_grab },
        { '?', 0, false, send_info },
};

ArduinoSerial serial(Serial1);
RomiSerial romiSerial(serial, serial, handlers, sizeof(handlers) / sizeof(MessageHandler));

void setup()
{
        Serial1.begin(115200);
        while (!Serial1)
                ;
        
        Serial.begin(115200);
        while (!Serial)
                ;
}

void loop()
{
        romiSerial.handle_input();
        delay(1);
}

void send_info(RomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        Serial.println("send_info");
        romiSerial->send("[\"test-serial\",\"0.1\"]"); 
}

void handle_grab(RomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        static int counter = 0;
        Serial.println("handle_grab");
        char buffer[64];
        snprintf(buffer, sizeof(buffer), "[0,%d]", counter++);
        romiSerial->send(buffer); 
}
