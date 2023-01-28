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
#include "BLDC.h"
#include "PwmOut.h"
#include "PwmGenerator.h"
#include "DigitalOut.h"
#include "pins.h"
#include "IMU.h"
#include "M0Timer3.h"
#include "SpeedController.h"
#include "PositionController.h"
#include "PowerController.h"
#include "test.h"

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

// When building on Arduino IDE you need to modify variant.cpp file
// (from the adafruit M0 core) to free SERCOM0:
// 1. Pins PA04 and PA05 changed from PIO_ANALOG to PIO_SERCOM_ALT
//   (lines ~57,58)
// 2. Comment references to Serial1 at the end of file
// In platformio this is not needed (it is solved using a custom variant)

TwoWire myWire(&sercom0, P_IMU_SDA, P_IMU_SCL);
IMU imu(&myWire);

#define I2C_ADDRESS 11
enum I2cCommands {
	I2C_FOLLOW,
	I2C_ANGLE,
	I2C_ZERO_OFFSET,
	I2C_ZERO,
	I2C_MAX,
	I2C_MOTOR_SLEEP,
	I2C_MOTOR_POWER,
	I2C_MOTOR_POSITION,
	I2C_KP,
	I2C_MAX_ACCEL,
	I2C_RESET,

	I2C_COMMAND_COUNT
}; 

BLDC motor(&pwmGenerator, &sleepPin, &resetPin, 11);
PowerController power_controller(encoder, motor);
SpeedController speed_controller(motor, power_controller, 4.0); // acceleration in revolution/s²
PositionController position_controller(encoder, speed_controller);

unsigned long prev_time = 0;

void send_info(IRomiSerial *romiSerial, int16_t *args, const char *string_arg);
void handle_enable(IRomiSerial *romiSerial, int16_t *args, const char *string_arg);
void handle_moveto(IRomiSerial *romiSerial, int16_t *args, const char *string_arg);
void handle_moveat(IRomiSerial *romiSerial, int16_t *args, const char *string_arg);
void handle_set_position(IRomiSerial *romiSerial, int16_t *args, const char *string_arg);
void handle_get_position(IRomiSerial *romiSerial, int16_t *args, const char *string_arg);
void handle_set_power(IRomiSerial *romiSerial, int16_t *args, const char *string_arg);
//void handle_calibrate(IRomiSerial *romiSerial, int16_t *args, const char *string_arg);
void handle_get_roll(IRomiSerial *romiSerial, int16_t *args, const char *string_arg);
void run_tests(IRomiSerial *romiSerial, int16_t *args, const char *string_arg);

const static MessageHandler handlers[] = {
        { '?', 0, false, send_info },
        { 'E', 1, false, handle_enable },
        { 'M', 1, false, handle_moveto },
        { 'V', 1, false, handle_moveat },
        { 's', 0, false, handle_get_position },
        { 'P', 1, false, handle_set_power },
        { 'r', 0, false, handle_get_roll },
        // { 'T', 1, false, run_tests },
        { 'T', 0, false, run_tests },
};

ArduinoSerial serial(Serial);
RomiSerial romiSerial(serial, serial, handlers, sizeof(handlers) / sizeof(MessageHandler));

// In degrees (0/180, 0/-180)
float max_travel = 110;
float targetAngle = 0;
bool follow = false;

void setup()
{

        Serial.begin(115200);

        // Comunication with Raspberry Pi via I2C
	Wire.begin(I2C_ADDRESS);
	Wire.onReceive(receiveEvent);
	Wire.onRequest(requestEvent);

	// IMU
	if (!imu.begin()) {
		Serial.println("Failed to start IMU");
		// TODO Manage error
	}

        //float angleAtStartup = encoder.getAngle();
        //motor.setOffsetAngleZero(angleAtStartup);
        motor.set_power(0.0f);
        motor.wakeup();

        encoder.set_inverted(true);
        
        position_controller.init_start_position();
        M0Timer3::get().set_handler(&position_controller);
        // M0Timer3::get().set_handler(&speed_controller);
        // speed_controller.set_target_speed(0.1f);
        
        Serial.println("OK");
}

