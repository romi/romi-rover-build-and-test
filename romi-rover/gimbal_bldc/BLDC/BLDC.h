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

  You should have received a copy opositionf the GNU General Public License
  along with this program.  If not, see
  <http://www.gnu.org/licenses/>.

 */
#ifndef __BLDC_H
#define __BLDC_H

#include <PID_v1.h>
#include "IArduino.h"
#include "IEncoder.h"
#include "IOutputPin.h"
#include "IPwmGenerator.h"

#define SINE_TABLE_SIZE 384

float normalizeAngle(float angle);
double normalizeAngle(double angle);

class BLDC
{
protected:
        IEncoder* encoder;
        IPwmGenerator *generator;
        IOutputPin *sleepPin;
        IOutputPin *resetPin;
        double targetPosition;
        double currentPosition;
        double outputSignal;
        double offsetAngleZero;
        double speed;
        PID pid;

        float power;
        double phase;
        
        float sineTable[SINE_TABLE_SIZE];
        
        void moveat(float rpm, float dt);
        void updatePosition(float dt);
        
        
        void initializeSineTable();

        void setPhase(double value);
        
        void incrPhase(double delta) {
                setPhase(phase + delta);
        }

        
public:
        BLDC(IEncoder* _encoder,
             IPwmGenerator *_generator,
             IOutputPin *_sleep,
             IOutputPin *_reset);
        
        virtual ~BLDC() {}

        void setPower(float p);
        
        /** Set the target position of the motor. The position is a
            normalized angle: a value of 1 is equal to an absolute
            angle of 360°. Values larger than 1 or smaller than zero
            will be mapped to the [0,1] range.  */
        void setTargetPosition(double pos);

        /** Set the offset that corresponds to a 0° angle on your
         * device.  */
        void setOffsetAngleZero(double pos);        

        void wake();
        void sleep();
        void setPIDValues(float kp, float ki, float kd);
        void getPIDValues(float &kp, float &ki, float &kd);

        void update(float dt);
};

#endif // __BLDC_H
