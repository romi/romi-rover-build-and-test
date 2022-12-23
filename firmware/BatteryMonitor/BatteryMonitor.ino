/*************************************************************************************
 ReadVeDirectFrameHandler

 Uses VeDirectFrameHandler library

 This example and library tested with NodeMCU 1.0 using Software Serial.
 If using with a platform containing 2 harware UART's, use those, not SoftwareSerial.
 Tested with Victron BMV712.

 VEDirect Device:
   pin 1 - gnd
   pin 2 - RX
   pin 3 - TX
   pin 4 - power

 History:
   2020.05.05 - 0.3 - initial release

**************************************************************************************/

#include "Arduino.h"
#include <SoftwareSerial.h>
#include "VeDirectFrameHandler.h"
#include <ArduinoSerial.h>
#include <RomiSerial.h>

using namespace romiserial;
VeDirectFrameHandler myve;

// SoftwareSerial
#define rxPin 2                            // RX using Software Serial so we can use the hardware UART to check the ouput
#define txPin 3                            // TX Not used
SoftwareSerial veSerial(rxPin, txPin);

void send_info(IRomiSerial *romiSerial, int16_t *args, const char *string_arg);
void handle_level_request(IRomiSerial *romiSerial, int16_t *args, const char *string_arg);
const static MessageHandler handlers[] = {
        { 'B', 0, false, handle_level_request },
        { '?', 0, false, send_info },
};

ArduinoSerial serial(Serial);
RomiSerial romiSerial(serial, serial, handlers, sizeof(handlers) / sizeof(MessageHandler));

String reply;

void setup() {
	Serial.begin(115200);                   // output serial port
    veSerial.begin(19200);                  // input serial port (VE device)
    veSerial.flush();
    // Length From EnvelopeParser
    reply.reserve(MAX_MESSAGE_LENGTH);
}

void loop() {    
    ReadVEData();
    romiSerial.handle_input();
    delay(1);
}

void ReadVEData() {
    while ( veSerial.available() ) {
        myve.rxData(veSerial.read());
    }
    yield();
}

void send_info(IRomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
    romiSerial->send("[0,\"BatteryMonitor\",\"0.1\",\"" __DATE__ " " __TIME__ "\"]");
}

void handle_level_request(IRomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
    reply = "[0";
    
    for ( int i = 0; i < myve.veEnd; i++ ) {
      reply += ",";
      reply += String(myve.veValue[i]); 
    }
    reply += "]";
    romiSerial->send(reply.c_str());
}
