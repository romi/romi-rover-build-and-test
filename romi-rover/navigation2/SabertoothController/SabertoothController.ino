/*
  Romi motor controller for brushed mortors

  Copyright (C) 2018-2019 Sony Computer Science Laboratories
  Author(s) Peter Hanappe

  The Romi Brush Controller is free software: you can
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
#include <Servo.h>
#include <PID_v1.h>
#include <RomiSerial.h>
#include <ArduinoSerial.h>
#include <SoftwareSerial.h>


void send_info(RomiSerial *romiSerial, int16_t *args, const char *string_arg);
void handle_configure(RomiSerial *romiSerial, int16_t *args, const char *string_arg);
void handle_enable(RomiSerial *romiSerial, int16_t *args, const char *string_arg);
void send_encoders(RomiSerial *romiSerial, int16_t *args, const char *string_arg);
void handle_moveat(RomiSerial *romiSerial, int16_t *args, const char *string_arg);
void send_status(RomiSerial *romiSerial, int16_t *args, const char *string_arg);
void send_configure(RomiSerial *romiSerial, int16_t *args, const char *string_arg);
void handle_stop(RomiSerial *romiSerial, int16_t *args, const char *string_arg);

const static MessageHandler handlers[] = {
        { '?', 0, false, send_info },
        { 'e', 0, false, send_encoders },
        { 'V', 2, false, handle_moveat },
        { 'C', 9, false, handle_configure },
        { 'E', 1, false, handle_enable },
        { 'S', 0, false, send_status },
        { 'c', 0, false, send_configuration },
        { 'X', 0, false, handle_stop },
};

ArduinoSerial serial;
RomiSerial romiSerial(serial, serial, handlers, sizeof(handlers) / sizeof(MessageHandler));

/////////////////////////////////////////////////////////////////////////

enum { STATE_UNINITIALIZED, STATE_DISABLED, STATE_ENABLED, STATE_ERROR };
enum { CONTROL_PID, CONTROL_DIRECT };
enum { ERROR_NONE = 0, ERROR_STOPSWITCH, ERROR_UNINITIALIZED, ERROR_DISABLED };

const char *msgOK = "OK";
const char *errBadState = "ERR bad state";
const char *errValue01 = "ERR expected 0|1";
const char *errMissingValues = "ERR missing values";
const char *errNotAnOption = "ERR not an option";

unsigned char state = STATE_UNINITIALIZED;
unsigned char controlMode = CONTROL_DIRECT;

/////////////////////////////////////////////////////////////////////////
#define leftEncoderPinA 2
#define leftEncoderPinB 4
#define rightEncoderPinA 3
#define rightEncoderPinB 5
#define pinFrontStopSwitch 11
#define pinBackStopSwitch 12

/////////////////////////////////////////////////////////////////////////

#define sabertoothTxPin 9
#define sabertoothRxPin 13

SoftwareSerial sabertooth(sabertoothRxPin, sabertoothTxPin); // RX, TX

/////////////////////////////////////////////////////////////////////////
// The H-bridge expects a PWM signal similar to those produced by RC
// receiver or accepted by servo motors. For ease, we use the standard
// Servo library to produce these signals.

//Servo leftMotor;
//Servo rightMotor;
// The maximum amplitude of the servo signal.
float maxSignal = 20.0f;

/////////////////////////////////////////////////////////////////////////
double lastLeftInput = 0;
double lastRightInput = 0;

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
volatile long leftEncoderTicks = 0;
long leftPrevEncoderTicks = 0;
long leftEncoderDirection = 1;

// Right encoder
volatile long rightEncoderTicks = 0;
long rightPrevEncoderTicks = 0;
long rightEncoderDirection = 1;

#define readLeftEncoderPinB() digitalRead(leftEncoderPinB)
#define readRightEncoderPinB() digitalRead(rightEncoderPinB)

/////////////////////////////////////////////////////////////////////////

void setup()
{
        serial.init(115200);
        
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
        
        // We use the standard Servo library to generate the PWM
        // signal that go to the motor driver
        //leftMotor.attach(pinLeftMotor);
        //rightMotor.attach(pinRightMotor);
        sabertooth.begin(9600);
  
        stop();

        leftPID.SetMode(AUTOMATIC);
        rightPID.SetMode(AUTOMATIC);
        leftPID.SetSampleTime(200);
        rightPID.SetSampleTime(200);
        leftPID.SetOutputLimits(-1.0, 1.0);
        rightPID.SetOutputLimits(-1.0, 1.0);
	//rightPID.SetDebug(true);
}

void send_info(RomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        romiSerial->send("[0,\"SabertoothController\",\"0.1\","
                         "\"" __DATE__ " " __TIME__ "\"]"); 
}

void stop()
{
        setOutputSignal(0, 0);
        setTargetSpeed(0, 0);
}

void enable()
{
        state = STATE_ENABLED;
}

void disable()
{
        stop();
        state = STATE_DISABLED;
}

// The left and right speed should have a value in [-1, 1].
/*
inline void setOutputSignalSERVO(float l, float r)
{
        if (0) {
                static float last_value = -1.0f;
                if (l != last_value) {
                        Serial.print("setOutputSignal ");
                        Serial.println(l);
                        last_value = l;
                }
        }
        // Memorize the last values
        leftSpeed = l;
        rightSpeed = r;
        
        // The Servo API expects a value between 0 and 180
        leftSignal = (int) (90.0f + maxSignal * l); 
        rightSignal = (int) (90.0f + maxSignal * r);

        leftMotor.write(leftSignal);
        rightMotor.write(rightSignal);
}
*/