void loop()
{
        romiSerial.handle_input();
        
        // Serial.print("Loop: angle=");
        // Serial.println(encoder.get_angle(), 4);
        // delay(1000);
        
        // if (follow)
        //         motor.followIMU(&imu, targetAngle);
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
        //bool success = motor.moveto(value);
        position_controller.set_target_position(value);
        // if (success) {
                romiSerial->send_ok();  
        // } else {
        //         romiSerial->send_error(1, "Failed");  
        // }
}

void handle_moveat(IRomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        float rps = (float) args[0] / 1000.0;
        //speed_controller.set_target_speed(rps);
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

void handle_get_roll(IRomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        static char buffer[32];
        int value = (int) imu.getRoll(); 
        snprintf(buffer, sizeof(buffer), "[0,%d]", value);
        romiSerial->send(buffer);                
}

void receiveEvent(int howMany)
{
	byte command = I2C_COMMAND_COUNT;
	if (Wire.available()) command = Wire.read();

	switch(command)
	{
		case I2C_FOLLOW:
		{
			int data;
			if (i2c_receive(&data)) follow = data;
			break;
		}
		case I2C_ANGLE:
		{
                        Serial.println("set angle");
			// This should be in the range zero-max to zero+max
			int data;
			if (i2c_receive(&data)) {
                                Serial.print("value ");
                                Serial.println(data);
				float pos = data / 10000.0f;
				if (pos > max_travel)
                                        pos = max_travel;
				else if (pos < -max_travel)
                                        pos = -max_travel;
				targetAngle = fromRealAngle(pos);
			}

			break;
		}
		case I2C_ZERO_OFFSET:
		{
			// Zero position should be in the center of the movement range
			int data;
			if (i2c_receive(&data)) {
				float value = data / 10000.0f;
				value = fromRealAngle(value);
				imu.setZeroOffset(value);

				// Set motor offset
				float imuRaw = imu.getRoll(true);
				float diff = value - imuRaw;
				double angle = (double) encoder.get_angle();
				//motor.setOffsetAngleZero(angle + diff);
			}
			break;
		}
		case I2C_ZERO:
		{
                        Serial.println("set zero");
			// This allows to set the zero offset based on
			// the real position, so you can align the
			// camera manually ant issue this command.
			float raw = imu.getRoll(true);
			imu.setZeroOffset(raw);

			// The motor offset is also set to this angle
			// (based on encoder input)
			double angle = (double) encoder.get_angle();
			//motor.setOffsetAngleZero(angle);
			break;
		}
		case I2C_MAX:
		{
			int data;
			if (i2c_receive(&data)) max_travel = data / 10000.0f;
			break;
		}
		case I2C_MOTOR_POWER:
		{
                        Serial.println("set motor power");
			int data;
			if (i2c_receive(&data)) {
                                Serial.print("value ");
                                Serial.println(data);
				float value = data / 100.0f;
				if (value > 1.0f)
					value = 1.0f;
				else if (value < 0.0f)
					value = 0.0f;
				motor.set_power(value);
			}
			break;
		}
		case I2C_MOTOR_SLEEP:
		{
			int goToSleep;
			if (i2c_receive(&goToSleep)) {
				if (goToSleep)
                                        motor.sleep();
				else
                                        motor.wakeup();
			}
			break;
		}
		case I2C_MOTOR_POSITION:
		{
			int data;
			if (i2c_receive(&data)) {
				float pos = data / 10000.0f;
				if (pos > max_travel)
                                        pos = max_travel;
				else if (pos < -max_travel)
                                        pos = -max_travel;
                                pos /= 360.0f;
                                pos = normalize_angle(pos);
                                position_controller.set_target_position(pos);
                                
                                Serial.print("motor position ");
                                Serial.print(data);
                                Serial.print(", ");
                                Serial.println(pos);
			}
			break;
		}
		case I2C_KP:
		{
			int data;
			if (i2c_receive(&data)) {
				double newKp = data / 10000.0f;
				//motor.setKp(newKp);
			}
			break;
		}
		case I2C_MAX_ACCEL:
		{
			int data;
			if (i2c_receive(&data)) {
				double maxAccel = data / 10000.0f;
				//motor.setMaxAcceleration(maxAccel);
			}
			break;
		}
		case I2C_RESET:
		{
			motor.reset();
			Serial.println("reset");
			break;
		}
		default:
			break;
	}
}

void requestEvent()
{
	byte command = Wire.read();
        Serial.println("requestEvent");
	switch(command)
	{
		case I2C_ANGLE: 
		{
                        Serial.println("get angle");
			float roll = imu.getRoll();
			int angle = toRealAngle(roll) * 10000 ;
			i2c_send(angle);
			break;
		}
		case I2C_ZERO_OFFSET:
		{
			float norm_offset = imu.getZeroOffset();
			int offset = toRealAngle(norm_offset) * 10000;
			i2c_send(offset);
			break;
		}
		case I2C_MAX:
		{
			int value = max_travel * 10000;
			i2c_send(value);
			break;
		}
		case I2C_MOTOR_POWER: 
		{
			int mpow = motor.get_power() * 100;
			Serial.println(motor.get_power());
			Serial.println(mpow);
			i2c_send(mpow);
			break;
		}
		case I2C_MOTOR_POSITION:
		{
			float norm_pos = encoder.get_angle();
			//norm_pos = norm_pos - motor.getOffsetAngleZero();
			int pos = toRealAngle(norm_pos) * 10000;
			i2c_send(pos);
			break;
		}
		case I2C_KP:
		{
			int theKp = 0; //motor.getKp() * 10000;
			i2c_send(theKp);
			break;
		}
		case I2C_MAX_ACCEL:
		{
			int maxAccel = 0; //motor.getMaxAcceleration() * 10000;
			i2c_send(maxAccel);
			break;
		}
		default:
			break;
	}
}

void i2c_send(int data)
{
	// All i2c comunications are done with integrers (4 bytes)
	uint8_t b[4];
	uint16_t sum = 0;
	b[0] =  data & 0x000000ff;
	b[1] = (data & 0x0000ff00) >> 8;
	b[2] = (data & 0x00ff0000) >> 16;
	b[3] = (data & 0xff000000) >> 24;
	for (uint8_t i=0; i<4; i++) {
		sum += b[i];
		Wire.write(b[i]);
	}
	Wire.write(sum & 0x00ff);
}

bool i2c_receive(int *data)
{
	// All i2c comunications are done with integrers (4 bytes)
	uint8_t b[4];
	uint16_t sum = 0;
	for (uint8_t i=0; i<4; i++) {
		b[i] = Wire.read();
		sum += b[i];
	}
	byte check = Wire.read();

	if ((sum & 0x00ff) == check) {
		*data = b[0] | b[1] << 8 | b[2] << 16 | b[3] << 24;
		return true;
	} else {
                Serial.println("i2c_receive: checksum failed");
		return false;
	}
}

float toRealAngle(float normalizedAngle)
{
	// Returns position in degrees from 0 to 180 (clockwise) and 0
	// to -180 (anticlockwise)
	while (normalizedAngle > 0.5f)
                normalizedAngle -= 1.0f;
	while (normalizedAngle < -0.5f)
                normalizedAngle += 1.0f;
	return normalizedAngle * 360.0f;
}

float fromRealAngle(float angle)
{
	// Return normalized angle between 0 and 1.0
	while (angle < 0.0f)
                angle += 360.0f;
	while (angle > 360.0f)
                angle += 360.0f;
	return angle / 360.0f;
}

void run_tests(IRomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        run_tests_all();
        // switch (args[0]) {
        // case 0:
        //         run_tests_all();
        //         break;
                
        // case 1:
        //         run_tests_fixed();
        //         break;
        // }

        romiSerial->send_ok();  
}
