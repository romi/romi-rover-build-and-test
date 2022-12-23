/*
  bldc_featherwing

  Copyright (C) 2019-2020 Sony Computer Science Laboratories
  Author(s) Timothée Wintz, Peter Hanappe

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

static inline float normalizeAngle(float angle)
{
        while (angle < 0.0)
                angle += 1.0;
        while (angle >= 1.0)
                angle -= 1.0;
        return angle;
}

BLDC::BLDC(IArduino *_arduino,
           IEncoder* _encoder,
           IPwmGenerator *_generator,
           IOutputPin *_sleep,
           IOutputPin *_reset)
        
        : arduino(_arduino),
          encoder(_encoder),
          generator(_generator),
          sleepPin(_sleep),
          resetPin(_reset)
{
        targetPosition = 0.0f;
        setPower(0.0f);
        setPhase(0.0f);
        speed = 0.0f;
        lastSpeed = 0.0f;
        offsetAngleZero = 0.0;
        resetPin->set(1.0f);
        maxAcceleration = 20.0;
        kp = 10.0;
}

void BLDC::setOffsetAngleZero(float angle)
{
        offsetAngleZero = normalizeAngle(angle);
}

void BLDC::setTargetPosition(float position)
{
        targetPosition = normalizeAngle(position + offsetAngleZero);
}

void BLDC::setPower(float p)
{
        if (p >= 0.0f && p <= 1.0f) {
                if (power == 0.0f)
                        //generator->setPhase(encoder->getAngle());
                        generator->setPhase(phase); // FIXME
                else 
                        generator->setPhase(phase);
                power = p;
                generator->setAmplitude(p);
                generator->enable();
        } else {
                power = 0.0f;
                generator->setAmplitude(p);
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

void BLDC::setAngle(float angle)
{
        phase = angleToPhase(angle);
        generator->setPhase(phase);
}

void BLDC::setPhase(float value)
{
        phase = normalizeAngle(value);
        generator->setPhase(phase);
}

void BLDC::moveat(float rpm, float dt)
{
        float delta = 11.0f * rpm * dt / 60.0f;
        incrPhase(delta);
}

float BLDC::angleToPhase(float angle)
{
        angle = normalizeAngle(angle);
        float phase = 11.0f * angle;
        int pole = (int) phase;
        phase -= (float) pole;
        return phase;
}

bool BLDC::updatePosition(float dt)
{
        bool done = false;
        
        /* Compute the current position, taking into account the wrap
         * around and the fact its shorter to go from 10° to 350°
         * going clock-wise than counter clock-wise. */
        float angle = encoder->getAngle();
        float error = targetPosition - angle;
        while (error > 0.5f)
                error -= 1.0f;
        while (error <= -0.5f)
                error += 1.0f;

        // Serial.print(targetPosition, 5);
        // Serial.print(',');
        // Serial.print(angle, 5);
        // Serial.print(',');
        // Serial.print(error, 5);
        // Serial.print(',');
        
        // The speed is proportional to the error...
        float speed = kp * error;

        // Serial.print(speed, 5);
        // Serial.print(',');
        
        // ... but the acceleration should not surpass the maximum
        // acceleration.
        float acceleration = (speed - lastSpeed) / dt;
        if (acceleration < -maxAcceleration)
                acceleration = -maxAcceleration;
        else if (acceleration > maxAcceleration)
                acceleration = maxAcceleration;
        speed = lastSpeed + acceleration * dt;

        // Serial.print(speed, 5);
        // Serial.print(',');
        // Serial.println(speed * dt, 5);
        
        // when the given precision is reached (+-0.36°) then stop.
        if (error >= -0.001 && error <= 0.001) {
                //setPhase(angleToPhase(targetPosition));
                done = true;
        } else {
                float delta_phase = speed * dt;
                incrPhase(delta_phase);
                lastSpeed = speed;
        }
        
        return done;
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

bool BLDC::moveto(float angle)
{
        return tryMoveto(angle, 20.0f);
}

bool BLDC::tryMoveto(float angle, float timeOut)
{
        unsigned long startTime = arduino->micros();
        unsigned long lastTime = startTime;
        bool done = false;
        float duration = 0.0f;
        
        setTargetPosition(angle);

        while (!done && duration < timeOut) {
                unsigned long t = arduino->micros();
                float dt = (t - lastTime) / 1000000.0f;
                
                done = updatePosition(dt);
                        
                duration = (t - startTime) / 1000000.0f;
                lastTime = t;
                arduino->delay(2);
        }

        return done;
}

void BLDC::calibrate()
{
        // Position at angle zero
        setTargetPosition(0.0);
        for (int i = 0; i < 1000; i++) {
                update(0.003f);
                arduino->delay(3);
        }
        
        // Mve towards phase zero
        float delta = phase / 100.0f;
        for (int i = 1; i <= 100; i++) {
                setPhase((100 - i) * delta);
                arduino->delay(10);
        }

        arduino->delay(200);

        int step = 15;
        
        // Start calibration
        for (int pole = 0; pole < 11; pole++) {
                for (int iphase = 0; iphase < 360; iphase += step) {
                        setPhase(iphase / 360.0);
                        arduino->delay(1000);

                        float angle = encoder->getAngle();
                        // Serial.print(angle, 5);
                        // Serial.print(", ");
                        // Serial.println(pole + iphase / 360.0, 5);

                        for (int d = 0; d < step; d++) {
                                setPhase((iphase + d) / 360.0);
                                arduino->delay(5);
                        }
                }
        }
}