inline void sabertooth_putc(unsigned char c)
{
        sabertooth.write(c);
}

#define ADDRESS 128
#define kLeftDriveForward 0
#define kLeftDriveBackward 1
#define kRightDriveForward 4
#define kRightDriveBackward 5

inline void sendCommand(unsigned char command, unsigned char data)
{
        unsigned char checksum = (ADDRESS + command + data) & 0b01111111;
        sabertooth_putc(ADDRESS);
        sabertooth_putc(command);
        sabertooth_putc(data);
        sabertooth_putc(checksum);
}

inline unsigned char speedToData(float speed)
{
        if (speed < 0.0)
                return speedToData(-speed);
        return (unsigned char) (speed * 127.0);
}

inline void leftDriveForwardAt(float speed)
{
        sendCommand(kLeftDriveForward, speedToData(speed));
}

inline void leftDriveBackwardAt(float speed)
{
        sendCommand(kLeftDriveBackward, speedToData(speed));
}

inline void leftDriveAt(float speed)
{
        if (speed >= 0.0)
                leftDriveForwardAt(speed);
        else 
                leftDriveBackwardAt(speed);
}

inline void rightDriveForwardAt(float speed)
{
        sendCommand(kRightDriveForward, speedToData(speed));
}

inline void rightDriveBackwardAt(float speed)
{
        sendCommand(kRightDriveBackward, speedToData(speed));
}

inline void rightDriveAt(float speed)
{
        if (speed >= 0.0)
                rightDriveForwardAt(speed);
        else 
                rightDriveBackwardAt(speed);
}

inline void setOutputSignal(float l, float r)
{
        // Memorize the last values
        leftSpeed = l;
        rightSpeed = r;
        
        leftDriveAt(leftSpeed);
        rightDriveAt(rightSpeed);
}

inline void setTargetSpeed(float l, float r)
{
        leftTarget = l;
        rightTarget = r;
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
                setTargetSpeed(l, r);
                setOutputSignal(l, r);
        } else {
                setTargetSpeed(l, r);
        }
}

void send_encoders(RomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        static char buffer[100];
        snprintf(buffer, sizeof(buffer), "[0,%ld,%ld,%lu]",
                 leftEncoderTicks, rightEncoderTicks, millis());
        romiSerial->send(buffer);                
}

