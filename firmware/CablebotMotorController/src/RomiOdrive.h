#pragma once

#include <HardwareSerial.h>
#include <Arduino.h>

// Resources
// https://docs.odriverobotics.com/ascii-protocol

class RomiOdrive {
	
public:
        // ODrive.Axis.AxisState
        // https://docs.odriverobotics.com/api/odrive.axis.axisstate
        enum AxisState {
                AXIS_STATE_UNDEFINED                     = 0,
                AXIS_STATE_IDLE                          = 1,
                AXIS_STATE_STARTUP_SEQUENCE              = 2,
                AXIS_STATE_FULL_CALIBRATION_SEQUENCE     = 3,
                AXIS_STATE_MOTOR_CALIBRATION             = 4,
                AXIS_STATE_ENCODER_INDEX_SEARCH          = 6,
                AXIS_STATE_ENCODER_OFFSET_CALIBRATION    = 7,
                AXIS_STATE_CLOSED_LOOP_CONTROL           = 8,
                AXIS_STATE_LOCKIN_SPIN                   = 9,
                AXIS_STATE_ENCODER_DIR_FIND              = 10,
                AXIS_STATE_HOMING                        = 11,
                AXIS_STATE_ENCODER_HALL_POLARITY_CALIBRATION = 12,
                AXIS_STATE_ENCODER_HALL_PHASE_CALIBRATION = 13
        };

        enum ControlMode {
                CONTROL_MODE_VOLTAGE_CONTROL = 0,
                CONTROL_MODE_TORQUE_CONTROL = 1,
                CONTROL_MODE_VELOCITY_CONTROL = 2,
                CONTROL_MODE_POSITION_CONTROL = 3
        };

        enum InputMode {
                INPUT_MODE_INACTIVE = 0,
                INPUT_MODE_PASSTHROUGH = 1,
                INPUT_MODE_VEL_RAMP = 2,
                INPUT_MODE_POS_FILTER = 3,
                INPUT_MODE_MIX_CHANNELS = 4,
                INPUT_MODE_TRAP_TRAJ = 5,
                INPUT_MODE_TORQUE_RAMP = 6,
                INPUT_MODE_MIRROR = 7,
                INPUT_MODE_TUNNING = 8
        };
		
        RomiOdrive (HardwareSerial& _s, uint8_t _pin_reset): odrvSer(_s) {
                odrvSer = _s;
                pin_reset = _pin_reset;
                pinMode(pin_reset, OUTPUT);
                digitalWrite(pin_reset, HIGH);
        }; 


        bool begin();
        void reset(bool hardReset=true);
        void rawCommand(const char* rawcom);

        bool setState(AxisState wichState);
        bool waitState(AxisState whichState, uint32_t timeout_ms);
        bool isEncoderOK();

        enum Info {
                INFO_VBUS_VOLTAGE,
                INFO_CURRENT_LIMIT,
                INFO_MEASURED_CURRENT,
                INFO_POSITION,
                INFO_AXIS_ERROR,
                INFO_CONTROLLER_ERROR,
                INFO_MOTOR_ERROR,
                INFO_VERSION
        };

        float getInfo(Info whichInfo);
        String readParameter(const char* whichParameter);
        float getPosition(); // in turns

        void stop();
        void moveTo(float posInTurns);
        void moveToWithRamp(float posInTurns);
        void moveAt(float speed);
        void moveAtNormalizedSpeed(float speed);

        // ramp_rate is in turns/sec²
        void moveAtWithRamp(float velocity, float ramp_rate = 5);

        // Normalized velocity (-1 - 1)
        void moveAtNormalizedSpeedWithRamp(float velocity, float ramp_rate = 5); 	

private:

        // TODO make this presistent and configurable by the user
        struct Config {
                uint32_t serial_speed = 115200;
                uint32_t serial_timeout = 2000; // in ms
                uint32_t idle_timeout = 10000; // in ms

                float odrvVer = 5.10;
			
                int current_limit = 40;
			 
                float turnsPerMeter = 6.28;
                uint32_t encoderTicks = 8192;
			
                float maxSpeed = 2.0; 	// m/s
                float maxAccel = 1.0;  //
        };
        
        struct State {
                bool serialStarted = false;
        };

        HardwareSerial& odrvSer;
        Config config;
        State state;
        uint8_t pin_reset;
        static const uint8_t OBSIZE = 255;
        char obuff[OBSIZE];

        void send(const char* msg);
        void send();
        String getString();
		
        // Utility functions
        bool setVersion(float fallback = 5.10f);
        int ticksPerTurn();
};
