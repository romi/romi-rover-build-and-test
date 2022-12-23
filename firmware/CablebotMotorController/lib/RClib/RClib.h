#ifndef __RCLIB_H
#define __RCLIB_H

#include <Arduino.h>

class RemoteControl
{
private:
	static uint8_t _speedPin = ;
	static uint8_t _directionPin = ;
	volatile unsigned long _timerStartSpeed;
	volatile unsigned long _speed;
	volatile unsigned long _timerStartDirection;
	volatile unsigned long _direction;

public:

	/* RemoteControl(uint8_t speedPin, uint8_t directionPin) { */
	/* 	_speedPin = speedPin; */
	/* 	_directionPin = directionPin; */
	/* } */

	void setup();
	static void handleSpeedUp();
	static void handleSpeedDown();
	static void handleDirectionUp();
	static void handleDirectionDown();

};

#endif
