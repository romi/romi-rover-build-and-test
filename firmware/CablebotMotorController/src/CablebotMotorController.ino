//// Romi cablebot MK2 motor controller

#include <math.h>
#include "wiring_private.h" // For pinPeripheral()
#include <FastLED.h>
#include <ArduinoSerial.h>
#include <RomiSerial.h>
#include "RomiOdrive.h"
#include "State.h"
#include "Pins.h"
#include "Endstop.h"
#include "test.h"

using namespace romiserial;

static float kHomingSpeed = 0.2f;

// Odrive
RomiOdrive romi_odrive(Serial1, kPinOdriveReset);

static float get_position();
static void check_battery();

        
void handle_moveto(IRomiSerial *romiSerial, int16_t *args, const char *string_arg);
void handle_moveat(IRomiSerial *romiSerial, int16_t *args, const char *string_arg);
void send_position(IRomiSerial *romiSerial, int16_t *args, const char *string_arg);
void handle_homing(IRomiSerial *romiSerial, int16_t *args, const char *string_arg);
void handle_enable(IRomiSerial *romiSerial, int16_t *args, const char *string_arg);
void handle_restart(IRomiSerial *romiSerial, int16_t *args, const char *string_arg);
void send_on_position(IRomiSerial *romiSerial, int16_t *args, const char *string_arg);
void send_status(IRomiSerial *romiSerial, int16_t *args, const char *string_arg);
void send_info(IRomiSerial *romiSerial, int16_t *args, const char *string_arg);
void run_tests(IRomiSerial *romiSerial, int16_t *args, const char *string_arg);

const static MessageHandler handlers[] = {
        { 'm', 1, false, handle_moveto },
        { 'M', 1, false, handle_moveat },
        { 'P', 0, false, send_position },
        { 'p', 0, false, send_on_position },
        { 'H', 0, false, handle_homing },
        { 'E', 1, false, handle_enable },
        { 'S', 0, false, send_status },
        { 'R', 0, false, handle_restart },
        { '?', 0, false, send_info },
        { 'T', 1, false, run_tests },
};

// Camera module
Uart camera_serial(&sercom1, kPinSerialCameraRX, kPinSerialCameraTX,
                   SERCOM_RX_PAD_3, UART_TX_PAD_2 ) ;
ArduinoSerial serial(camera_serial);
RomiSerial romiSerial(serial, serial, handlers, sizeof(handlers) / sizeof(MessageHandler));

// Debug interface
ArduinoSerial debugSerial(Serial);
RomiSerial debugRomiSerial(debugSerial, debugSerial, handlers,
                           sizeof(handlers) / sizeof(MessageHandler));


void SERCOM1_Handler() { camera_serial.IrqHandler(); }

State state;

float start_position = 0.0f;
float target_position = 0.0f;
static const constexpr float kMaximumPositionError = 0.01f;
static char reply_string[80];

        
void setup()
{
	//// Led
        state.init();
        
	//// Debug USB serial port
        state.set(STATE_INITIALIZING_DEBUG_SERIAL);
	Serial.begin(115200);
        // while (!Serial)
        //         ;
        
	//// Camera module
	// Serial port
	// Assign sercom functionality
        state.set(STATE_INITIALIZING_CAMERA_SERIAL);
	camera_serial.begin(115200);
	pinPeripheral(kPinSerialCameraRX, PIO_SERCOM);
	pinPeripheral(kPinSerialCameraTX, PIO_SERCOM);
        
        //// Endstops interrupt setup
        state.set(STATE_INITIALIZING_SWITCHES);
        init_endstops();
        
	//// Odrive
        state.set(STATE_INITIALIZING_MOTORCONTROLLER);
        if (init_odrive()) {
                state.set(STATE_INITIALIZING_POSITION);
                init_position();
                state.set(STATE_READY);
        }
}

