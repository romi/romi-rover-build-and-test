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
#include "PwmOut.h"
#include "PwmGenerator.h"
#include "DigitalOut.h"
#include "pins.h"
#include "M0Timer3.h"
#include "Controller.h"
#include <RomiSerial.h>
#include <ArduinoSerial.h>
#include <math.h>

ArduinoImpl arduino;
PwmEncoder encoder(&arduino, P_ENC, 11, 959);

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
Controller controller(motor, 4.0); // acceleration in revolution/s²

unsigned long prev_time = 0;

using namespace romiserial;

void send_info(IRomiSerial *romiSerial, int16_t *args, const char *string_arg);
void handle_get_angle(IRomiSerial *romiSerial, int16_t *args, const char *string_arg);
void handle_set_angle(IRomiSerial *romiSerial, int16_t *args, const char *string_arg);
void handle_moveat(IRomiSerial *romiSerial, int16_t *args, const char *string_arg);
void handle_moveto(IRomiSerial *romiSerial, int16_t *args, const char *string_arg);
void handle_set_power(IRomiSerial *romiSerial, int16_t *args, const char *string_arg);
void handle_calibrate(IRomiSerial *romiSerial, int16_t *args, const char *string_arg);

const static MessageHandler handlers[] = {
        { '?', 0, false, send_info },
        { 's', 0, false, handle_get_angle },
        { 'A', 1, false, handle_set_angle },
        { 'V', 1, false, handle_moveat },
        { 'M', 1, false, handle_moveto },
        { 'P', 1, false, handle_set_power },
        { 'C', 1, false, handle_calibrate },
};

#define USE_SERIAL    0
#define USE_SERIAL_1  1

#if USE_SERIAL
ArduinoSerial serial(Serial); // TODO
RomiSerial romiSerial(serial, serial, handlers, sizeof(handlers) / sizeof(MessageHandler)); // TODO
#endif

#if USE_SERIAL_1
ArduinoSerial serial1(Serial1); // TODO
RomiSerial romiSerial1(serial1, serial1, handlers, sizeof(handlers) / sizeof(MessageHandler)); // TODO
#endif


void setup()
{
#if USE_SERIAL
         Serial.begin(115200); // TODO
         while (!Serial) // TODO
                 ; // TODO
#endif
        
#if USE_SERIAL_1
        Serial1.begin(115200); // TODO
        while (!Serial1) // TODO
                ; // TODO
#endif

        // Serial.println("OK");
        
        motor.wake();
        motor.setPower(0.5f);

        M0Timer3::get().set_handler(&controller);
}

void loop()
{
#if USE_SERIAL
        romiSerial.handle_input(); // TODO
#endif
        
#if USE_SERIAL_1
        romiSerial1.handle_input(); // TODO
#endif
        
        // Serial.println(encoder.getValue());
        // delay(1000);

        //Serial.println(controller.t_);
        //delay(1000);
}

void send_info(IRomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        romiSerial->send("[0,\"BLDCController\",\"0.1\","
                         "\"" __DATE__ " " __TIME__ "\"]"); 
}

void handle_moveto(IRomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        float value = (float) args[0] / 3600.0f;
        bool success = motor.moveto(value);
        if (success) {
                romiSerial->send_ok();  
        } else {
                romiSerial->send_error(1, "Failed");  
        }
}

void handle_moveat(IRomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        float rps = (float) args[0] / 1000.0;
        controller.set_target_speed(rps);
        romiSerial->send_ok();  
}

void handle_set_angle(IRomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        float angle = (float) args[0] / 3600.0;
        motor.setAngle(angle);
        romiSerial->send_ok();  
}

void handle_get_angle(IRomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        static char buffer[32];
        int value = (int) (3600.0f * encoder.getAngle()); 
        snprintf(buffer, sizeof(buffer), "[0,%d]", value);
        romiSerial->send(buffer);                
}

void handle_set_power(IRomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        float value = (float) args[0] / 100.0f;
        if (value > 1.0f)
                value = 1.0f;
        else if (value < 0.0f)
                value = 0.0f;
        motor.setPower(value);
        romiSerial->send_ok();  
}

void handle_calibrate(IRomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        romiSerial->send_ok();  
        motor.calibrate();
}
