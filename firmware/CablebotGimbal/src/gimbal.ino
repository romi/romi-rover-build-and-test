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
#include <RomiSerial.h>
#include <ArduinoSerial.h>
#include "ArduinoImpl.h"
#include "PwmEncoder.h"
#include "PwmOut.h"
#include "DigitalOut.h"
#include "PwmGenerator.h"
#include "M0Timer3.h"
#include "SpeedController.h"
#include "PositionController.h"
#include "PowerController.h"
#include "BLDC.h"
#include "pins.h"

using namespace romiserial;

ArduinoImpl arduino;
PwmEncoder encoder(&arduino, P_ENC, 11, 915);

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

BLDC motor(&pwmGenerator, &sleepPin, &resetPin, 11);
PowerController power_controller(encoder, motor);
SpeedController speed_controller(motor, power_controller, 4.0); // acceleration in revolution/s²
PositionController position_controller(encoder, speed_controller);

void send_info(IRomiSerial *romiSerial, int16_t *args, const char *string_arg);
void handle_enable(IRomiSerial *romiSerial, int16_t *args, const char *string_arg);
void handle_moveto(IRomiSerial *romiSerial, int16_t *args, const char *string_arg);
void handle_set_position(IRomiSerial *romiSerial, int16_t *args, const char *string_arg);
void handle_get_position(IRomiSerial *romiSerial, int16_t *args, const char *string_arg);
void handle_set_power(IRomiSerial *romiSerial, int16_t *args, const char *string_arg);

const static MessageHandler handlers[] = {
        { '?', 0, false, send_info },
        { 'E', 1, false, handle_enable },
        { 'M', 1, false, handle_moveto },
        { 's', 0, false, handle_get_position },
        { 'P', 1, false, handle_set_power },
};

ArduinoSerial serial(Serial);
RomiSerial romiSerial(serial, serial, handlers, sizeof(handlers) / sizeof(MessageHandler));

void setup()
{
        Serial.begin(115200);

        Serial.println("init@1");

        motor.set_power(0.0f);
        motor.wakeup();

        Serial.println("init@2");

        encoder.set_inverted(true);

        Serial.println("init@3");
        
        position_controller.init_start_position();
        M0Timer3::get().set_handler(&position_controller);

        Serial.println("init: OK");
}

void loop()
{
        romiSerial.handle_input();
        
        // Serial.print("value=");
        // Serial.print(encoder.get_value());
        // Serial.print(", value0=");
        // Serial.print(encoder.value0_);
        // Serial.print(", count=");
        // Serial.println(encoder.count_);
        
        delay(10);
}

void send_info(IRomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        romiSerial->send("[0,\"CablebotGimbal\",\"0.1\","
                         "\"" __DATE__ " " __TIME__ "\"]"); 
}

void handle_enable(IRomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        if (args[0] == 0) {
                motor.sleep();
                romiSerial->send_ok();  
        } else {
                motor.wakeup();
                romiSerial->send_ok();  
        }
}

void handle_moveto(IRomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        float value = (float) args[0] / 3600.0f;
        position_controller.set_target_position(value);
        romiSerial->send_ok();  
}

void handle_get_position(IRomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        static char buffer[32];
        int value = (int) (3600.0f * encoder.get_angle()); 
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
        // Serial.print("power ");
        // Serial.println(value);
        motor.set_power(value);
        romiSerial->send_ok();  
}