bool init_odrive()
{
        bool success = false;
        
	if (romi_odrive.begin()) {
                // if (romi_odrive.setState(RomiOdrive::AXIS_STATE_ENCODER_INDEX_SEARCH)) {
                        // if (romi_odrive.isEncoderOK()) {
                if (romi_odrive.waitState(RomiOdrive::AXIS_STATE_CLOSED_LOOP_CONTROL, 20000)) {
                                // if (romi_odrive.setState(RomiOdrive::AXIS_STATE_CLOSED_LOOP_CONTROL)) {
                                        success = true;

                                } else {
                                        state.set_error(ERROR_ODRIVE,
                                                  "Closed-loop control failed");
                                }
                        // } else {
                        //         state.set_error(ERROR_ODRIVE, "Encoder not ok");
                        // }
                // } else {
                //         state.set_error(ERROR_ODRIVE, "Index search failed");
                // }
        } else {
                state.set_error(ERROR_ODRIVE, "Odrive begin failed");
        }
        return success;
}

void emergency_stop()
{
        stop();
}

void stop()
{
	// This should stop the motor smoothly
        romi_odrive.moveAt(0.0);
        romi_odrive.stop();
}

bool doHoming = false;

void loop()
{
        if (doHoming) {
                do_homing();
                doHoming = false;
        }
        
        check_endstops();
        check_battery();
        state.update();
        romiSerial.handle_input();
        debugRomiSerial.handle_input();
}

static inline float steps_to_position(int16_t steps)
{
        return steps / 36.0f;
}

static inline float position_to_steps(float position)
{
        return position * 36.0f;
}

void init_position()
{
        if (state.error() == ERROR_NONE) {
                start_position = romi_odrive.getPosition();
                target_position = 0.0f;
        }
}

float get_position()
{
        return romi_odrive.getPosition() - start_position;
}

bool set_position(float position)
{
        target_position = position;
        romi_odrive.moveToWithRamp(start_position + position);
        return true; // TODO
}

float distance_to_target()
{
        float position = get_position();
        float distance = fabsf(position - target_position);
        return distance;
}

bool moveto(float position, unsigned long timeout)
{
        //Serial.print("moveto ");
        //Serial.println(position);
        return set_position(position);
}

int moveat(float speed)
{
        //Serial.print("moveat ");
        //Serial.println(speed);
        romi_odrive.moveAt(speed); // turns/s
        return 0;
}

void handle_moveto(IRomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        if (state.error() == ERROR_NONE) {
                int16_t steps = args[0];
                float position = steps_to_position(steps);

                // Timeout = one minute: TODO: should be based on distance x speed
                bool success = moveto(position, 60000);
                if (success)
                        romiSerial->send_ok();
                else
                        romiSerial->send_error(1, "Moveto failed");
        } else {
                romiSerial->send_error(state.error(), state.message());
        }
}

void handle_moveat(IRomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        if (state.error() == ERROR_NONE) {
                int16_t rpm = args[0];
                float speed = (float) rpm / 60.0f;

                // Timeout = one minute: TODO: should be based on distance x speed
                int err = moveat(speed);
                if (err == 0)
                        romiSerial->send_ok();
                else
                        romiSerial->send_error(1, "Moveto failed");
        } else {
                romiSerial->send_error(state.error(), state.message());
        }
}

void send_position(IRomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        if (state.error() == ERROR_NONE) {
                float position = get_position();
                snprintf(reply_string, sizeof(reply_string), "[0,%0.2f]",
                         position_to_steps(position));
                romiSerial->send(reply_string); 
        } else {
                romiSerial->send_error(state.error(), state.message());
        }
}

void handle_homing(IRomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        if (state.error() == ERROR_NONE) {
                doHoming = true;
                romiSerial->send_ok();
        } else {
                romiSerial->send_error(state.error(), state.message());
        }
}

