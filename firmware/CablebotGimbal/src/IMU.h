/*
  bldc_featherwing

  Copyright (C) 2019-2020 Sony Computer Science Laboratories
  Author(s) Peter Hanappe, Victor Barberan

  bldc_featherwing is Arduino firmware to control a brushless motor.

  bldc_featherwing is free software: you can redistribute it and/or
  modify it under the terms of the GNU General Public License as
  published by the Free Software Foundation, either version 3 of the
  License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy opositionf the GNU General Public License
  along with this program.  If not, see
  <http://www.gnu.org/licenses/>.

 */
#ifndef __IMU_H
#define __IMU_H


#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_ISM330DHCX.h>
#include <Adafruit_LSM6DSOX.h>
#include "BLDC.h"

#define GRAVITY_EARTH 9.807
#define DEGREES_PER_RADIAN 180.0 / 3.141592653589793238463

class IMU
{
	private:
		TwoWire *theWire;

		enum accSources {
			SRC_ISM330DHCX,
			SRC_LSM6DSOX,

			ACC_SOURCE_NUM
		};
		
		Adafruit_ISM330DHCX ism330dhcx;
		Adafruit_LSM6DSOX lsm6dsox;

		byte sourceAddress[ACC_SOURCE_NUM] = {
			0x6A, 	// ISM330DHCX
			0x6A, 	// LSM6DSOX
		};

		float zero_offset = 0;
		float max_travel = 0.5f;

	public:
		IMU (TwoWire *_theWire,
		     byte _deviceAdress=0);

		accSources mySource;
		byte deviceAddress;

		// accel.acceleration.x, y and z
		sensors_event_t accel;
		// gyro.gyro.x, y and z
		sensors_event_t gyro;
		// temp.temperature
		sensors_event_t temp;

		bool begin();
		void setZeroOffset(float wichOffset) {
			zero_offset = wichOffset;
		};
		float getZeroOffset() {
			return zero_offset;
		};

		// Update sensor readings
		void update();

		// Returns inclination of camera (0 is verticaly aligned)
		float getRoll(bool raw = false);
};


#endif // __IMU_H
