/*
  Romi motor controller for brushed mortors

  Copyright (C) 2018-2019 Sony Computer Science Laboratories
  Author(s) Peter Hanappe

  LettuceThink Motor Controller is a firmware for Arduino. It reads
  two RC signals for speed and direction, two encoders for two wheel,
  and generates two PWM motor signals for a motor driver.

  The LettuceThink Motor Controller is free software: you can
  redistribute it and/or modify it under the terms of the GNU Lesser
  General Public License as published by the Free Software Foundation,
  either version 3 of the License, or (at your option) any later
  version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see
  <http://www.gnu.org/licenses/>.

 */
/*
   Uses code from:
   https://forum.arduino.cc/index.php?topic=390305.0
   and
   http://www.camelsoftware.com/2015/12/25/reading-pwm-signals-from-an-rc-receiver-with-arduino/
   --> FIXME: check licensing!!!
*/
#include <Servo.h>
#include <PID_v1.h>
#include "Parser.h"

/////////////////////////////////////////////////////////////////////////

#if defined(ARDUINO_AVR_UNO)
// Using Raspberry Pi + Arduino Uno
#define HAS_RC 0

#elif defined(ARDUINO_AVR_LEONARDO)
// Using Udoo X86 II
#define HAS_RC 1

#elif defined(ARDUINO_ARC32_TOOLS)
// Using Udoo X86 I
#define HAS_RC 1
#else
#define HAS_RC 0
#endif


// #if defined(UDOO)
#define LeftEncoderIsReversed
#define RightEncoderIsReversed
// #endif

// #if defined(ROMI_ROVER_DEV)
// #define LeftEncoderIsReversed
// #undef RightEncoderIsReversed
// #endif

// #if defined(ROMI_ROVER_LYNX)
// #define LeftEncoderIsReversed
// #undef RightEncoderIsReversed
// #endif

/////////////////////////////////////////////////////////////////////////

enum { STATE_UNINITIALIZED, STATE_DISABLED, STATE_ENABLED, STATE_HOMING, STATE_ERROR };
enum { INPUT_RC, INPUT_SERIAL };
enum { CONTROL_PID, CONTROL_DIRECT };
enum { ERROR_NONE = 0, ERROR_STOPSWITCH, ERROR_UNINITIALIZED, ERROR_DISABLED, ERROR_ALARM };

const char *msgOK = "OK";
const char *errBadState = "ERR bad state";
const char *errValue01 = "ERR expected 0|1";
const char *errMissingValues = "ERR missing values";
const char *errNotAnOption = "ERR not an option";

unsigned char state = STATE_UNINITIALIZED;
unsigned char error = ERROR_NONE;
unsigned char inputMode = INPUT_SERIAL;
unsigned char controlMode = CONTROL_DIRECT;
unsigned char withHoming = 0;
unsigned char withRC = 0;

/////////////////////////////////////////////////////////////////////////
#define leftEncoderPinA 2
#define leftEncoderPinB 4
#define rightEncoderPinA 3
#define rightEncoderPinB 5
#if HAS_RC
#define rcSpeedPin 7
#define rcDirectionPin 8
#endif
#define pinLeftMotor 9
#define pinRightMotor 10
#define pinFrontStopSwitch 11
#define pinBackStopSwitch 12

/////////////////////////////////////////////////////////////////////////
// The parser for handling user input.

Parser parser("MDEIKPR", "?!eXHS");

/////////////////////////////////////////////////////////////////////////
// The H-bridge expects a PWM signal similar to those produced by RC
// receiver or accepted by servo motors. For ease, we use the standard
// Servo library to produce these signals.

Servo leftMotor;
Servo rightMotor;
// The maximum amplitude of the servo signal.
float maxSignal = 20.0f;

/////////////////////////////////////////////////////////////////////////
double lastLeftInput = 0;
double lastRightInput = 0;

