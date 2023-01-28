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

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see
  <http://www.gnu.org/licenses/>.

 */
#include "IMU.h"

IMU::IMU (TwoWire *_theWire, 
     byte _deviceAdress) 
{
	theWire = _theWire;
	deviceAddress = _deviceAdress;
}

bool IMU::begin()
{
	// Find device I2C address (if not manually specified)
	if (deviceAddress == 0) {
		for (uint8_t src = 0; src < ACC_SOURCE_NUM; src++){
			deviceAddress = sourceAddress[src];
			theWire->beginTransmission(deviceAddress);
			byte error = theWire->endTransmission();
			if (error == 0) break;
		}
	}

	if (ism330dhcx.begin_I2C(deviceAddress, theWire)) {
		mySource = SRC_ISM330DHCX;

		// Put any model-specific initialization code here

		return true;
	}

	if (lsm6dsox.begin_I2C(deviceAddress, theWire)) {
		mySource = SRC_LSM6DSOX; 

		// Put any model-specific initialization code here
		
		return true;
	}


	// TODO put setup IMU code here
	// Stop gyro and start accel
	
	/* // Adafruit default init as example */
	/* // Enable accelerometer with 104 Hz data rate, 4G */
	/* setAccelDataRate(LSM6DS_RATE_104_HZ); */
	/* setAccelRange(LSM6DS_ACCEL_RANGE_4_G); */

	/* // Enable gyro with 104 Hz data rate, 2000 dps */
	/* setGyroDataRate(LSM6DS_RATE_104_HZ); */
	/* setGyroRange(LSM6DS_GYRO_RANGE_2000_DPS); */

	/* delay(10); */

	return true;
}

void IMU::update()
{
	ism330dhcx.getEvent(&accel, &gyro, &temp);
}

float IMU::getRoll(bool raw)
{
	update();
	double accel_yg = accel.acceleration.y / GRAVITY_EARTH;
	accel_yg = min(accel_yg, 0.9999999);
	accel_yg = max(accel_yg, -0.9999999);	

	double accel_zg = accel.acceleration.z / GRAVITY_EARTH;
	accel_zg = min(accel_zg, 0.9999999);
	accel_zg = max(accel_zg, -0.9999999);	

	double inclination_y = (acos(accel_yg) * DEGREES_PER_RADIAN);
	if (accel_zg > 0)
                inclination_y = 360 - inclination_y;
	inclination_y =
                inclination_y / 360;

	if (raw)
                inclination_y = normalize_angle(inclination_y);
	else
                inclination_y = normalize_angle(inclination_y + zero_offset);

	return (float) inclination_y;
}
