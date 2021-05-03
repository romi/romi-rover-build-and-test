/*
  bldc_featherwing

  Copyright (C) 2019-2020 Sony Computer Science Laboratories
  Author(s) Peter Hanappe

  bldc_featherwing is Arduino firmware to control a brushless motor.

  bldc_featherwing is free software: you can redistribute it and/or
  modify it under the terms of the GNU General Public License as
  published by the Free Software Foundation, either version 3 of the
  License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see
  <http://www.gnu.org/licenses/>.

 */
#include "ArduinoImpl.h"
#include "PwmEncoder.h"
#include "BLDC.h"
#include "Parser.h"
#include "PwmOut.h"
#include "PwmGenerator.h"
#include "DigitalOut.h"
#include "pins.h"
#include <RomiSerial.h>
#include <ArduinoSerial.h>

ArduinoImpl arduino;
PwmEncoder encoder(&arduino, P_ENC, 11, 959);
Parser parser("XP", "?sC");

PwmOut pwm1(&arduino, P_IN1);
PwmOut pwm2(&arduino, P_IN2);
PwmOut pwm3(&arduino, P_IN3);
DigitalOut enable1(&arduino, P_EN1);
DigitalOut enable2(&arduino, P_EN2);
DigitalOut enable3(&arduino, P_EN3);

PwmGenerator pwmGenerator(&pwm1, &pwm2, &pwm3,
                          &enable1, &enable2, &enable3);

DigitalOut sleepPin(&arduino, P_SLEEP);
DigitalOut resetPin(&arduino, P_RESET);


BLDC motor(&arduino, &encoder, &pwmGenerator, &sleepPin, &resetPin);

unsigned long prev_time = 0;

void send_info(RomiSerial *romiSerial, int16_t *args, const char *string_arg);
void handle_moveto(RomiSerial *romiSerial, int16_t *args, const char *string_arg);
void handle_set_position(RomiSerial *romiSerial, int16_t *args, const char *string_arg);
void handle_get_position(RomiSerial *romiSerial, int16_t *args, const char *string_arg);
void handle_set_power(RomiSerial *romiSerial, int16_t *args, const char *string_arg);
void handle_calibrate(RomiSerial *romiSerial, int16_t *args, const char *string_arg);

const static MessageHandler handlers[] = {
        { '?', 0, false, send_info },
        { 'M', 1, false, handle_moveto },
        { 's', 0, false, handle_get_position },
        { 'P', 1, false, handle_set_power },
        { 'C', 1, false, handle_calibrate },
};

// ArduinoSerial serial(Serial);
// RomiSerial romiSerial(serial, serial, handlers, sizeof(handlers) / sizeof(MessageHandler));

ArduinoSerial serial1(Serial1);
RomiSerial romiSerial1(serial1, serial1, handlers, sizeof(handlers) / sizeof(MessageHandler));

void setup()
{
        // Serial.begin(115200);
        // while (!Serial)
        //         ;
        Serial1.begin(115200);
        while (!Serial1)
                ;

        // Serial.println("OK");
        
        motor.wake();
        motor.setPower(0.5f);
}

// static int counter = 0; 

void loop()
{
        // romiSerial.handle_input();
        romiSerial1.handle_input();
        // Serial1.println(counter++);
        // while (Serial1.available()) {
        //         char c = Serial1.read();
        //         Serial1.write(c);
        // }
        delay(1000);
}

void send_info(RomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        romiSerial->send("[0,\"BLDCController\",\"0.1\","
                         "\"" __DATE__ " " __TIME__ "\"]"); 
}

void handle_moveto(RomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        float value = (float) args[0] / 3600.0f;
        bool success = motor.moveto(value);
        if (success) {
                romiSerial->send_ok();  
        } else {
                romiSerial->send_error(1, "Failed");  
        }
}

void handle_get_position(RomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        static char buffer[32];
        int value = (int) (3600.0f * encoder.getAngle()); 
        snprintf(buffer, sizeof(buffer), "[0,%d]", value);
        romiSerial->send(buffer);                
}

void handle_set_power(RomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        float value = (float) args[0] / 100.0f;
        if (value > 1.0f)
                value = 1.0f;
        else if (value < 0.0f)
                value = 0.0f;
        motor.setPower(value);
        romiSerial->send_ok();  
}

void handle_calibrate(RomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        romiSerial->send_ok();  
        motor.calibrate();
}