/////////////////////////////////////////////////////////////////////////
// Code to read the PWM signals from the RC receiver.
//
// From http://www.camelsoftware.com/2015/12/25/reading-pwm-signals-from-an-rc-receiver-with-arduino/

// Micros when the pin goes HIGH
volatile unsigned long timerstartSpeed;
volatile unsigned long timerstartDirection;

// The difference between timerstart and micros() is the length of
// time that the pin was HIGH - the PWM pulse length.
volatile int rcSpeed = 0;
volatile int rcDirection = 0;
int lastRcSpeed = 0;
int lastRcDirection = 0;

// This is the time that the last interrupt occurred. You can use this
// to determine if your receiver has a signal or not.
volatile int lastInterruptTimeSpeed; 
volatile int lastInterruptTimeDirection; 

/////////////////////////////////////////////////////////////////////////
// The PID controllers that manage the motor speeds.


// The target, the input speed, and the output signal are measured
// between [-1,1].
double leftTarget = 0, rightTarget = 0; // The target speeds
double leftInput = 0, rightInput = 0; // The measured speeds
double leftOutput = 0, rightOutput = 0; // The values computed by the PID controllers

// The number of encoder steps per revolution. This value must be
// initialized using the 'I' command. The value below is just a
// safeguard.
float stepsPerRevolution = 10000.0f;

// The maximum speed in revolutions per second
float maxSpeed = 1.0f; 

// The absolute speed values in revolutions per second.
float leftAbsoluteSpeed = 0;
float rightAbsoluteSpeed = 0; 

// The speed values that were last sent to setOutputSignal() [-1,1].
float leftSpeed = 0;
float rightSpeed = 0; 

// The values send to the Servo controllers [0,180]
unsigned char leftSignal = 0;
unsigned char rightSignal = 0; 

// The PID controllers. The Kp, Ki, and Kd values must be set with
// using the 'K' command.
PID leftPID(&leftInput, &leftOutput, &leftTarget, 0, 0, 0, DIRECT);
PID rightPID(&rightInput, &rightOutput, &rightTarget, 0, 0, 0, DIRECT);

/////////////////////////////////////////////////////////////////////////
// The quadrature encoders

// Left encoder
// #define LeftEncoderIsReversed
volatile long leftEncoderTicks = 0;
long leftPrevEncoderTicks = 0;

// Right encoder
// #define RightEncoderIsReversed
volatile long rightEncoderTicks = 0;
long rightPrevEncoderTicks = 0;

#if defined(ARDUINO_ARC32_TOOLS)
// The code to read the pin values directly in the registers. This is
// faster than using the digitalRead() function.  These macros are
// specific to Arduino 101. For other boards, you will have to include
// the appropriate translations.
//
// From https://forum.arduino.cc/index.php?topic=390305.0
uint32_t ioReg1 = SS_GPIO_8B1_BASE_ADDR + SS_GPIO_EXT_PORTA;
uint32_t ioReg2 = SOC_GPIO_BASE_ADDR + SOC_GPIO_EXT_PORTA;
uint32_t ioRegA = SS_GPIO_8B0_BASE_ADDR + SS_GPIO_EXT_PORTA;
#define readPin4  (MMIO_REG_VAL(ioReg2) & 0b10000000000000000000)
#define readPin5 (__builtin_arc_lr(ioReg1) & 0b00001000)
#define readLeftEncoderPinB() readPin4
#define readRightEncoderPinB() readPin5

#else
#define readLeftEncoderPinB() digitalRead(leftEncoderPinB)
#define readRightEncoderPinB() digitalRead(rightEncoderPinB)
#endif

/////////////////////////////////////////////////////////////////////////

int debug = 1;
#define debug_print(__x)  if (debug) Serial.print(__x)
#define debug_println(__x)  if (debug) Serial.println(__x)
#define debug_print_float(__x,__y)  if (debug) Serial.print(__x,__y)
#define debug_print_name_value(__s, __v) if (debug) { Serial.print(__s); Serial.print(" "); Serial.println(__v); }

/////////////////////////////////////////////////////////////////////////

