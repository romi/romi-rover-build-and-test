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
#include "BLDC.h"
#include <math.h>

float normalizeAngle(float angle)
{
        while (angle < 0.0f)
                angle += 1.0f;
        while (angle >= 1.0f)
                angle -= 1.0f;
        return angle;
}

double normalizeAngle(double angle)
{
        while (angle < 0.0)
                angle += 1.0;
        while (angle >= 1.0)
                angle -= 1.0;
        return angle;
}

BLDC::BLDC(IEncoder* _encoder,
           IPwmGenerator *_generator,
           IOutputPin *_sleep,
           IOutputPin *_reset)
        
        : encoder(_encoder),
          generator(_generator),
          sleepPin(_sleep),
          resetPin(_reset),
          pid(&currentPosition, &outputSignal, &targetPosition, 0.0, 0.0, 0.0, DIRECT)
{
        targetPosition = 0.0f;
        setPower(0.0f);
        setPhase(0.0f);
        speed = 0.0f;
        offsetAngleZero = 0.0;
        initializeSineTable();
        resetPin->set(1.0f);

        setTargetPosition(0.5f); // DEBUG
        
        pid.SetMode(AUTOMATIC);
        pid.SetSampleTime(10);
        pid.SetOutputLimits(-1.0, 1.0);
}

void BLDC::setPIDValues(float kp, float ki, float kd)
{
        pid.SetTunings(kp, ki, kd);
}

void BLDC::getPIDValues(float &kp, float &ki, float &kd)
{
        kp = pid.GetKp();
        ki = pid.GetKi();
        kd = pid.GetKd();
}

void BLDC::setOffsetAngleZero(double angle)
{
        offsetAngleZero = normalizeAngle(angle);
}

void BLDC::setTargetPosition(double position)
{
        targetPosition = normalizeAngle(position + offsetAngleZero);
}

void BLDC::setPower(float p)
{
        if (p >= 0.0f && p <= 1.0f) {
                power = p;
                setPhase(phase);
                generator->enable();
        } else {
                power = 0.0f;
                generator->set(0.0f, 0.0f, 0.0f);
                generator->disable();
        }
}

void BLDC::wake()
{
        sleepPin->set(1.0f);
}

void BLDC::sleep()
{
        sleepPin->set(0.0f);
}

void BLDC::setPhase(double value)
{
        phase = value;

        int i1 = phase * SINE_TABLE_SIZE;
        while (i1 < 0)
                i1 += SINE_TABLE_SIZE;
        while (i1 >= SINE_TABLE_SIZE)
                i1 -= SINE_TABLE_SIZE;
        
        int i2 = i1 + SINE_TABLE_SIZE / 3;
        while (i2 < 0)
                i2 += SINE_TABLE_SIZE;
        while (i2 >= SINE_TABLE_SIZE)
                i2 -= SINE_TABLE_SIZE;
        
        int i3 = i1 + 2 * SINE_TABLE_SIZE / 3;
        while (i3 < 0)
                i3 += SINE_TABLE_SIZE;
        while (i3 >= SINE_TABLE_SIZE)
                i3 -= SINE_TABLE_SIZE;
        
        generator->set(0.5 + power * 0.5 * sineTable[i1],
                       0.5 + power * 0.5 * sineTable[i2],
                       0.5 + power * 0.5 * sineTable[i3]);
}

void BLDC::initializeSineTable()
{
        for (int i = 0; i < SINE_TABLE_SIZE; i++)
                sineTable[i] = sinf((float) (i * 2.0f * M_PI) / SINE_TABLE_SIZE);
}

void BLDC::moveat(float rpm, float dt)
{
        float delta = 11.0f * rpm * dt / 60.0f;
        //float delta = 0.0005f;
        incrPhase(delta);
        Serial.print(phase);
        Serial.print(", ");
        Serial.println(encoder->getAngle());
}

void BLDC::updatePosition(float dt)
{
        /* The PID controller doesn't understand anything about angles
         * that wrap around and gets pretty confused when the angle
         * jumps from 0.01 to 0.99. Therefore, we'll compute the
         * current position manually and take into account the wrap
         * around and the fact its shorter to go from 10° to 350°
         * going clock-wise than counter clock-wise. */
        float angle = encoder->getAngle();
        float error = targetPosition - angle;
        while (error > 0.5f)
                error -= 1.0f;
        while (error < -0.5f)
                error += 1.0f;
        currentPosition = targetPosition - error;

        if (pid.Compute()) {
                speed = outputSignal * 10.0f;
                
                if (0) {
                        Serial.print("T:");
                        Serial.print(targetPosition);
                        Serial.print(", C:");
                        Serial.print(currentPosition);
                        Serial.print(", P:");
                        Serial.print(phase);
                        Serial.print(", O:");
                        Serial.print(outputSignal);
                        Serial.print(", v:");
                        Serial.print(speed);
                        Serial.print(", dt:");
                        Serial.print(dt);
                        Serial.print(", D:");
                        Serial.println(speed * dt);
                }
        }

        // delta = speed * dt = outputSignal * 1.0f * dt;
        // max(delta) = max(outputSignal) * max(dt) * 0.1f 
        // max(delta) = 1 * 0.005 * 0.1 = 0.0005 * 360° / 11 pole pairs
        // = 0.16° 
                
        float delta_phase = speed * dt;
        incrPhase(delta_phase);
}

void BLDC::update(float dt)
{
        if (power > 0.0f) {
                if (1)
                        updatePosition(dt);
                else
                        moveat(15.0f, dt);
        }        
}
