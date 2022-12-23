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

  You should have received a copy opositionf the GNU General Public License
  along with this program.  If not, see
  <http://www.gnu.org/licenses/>.

 */
#ifndef __BLDC_H
#define __BLDC_H

#include "IArduino.h"
#include "IEncoder.h"
#include "IOutputPin.h"
#include "IPwmGenerator.h"

#define SINE_TABLE_SIZE 384

class BLDC
{
protected:
        IArduino *arduino;
        IEncoder* encoder;
        IPwmGenerator *generator;
        IOutputPin *sleepPin;
        IOutputPin *resetPin;
        float targetPosition;
        float offsetAngleZero;
        float speed;
        float lastSpeed;
        float maxAcceleration;
        float kp;
        float power;
        float phase;
        
        void moveat(float rpm, float dt);
        bool updatePosition(float dt);
                
        //void setPhase(float value);
        
        /* void incrPhase(float delta) { */
        /*         setPhase(phase + delta); */
        /* } */
        
        float angleToPhase(float angle);

        bool tryMoveto(float angle, float timeOut);

public:
        BLDC(IArduino *_arduino,
             IEncoder* _encoder,
             IPwmGenerator *_generator,
             IOutputPin *_sleep,
             IOutputPin *_reset);
        
        virtual ~BLDC() {}

        void setPower(float p);
        
        /** Set the target position of the motor. The position is a
            normalized angle: a value of 1 is equal to an absolute
            angle of 360°. Values larger than 1 or smaller than zero
            will be mapped to the [0,1] range.  */
        void setTargetPosition(float pos);

        /** Set the offset that corresponds to a 0° angle on your
         * device.  */
        void setOffsetAngleZero(float pos);        

        void wake();
        void sleep();

        void update(float dt);

        void calibrate();

        bool moveto(float angle);
        
        void setAngle(float value);
        
        void setPhase(float value);
        
        void incrPhase(float delta) {
                setPhase(phase + delta);
        }
};

#endif // __BLDC_H