void setup_rc()
{
#if HAS_RC
        pinMode(rcSpeedPin, INPUT);
        attachInterrupt(digitalPinToInterrupt(rcSpeedPin),
                        handleRCSpeedUp, RISING);
        
        pinMode(rcDirectionPin, INPUT);
        attachInterrupt(digitalPinToInterrupt(rcDirectionPin),
                        handleRCDirectionUp, RISING);
#endif
}

void setup()
{
        Serial.begin(115200);
        while (!Serial)
                ;

        pinMode(pinFrontStopSwitch, INPUT_PULLUP);
        pinMode(pinBackStopSwitch, INPUT_PULLUP);

        // Set up the interrupt handlers for the quadrature encoders
        // Left encoder
        pinMode(leftEncoderPinA, INPUT_PULLUP);
        pinMode(leftEncoderPinB, INPUT_PULLUP);
        attachInterrupt(digitalPinToInterrupt(leftEncoderPinA),
                        handleLeftMotorInterruptA, RISING);
 
        // Right encoder
        pinMode(rightEncoderPinA, INPUT_PULLUP);
        pinMode(rightEncoderPinB, INPUT_PULLUP);
        attachInterrupt(digitalPinToInterrupt(rightEncoderPinA),
                        handleRightMotorInterruptA, RISING);

        // Set up the interrupt handlers reading the PWM signals of
        // the RC receiver
        timerstartSpeed = 0; 
        timerstartDirection = 0; 
        if (withRC)
                setup_rc();
        
        // We use the standard Servo library to generate the PWM
        // signal that go to the motor driver
        leftMotor.attach(pinLeftMotor);
        rightMotor.attach(pinRightMotor);
        stop();

        leftPID.SetMode(AUTOMATIC);
        rightPID.SetMode(AUTOMATIC);
        leftPID.SetSampleTime(200);
        rightPID.SetSampleTime(200);
        leftPID.SetOutputLimits(-1.0, 1.0);
        rightPID.SetOutputLimits(-1.0, 1.0);
	//rightPID.SetDebug(true);
}


void stop()
{
        setOutputSignal(0, 0);
        setTargetSpeed(0, 0);
}

void enable()
{
        if (state == STATE_DISABLED)
                state = STATE_ENABLED;
}

void disable()
{
        stop();
        if (state == STATE_ENABLED) 
                state = STATE_DISABLED;
}

void setError(int num)
{
        stop();
        state = STATE_ERROR;
        error = num;
}

const char *getErrorString()
{
        switch (error) {
        case ERROR_NONE:
                return "No error";
        case ERROR_STOPSWITCH:
                return "Stopswitch";
        case ERROR_UNINITIALIZED:
                return "Uninitialized";
        case ERROR_DISABLED:
                return "Disabled";
        case ERROR_ALARM:
                return "Alarm";
        }
        return "Unknown";
}

int frontStopSwitchPressed()
{
        return (digitalRead(pinFrontStopSwitch) == LOW);
}

int backStopSwitchPressed()
{
        return (digitalRead(pinBackStopSwitch) == LOW);
}

void checkStopSwitches()
{
        if (frontStopSwitchPressed()
            || backStopSwitchPressed()) {
                setError(ERROR_STOPSWITCH);
        }
}

void moveAwayFromFrontStopSwitch()
{
        moveat(-100, -100);
        while (frontStopSwitchPressed())
                updateOutputSignal();
        delay(2000);
        stop();
}

void moveAwayFromBackStopSwitch()
{
        moveat(100, 100);
        while (backStopSwitchPressed())
                updateOutputSignal();
        delay(2000);
        stop();        
}

void moveAwayFromStopSwitch()
{
        if (frontStopSwitchPressed())
                moveAwayFromFrontStopSwitch();
        else if (backStopSwitchPressed())
                moveAwayFromBackStopSwitch();
}

