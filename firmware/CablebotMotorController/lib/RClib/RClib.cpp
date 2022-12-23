#include "RClib.h"

void RemoteControl::setup()
{
	_timerStartSpeed = 0;
        pinMode(_speedPin, INPUT);
        attachInterrupt(digitalPinToInterrupt(_speedPin), handleSpeedUp, RISING);
        
	_timerStartDirection = 0;
        pinMode(_directionPin, INPUT);
        attachInterrupt(digitalPinToInterrupt(_directionPin), handleDirectionUp, RISING);
}


static void RemoteControl::handleSpeedUp() 
{
        _timerStartSpeed = micros();
        attachInterrupt(digitalPinToInterrupt(_speedPin), handleSpeedDown, FALLING);
} 

void RemoteControl::handleSpeedDown()
{
        if (_timerStartSpeed != 0) { 
                // Record the pulse time
                _speed = micros() - _timerStartSpeed;
                // Restart the timer
                _timerStartSpeed = 0;
        }
        attachInterrupt(digitalPinToInterrupt(_speedPin), handleSpeedUp, RISING);
}

static void RemoteControl::handleDirectionUp() 
{
        _timerStartDirection = micros();
        attachInterrupt(digitalPinToInterrupt(_directionPin), handleDirectionDown, FALLING);
} 

void RemoteControl::handleDirectionDown() 
{
        if (_timerStartDirection != 0) { 
                // Record the pulse time
                _direction = micros() - _timerStartDirection;
                // Restart the timer
                _timerStartDirection = 0;
        }
        attachInterrupt(digitalPinToInterrupt(_directionPin), handleDirectionUp, RISING);
}