int do_homing()
{
        auto t0 = millis();
        romi_odrive.moveAt(-kHomingSpeed); // turns/s

        Serial.println("#!homing");        

        while (is_endstop_charger_pressed() == false) {
                if (millis() - t0 > 30000) {
                        state.set_error(ERROR_HOMING, "Homing timed out");
                        return -1;
                }
                
                romiSerial.handle_input();
                debugRomiSerial.handle_input();
                delay(10);
                Serial.print("#! Endstop: ");
                Serial.println(is_endstop_charger_pressed());
        }
        
        romi_odrive.moveAt(0); // turns/s
        init_position();
        
        Serial.println("#!homing done");
        
        return 0;
}

void handle_enable(IRomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        bool enable = args[0] == 1;
        if (enable) { 
                //romi_odrive.setState(XXX)
        } else {
                romi_odrive.stop(); // or stop()??
                //romi_odrive.setState(RomiOdrive::AXIS_STATE_IDLE)
        }
        romiSerial->send_ok();  
}

void send_on_position(IRomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        float distance = distance_to_target();
        bool on_position = (distance < kMaximumPositionError);
        snprintf(reply_string, sizeof(reply_string), "[0,%d,%0.3f]",
                 (int) on_position, distance);
        romiSerial->send(reply_string);         
}

void check_battery()
{
        static unsigned long last_battery_check = 0;
        unsigned long now = millis();
        if (now - last_battery_check > 30000) {
                do_check_battery();
                last_battery_check = now;
        }
}

void do_check_battery()
{
        float voltage = romi_odrive.getInfo(RomiOdrive::INFO_VBUS_VOLTAGE);
        state.set_battery(voltage);
}

void send_status(IRomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        float distance = distance_to_target();
        int on_target = (int)(distance < kMaximumPositionError);
        float voltage = romi_odrive.getInfo(RomiOdrive::INFO_VBUS_VOLTAGE);
        float current_limit = romi_odrive.getInfo(RomiOdrive::INFO_CURRENT_LIMIT);
        float current = romi_odrive.getInfo(RomiOdrive::INFO_MEASURED_CURRENT);
        float position = get_position();
        int axis_error = (int) romi_odrive.getInfo(RomiOdrive::INFO_AXIS_ERROR);
        int ctrl_error = (int) romi_odrive.getInfo(RomiOdrive::INFO_CONTROLLER_ERROR);
        int motor_error = (int) romi_odrive.getInfo(RomiOdrive::INFO_MOTOR_ERROR);
        float version = romi_odrive.getInfo(RomiOdrive::INFO_VERSION);
        snprintf(reply_string, sizeof(reply_string),
                 "[0,%d,%d,%0.3f,%0.1f,%0.1f,%0.1f,%0.2f,%d,%d,%d,%0.2f]",
                 state.error(), on_target, distance, voltage, current_limit, current,
                 position, axis_error, ctrl_error, motor_error, version);
        romiSerial->send(reply_string);         
}

void send_info(IRomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        romiSerial->send("[0,\"CablebotMotorController\",\"0.1\",\""
                         __DATE__ " " __TIME__ "\"]"); 
}

void handle_restart(IRomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        if (!romi_odrive.setState(RomiOdrive::AXIS_STATE_ENCODER_INDEX_SEARCH)) {
		state.set_error(ERROR_ODRIVE, "Index search failed");
        } else if (!romi_odrive.isEncoderOK()) {
		state.set_error(ERROR_ODRIVE, "Encoder not ok");
        } else if (!romi_odrive.setState(RomiOdrive::AXIS_STATE_CLOSED_LOOP_CONTROL)) {
		state.set_error(ERROR_ODRIVE, "Closed-loop control failed");
	} else {
                state.clear_error();
                init_position();
        }
}

void run_tests(IRomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        switch (args[0]) {
        case 0:
                run_tests_all();
                break;
        case 1:
                run_tests_leds();
                break;
        case 2:
                run_tests_motor();
                break;
        default:
                break;
        }
        romiSerial->send_ok();  
}