// The left and right speed should have a value in [-1, 1].
inline void setOutputSignal(float l, float r)
{
        if (0) {
                static float last_value = -1.0f;
                if (l != last_value) {
                        Serial.print("setOutputSignal ");
                        Serial.println(l);
                        last_value = l;
                }
        }
        if (state != STATE_ENABLED
            && state != STATE_HOMING)
                return;
        
        // Memorize the last values
        leftSpeed = l;
        rightSpeed = r;
        
        // The Servo API expects a value between 0 and 180
        leftSignal = (int) (90.0f + maxSignal * l); 
        rightSignal = (int) (90.0f + maxSignal * r);

        // debug_print_name_value("servo leftMotor: ", (int) leftSignal);
        // debug_print_name_value("servo rightMotor: ", (int) rightSignal);

        leftMotor.write(leftSignal);
        rightMotor.write(rightSignal);
}

inline void setTargetSpeed(float l, float r)
{
        leftTarget = l;
        rightTarget = r;
}

inline void printCurrentSpeed()
{
        unsigned long t = millis();
        debug_print("t=");
        debug_print(t);
        debug_print(",e=");
        debug_print(rightEncoderTicks);
        debug_print(",v=");
        debug_print_float(rightAbsoluteSpeed, 4);
        debug_print(",target=");
        debug_print_float(rightTarget, 4);
        debug_print(",in=");
        debug_print_float(rightInput, 4);
        debug_print(",out=");
        debug_print(rightOutput);
        debug_print(",sig=");
        debug_print((int)rightSignal);
        debug_print("\r\n");
}

inline void measureCurrentSpeed()
{
        static unsigned long lastTime = 0;
        unsigned long t = millis();
        
        if (t > lastTime + 50) {
                float dt = (t - lastTime) / 1000.0;

                leftAbsoluteSpeed = (leftEncoderTicks - leftPrevEncoderTicks) / dt / stepsPerRevolution;
                rightAbsoluteSpeed = (rightEncoderTicks - rightPrevEncoderTicks) / dt / stepsPerRevolution;

                leftInput = leftAbsoluteSpeed / maxSpeed;
                rightInput = rightAbsoluteSpeed / maxSpeed;
                
                leftInput = 0.2 * leftInput + 0.8 * lastLeftInput;
                rightInput = 0.2 * rightInput + 0.8 * lastRightInput;

                //printCurrentSpeed();

                // Update state for next loop
                leftPrevEncoderTicks = leftEncoderTicks;
                rightPrevEncoderTicks = rightEncoderTicks;
                lastLeftInput = leftInput;
                lastRightInput = rightInput;
                lastTime = t;
        }
}

void moveat(int left, int right)
{
        float l = (float) left / 1000.0f;
        float r = (float) right / 1000.0f;
        if (l > 1.0) l = 1.0f;
        if (r > 1.0) r = 1.0f;
        if (l < -1.0) l = -1.0f;
        if (r < -1.0) r = -1.0f;
        
        if (controlMode == CONTROL_DIRECT)  {
                setOutputSignal(l, r);
        } else {
                setTargetSpeed(l, r);
        }
}

void doHoming()
{
        state = STATE_HOMING;
        
        stop();
        
        moveat(-100, -100);
        while (!backStopSwitchPressed()) {
                updateOutputSignal();
                handleSerialInput();
                delay(1);
        }
        
        stop();
        
        moveat(100, 100);
        while (backStopSwitchPressed()) {
                updateOutputSignal();
                handleSerialInput();
                delay(1);
        }

        // Continue for 1 second
        delay(2000);

        stop();
        state = STATE_ENABLED;
}

void switchToSerialInput()
{
        stop();
        inputMode = INPUT_SERIAL;
}

void switchToRCInput()
{
        stop();
        inputMode = INPUT_RC;
}

void switchToDirectControl()
{
        stop();
        controlMode = CONTROL_DIRECT;
}

void switchToPIDControl()
{
        stop();
        lastLeftInput = 0;
        lastRightInput = 0;
        controlMode = CONTROL_PID;
}