void send_status(RomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        static char buffer[64];
        const char *state_str;
        const char *control_str;
        
        switch (state) {
        case STATE_UNINITIALIZED: state_str = "U"; break;
        case STATE_DISABLED: state_str = "D"; break;
        case STATE_ENABLED: state_str = "E"; break;
        case STATE_ERROR: state_str = "X"; break;
        default: state_str = "?"; break;
        }

        char c[8]; dtostrf(leftAbsoluteSpeed, 5, 3, c);
        char d[8]; dtostrf(rightAbsoluteSpeed, 5, 3, d);
        char e[8]; dtostrf(leftInput, 5, 3, e);
        char f[8]; dtostrf(rightInput, 5, 3, f);
        char g[8]; dtostrf(leftOutput, 5, 3, g);
        char h[8]; dtostrf(rightOutput, 5, 3, h);
        
        snprintf(buffer, sizeof(buffer),
                 "[0,\"%s\",%d,%d,%s,%s,%s,%s,%s,%s,%d,%d]",
                 state_str, 
                 (int) (1000.0 * leftTarget),
                 (int) (1000.0 * rightTarget),
                 c, d, e, f, g, h, leftSignal, rightSignal);
        
        romiSerial->send(buffer);                
}

void send_configuration(RomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        static char buffer[64];
        const char *control_str;
        
        switch (controlMode) {
        case CONTROL_PID: control_str = "P1"; break;
        case CONTROL_DIRECT: control_str = "P0"; break;
        default: control_str = "?"; break;
        }

        char kp[8]; dtostrf(leftPID.GetKp(), 5, 3, kp);
        char ki[8]; dtostrf(leftPID.GetKi(), 5, 3, ki);
        char kd[8]; dtostrf(leftPID.GetKd(), 5, 3, kd);
        
        snprintf(buffer, sizeof(buffer), "[0,\"%s\",%s,%s,%s]", control_str, kp, ki, kd);
        
        romiSerial->send(buffer);                
}

void handle_configure(RomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        switch(state) {
        case STATE_ERROR:
        case STATE_ENABLED:
                // If we receive an init command while we are already
                // enable (possibly moving), stop the rover and set it
                // in an error state. This should never happen and if
                // it does, it needs checking.
                stop();
                romiSerial->send_error(1, errBadState);  
                break;                
        case STATE_DISABLED:
        case STATE_UNINITIALIZED:
                stepsPerRevolution = (float) args[0];
                maxSpeed = (float) args[1] / 100.0f;
                maxSignal = (float) args[2];
        
                int enablePID = args[3];
                if (enablePID) {
                        float kp = (float) args[4] / 1000.0f;
                        float ki = (float) args[5] / 1000.0f;
                        float kd = (float) args[6] / 1000.0f;
                        leftPID.SetTunings(kp, ki, kd);
                        rightPID.SetTunings(kp, ki, kd);
                        lastLeftInput = 0;
                        lastRightInput = 0;
                        controlMode = CONTROL_PID;
                } else {
                        controlMode = CONTROL_DIRECT;
                }
                leftEncoderDirection = args[7] > 0? 1 : -1;
                rightEncoderDirection = args[8] > 0? 1 : -1;
                state = STATE_DISABLED;
                romiSerial->send_ok();  
                break;
        }
}

void handle_stop(RomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        stop();
        romiSerial->send_ok();  
}

void handle_moveat(RomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        switch(state) {
        case STATE_ERROR:
        case STATE_DISABLED:
        case STATE_UNINITIALIZED:
                romiSerial->send_error(1, errBadState);  
                break;
        case STATE_ENABLED:
                moveat(args[0], args[1]);
                romiSerial->send_ok();  
                break;                
        }
}

void handle_enable(RomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        switch(state) {
        case STATE_ERROR:
        case STATE_UNINITIALIZED:
                romiSerial->send_error(1, errBadState);  
                break;
        case STATE_DISABLED:
                if (args[0] != 0)
                        enable();
                romiSerial->send_ok();  
                break;
        case STATE_ENABLED:
                if (args[0] == 0)
                        disable();
                romiSerial->send_ok();  
                break;                
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
        updateOutputSignal();
        romiSerial.handle_input();
        
        delay(1);
}
 
// Interrupt service routines for the left motor's quadrature encoder
void handleLeftMotorInterruptA()
{
        bool b = readLeftEncoderPinB();
        leftEncoderTicks += b ? -leftEncoderDirection : +leftEncoderDirection;
}
 
// Interrupt service routines for the right motor's quadrature encoder
void handleRightMotorInterruptA()
{
        bool b = readRightEncoderPinB();
        rightEncoderTicks += b ? -rightEncoderDirection : +rightEncoderDirection;
}