void handleRemoteControl()
{        
        if ((inputMode == INPUT_RC)
            && ((rcSpeed != lastRcSpeed)
                || (rcDirection != lastRcDirection))) {
                
                debug_print_name_value("rcSpeed", rcSpeed);
                debug_print_name_value("rcDirection", rcDirection);

                // Map the RC inputs to [-1, 1]
                float speed = map(rcSpeed, 1100, 1900, -1000, 1000) / 1000.0f;
                float direction = map(rcDirection, 1100, 1900, -1000, 1000) / 1000.0f;
                        
                // Filter out small noise in the PWM around zero
                float delta1 = 0.03f;
                
                if (-delta1 < speed && speed < delta1)
                        speed = 0.0f;
                else if (speed < 0.0)
                        speed = speed + delta1;
                else
                        speed = speed - delta1;
                
                if (-delta1 < direction && direction < delta1)
                        direction = 0.0f;
                else if (direction < 0.0)
                        direction = direction + delta1;
                else
                        direction = direction - delta1;

                // Limit the values to [-1, 1]
                if (speed < -1.0f)
                        speed = -1.0f;
                if (speed > 1.0f)
                        speed = 1.0f;
                if (direction < -1.0f)
                        direction = -1.0f;
                if (direction > 1.0f)
                        direction = 1.0f;
                
                // Mix the signal to obtain the speed of the left and the right motor
                float vL = speed - direction;
                float vR = speed + direction;

                // Limit the values to [-1, 1]
                if (vL < -1.0f)
                        vL = -1.0f;
                if (vL > 1.0f)
                        vL = 1.0f;
                if (vR < -1.0f)
                        vR = -1.0f;
                if (vR > 1.0f)
                        vR = 1.0f;

                if (controlMode == CONTROL_PID) {
                        // Set the target speed
                        setTargetSpeed(vL, vR);

                } else {
                        // Set the output signal directly
                        setOutputSignal(vL, vR);
                }
                
                lastRcSpeed = rcSpeed;
                lastRcDirection = rcDirection;
        }
}

int codeIn(char c, const char *validCodes)
{
        for (; *validCodes != '\0'; validCodes++)
                if (c == *validCodes)
                        return 1;
        return 0;
}

void doReset()
{
        stop();
        lastLeftInput = 0;
        lastRightInput = 0;
        error = 0;
        state = STATE_UNINITIALIZED;
}

void handleReset()
{        
        switch(state) {
        case STATE_DISABLED:
        case STATE_HOMING:
        case STATE_ENABLED:
        case STATE_UNINITIALIZED:
        case STATE_ERROR:
                doReset();
                break;
        }
        Serial.println(msgOK);
}

void transmitEncoderValues()
{
        static char buffer[100];
        snprintf(buffer, sizeof(buffer), "e[%ld,%ld,%lu]",
                 leftEncoderTicks, rightEncoderTicks, millis());
        Serial.println(buffer);                
}

void transmitState()
{
        static char buffer[200];
        const char *state_str;
        const char *input_str;
        const char *control_str;
        
        switch (state) {
        case STATE_UNINITIALIZED: state_str = "Uninitialized"; break;
        case STATE_DISABLED: state_str = "Disabled"; break;
        case STATE_ENABLED: state_str = "Enabled"; break;
        case STATE_HOMING: state_str = "Homing"; break;
        case STATE_ERROR: state_str = "Error"; break;
        default: state_str = "?"; break;
        }
        
        switch (inputMode) {
        case INPUT_RC: input_str = "R1"; break;
        case INPUT_SERIAL: input_str = "R0"; break;
        default: input_str = "?"; break;
        }
        
        switch (controlMode) {
        case CONTROL_PID: control_str = "P1"; break;
        case CONTROL_DIRECT: control_str = "P0"; break;
        default: control_str = "?"; break;
        }

        char a[8]; dtostrf(leftTarget, 5, 3, a);
        char b[8]; dtostrf(rightTarget, 5, 3, b);
        char c[8]; dtostrf(leftAbsoluteSpeed, 5, 3, c);
        char d[8]; dtostrf(rightAbsoluteSpeed, 5, 3, d);
        char e[8]; dtostrf(leftInput, 5, 3, e);
        char f[8]; dtostrf(rightInput, 5, 3, f);
        char g[8]; dtostrf(leftOutput, 5, 3, g);
        char h[8]; dtostrf(rightOutput, 5, 3, h);
        char kp[8]; dtostrf(leftPID.GetKp(), 5, 3, kp);
        char ki[8]; dtostrf(leftPID.GetKi(), 5, 3, ki);
        char kd[8]; dtostrf(leftPID.GetKd(), 5, 3, kd);
        
        snprintf(buffer, sizeof(buffer),
                 "S[\"%s\",\"%s\",\"%s\",\"%s\",\"%s\","
                 "%s,%s,%s,%s,%s,%s,%s,%s,"
                 "%d,%d,%d,%d,%s,%s,%s]",
                 state_str, getErrorString(),
                 input_str, control_str,
                 withHoming? "Homing" : "NoHoming",
                 a, b, c, d, e, f, g, h, leftSignal, rightSignal,
                 rcSpeed, rcDirection, kp, ki, kd);
        Serial.println(buffer);                
}

void handleInitialize()
{
        switch(state) {
        case STATE_DISABLED:
                Serial.println(errBadState);
                setError(ERROR_DISABLED);
                break;
        case STATE_HOMING:
                Serial.println(errBadState);
                break;
        case STATE_ERROR:
                Serial.println(errBadState);
                break;
        case STATE_ENABLED:
                // If we receive an init command while we are already
                // enable (possibly moving), stop the rover and set it
                // in an error state. This should never happen and if
                // it does, it needs checking.
                stop();
                setError(ERROR_DISABLED);
                Serial.println(errBadState);
                break;                
        case STATE_UNINITIALIZED:
                stepsPerRevolution = (float) parser.value(0);
                maxSpeed = (float) parser.value(1) / 100.0f;
                maxSignal = (float) parser.value(2);
        
                int enablePID = parser.value(3);
                if (enablePID) {
                        float kp = (float) parser.value(4) / 1000.0f;
                        float ki = (float) parser.value(5) / 1000.0f;
                        float kd = (float) parser.value(6) / 1000.0f;
                        leftPID.SetTunings(kp, ki, kd);
                        rightPID.SetTunings(kp, ki, kd);
                        switchToPIDControl();
                } else {
                        switchToDirectControl();
                }
        
                withRC = (parser.value(7) == 0)? 0 : 1;
                if (withRC)
                        setup_rc(); // FIXME
        
                withHoming = (parser.value(8) == 0)? 0 : 1;
                state = STATE_DISABLED;
                Serial.println(msgOK);
                break;
        }
}

void handleRCEnable()
{
        if (parser.length() == 1) {
                if (parser.value() == 0) {
                        switchToSerialInput();
                        Serial.println(msgOK);
                } else if (parser.value() == 1) { 
                        switchToRCInput();
                        Serial.println(msgOK);
                } else {
                        Serial.println(errValue01);
                }
        } else {
                Serial.println(errValue01);
        }
}

void handleDebugEnable()
{
        if (parser.length() == 1) {
                if (parser.value() == 0) {
                        debug = 0;
                        Serial.println(msgOK);
                } else if (parser.value() == 1) { 
                        debug = 1;
                        Serial.println(msgOK);
                } else {
                        Serial.println(errValue01);
                }
        } else {
                Serial.println(errValue01);
        }
}

void handlePIDEnable()
{
        if (state == STATE_UNINITIALIZED
            || state == STATE_DISABLED) {
                if (parser.length() == 1) {
                        if (parser.value() == 0) {
                                switchToDirectControl();
                                Serial.println(msgOK);
                        } else if (parser.value() == 1) {
                                switchToPIDControl();
                                Serial.println(msgOK);
                        } else {
                                Serial.println(errValue01);
                        }
                } else {
                        Serial.println(errValue01);
                }
        } else {
                Serial.println(errBadState);
        }
}

static inline void setPidValues(float kp, float ki, float kd)
{
        stop();
        delay(1000);
        leftPID.SetTunings(kp, ki, kd);
        rightPID.SetTunings(kp, ki, kd);
}

void handlePIDInit()
{
        if (state == STATE_UNINITIALIZED
            || state == STATE_DISABLED) {
                // K[KP, KI, KD]
                if (parser.length() == 3) {
                        float kp = (float) parser.value(0) / 1000.0f;
                        float ki = (float) parser.value(1) / 1000.0f;
                        float kd = (float) parser.value(2) / 1000.0f;
                        setPidValues(kp, ki, kd);
                        Serial.println(msgOK);
                } else {
                        Serial.println(errMissingValues);
                }
        } else {
                Serial.println(errBadState);
        }
}

void transmitVersion()
{
        Serial.println("?[\"BrushMotorController\",\"0.3\"]"); 
}

void handleMoveat()
{
        switch(state) {
        case STATE_UNINITIALIZED:
                Serial.println(errBadState);
                setError(ERROR_UNINITIALIZED);
                break;
        case STATE_DISABLED:
                Serial.println(errBadState);
                setError(ERROR_DISABLED);
                break;
        case STATE_HOMING:
                Serial.println(errBadState);
                break;
        case STATE_ERROR:
                Serial.println(errBadState);
                break;
        case STATE_ENABLED:
                if (parser.length() == 1) {
                        moveat(parser.value(), parser.value());
                        Serial.println(msgOK);
                } else if (parser.length() == 2) {
                        moveat(parser.value(0), parser.value(1));
                        Serial.println(msgOK);
                } else {
                        Serial.println(errMissingValues);
                }
                break;                
        }
}

void handleHoming()
{
        switch(state) {
        case STATE_UNINITIALIZED:
                Serial.println(errBadState);
                setError(ERROR_UNINITIALIZED);
                break;
        case STATE_DISABLED:
                Serial.println(errBadState);
                setError(ERROR_DISABLED);
                break;
        case STATE_HOMING:
                Serial.println(msgOK);
                break;
        case STATE_ERROR:
                Serial.println(errBadState);
                break;
        case STATE_ENABLED:
                if (withHoming) {
                        doHoming();
                        Serial.println(msgOK);
                } else { 
                        Serial.println(errNotAnOption);
                }
                break;                
        }
}

void handleEnable()
{
        switch(state) {
        case STATE_UNINITIALIZED:
                Serial.println(errBadState);
                setError(ERROR_UNINITIALIZED);
                break;
        case STATE_DISABLED:
                if (parser.value() == 1)
                        enable();
                Serial.println(msgOK);
                break;
        case STATE_HOMING:
                Serial.println(errBadState);
                break;
        case STATE_ERROR:
                Serial.println(errBadState);
                break;
        case STATE_ENABLED:
                if (parser.value() == 0)
                        disable();
                Serial.println(msgOK);
                break;                
        }
}

void handleAlarm()
{
        switchToDirectControl();
        stop();
        setError(ERROR_ALARM);
        Serial.println(msgOK); 
}

void handleSerialInput()
{
        while (Serial.available()) {
                char c = Serial.read();
                if (parser.process(c)) {                        
                        switch (parser.opcode()) {
                        default: break;
                        case '?':
                                transmitVersion();
                                break;
                        case 'e':
                                transmitEncoderValues();
                                break;
                        case 'S':
                                transmitState();
                                break;
                        case 'K':
                                handlePIDInit();
                                break;
                        case 'P':
                                handlePIDEnable();
                                break;
                        case 'R':
                                handleRCEnable();
                                break;
                        case 'D':
                                handleDebugEnable();
                                break;
                        case 'M':
                                handleMoveat();
                                break;
                        case 'I':
                                handleInitialize();
                                break;
                        case '!':
                                handleAlarm();
                                break;
                        case 'X':
                                handleReset();
                                break;
                        case 'H':
                                handleHoming();
                                break;
                        case 'E':
                                handleEnable();
                                break;
                        }
                }
        }
}

void updateOutputSignal()
{
        measureCurrentSpeed();
        if (controlMode == CONTROL_PID) {
                // Check for updates from the PID
                if (leftPID.Compute() || rightPID.Compute()) {
                        // Force small outputs to zero when the target is zero 
                        if (leftTarget == 0 && leftOutput > -0.1 && leftOutput < 0.1)
                                leftOutput = 0.0;
                        if (rightTarget == 0 && rightOutput > -0.1 && rightOutput < 0.1)
                                rightOutput = 0.0;

                        setOutputSignal(leftOutput, rightOutput);
                }
        } else if (controlMode == CONTROL_DIRECT) {
                // Nothing to do
        }
}

void loop()
{
        checkStopSwitches();
        updateOutputSignal();
        handleSerialInput();
        if (withRC)
                handleRemoteControl();

        // As a safety measure, if someone is pulling the RC sticks,
        // give the control back to the RC emitter.
        if (withRC
            && (inputMode == INPUT_SERIAL)
            && ((rcSpeed > 1700) || (1000 < rcSpeed && rcSpeed < 1300))) {
                debug_println("Switching to remote control");
                switchToRCInput();
                delay(3000);
        }
        
        delay(1);
}
 
// Interrupt service routines for the left motor's quadrature encoder
void handleLeftMotorInterruptA()
{
        // Test transition; since the interrupt will only fire on 'rising' we don't need to read pin A
//      bool b = digitalRead(leftEncoderPinB);   // read the input pin
        bool b = readLeftEncoderPinB();
 
        // and adjust counter + if A leads B
#ifdef LeftEncoderIsReversed
        leftEncoderTicks -= b ? -1 : +1;
#else
        leftEncoderTicks += b ? -1 : +1;
#endif
}
 
// Interrupt service routines for the right motor's quadrature encoder
void handleRightMotorInterruptA()
{
        // Test transition; since the interrupt will only fire on 'rising' we don't need to read pin A
//        bool b = digitalRead(rightEncoderPinB);   // read the input pin
        bool b = readRightEncoderPinB();
 
        // and adjust counter + if A leads B
#ifdef RightEncoderIsReversed
        rightEncoderTicks -= b ? -1 : +1;
#else
        rightEncoderTicks += b ? -1 : +1;
#endif
}

#if HAS_RC
void handleRCSpeedUp() 
{
        // Record the interrupt time so that we can tell if the
        // receiver has a signal from the transmitter
        lastInterruptTimeSpeed = micros();
        timerstartSpeed = micros();
        attachInterrupt(digitalPinToInterrupt(rcSpeedPin), handleRCSpeedDown, FALLING);
} 

void handleRCSpeedDown() 
{
        if (timerstartSpeed != 0) { 
                // Record the pulse time
                rcSpeed = micros() - timerstartSpeed;
                // Restart the timer
                timerstartSpeed = 0;
        }
        attachInterrupt(digitalPinToInterrupt(rcSpeedPin), handleRCSpeedUp, RISING);
} 

void handleRCDirectionUp() 
{
        // Record the interrupt time so that we can tell if the
        // receiver has a signal from the transmitter
        lastInterruptTimeDirection = micros();
        timerstartDirection = micros();
        attachInterrupt(digitalPinToInterrupt(rcDirectionPin), handleRCDirectionDown, FALLING);
} 

void handleRCDirectionDown() 
{
        if (timerstartDirection != 0) { 
                // Record the pulse time
                rcDirection = micros() - timerstartDirection;
                // Restart the timer
                timerstartDirection = 0;
        }
        attachInterrupt(digitalPinToInterrupt(rcDirectionPin), handleRCDirectionUp, RISING);
}
#endif
