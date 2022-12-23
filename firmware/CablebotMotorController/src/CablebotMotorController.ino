//// Romi cablebot MK2 motor controller

#include <math.h>
#include "wiring_private.h" // For pinPeripheral()
#include <FastLED.h>
#include <ArduinoSerial.h>
#include <RomiSerial.h>
#include "RomiOdrive.h"
#include "State.h"

using namespace romiserial;

//// Pin definitions

// Input pins
#define PIN_ENDSTOP_BACK 9
#define PIN_ENDSTOP_FRONT 6

// Odrive pins
#define PIN_ODRIVE_RESET 15

// Camera module pins
#define PIN_SERIAL_CAM_TX 10
#define PIN_SERIAL_CAM_RX 12

// Odrive
RomiOdrive romi_odrive(Serial1, PIN_ODRIVE_RESET);

static float get_position();
static void check_battery();

        
void handle_moveto(IRomiSerial *romiSerial, int16_t *args, const char *string_arg);
void send_position(IRomiSerial *romiSerial, int16_t *args, const char *string_arg);
void handle_homing(IRomiSerial *romiSerial, int16_t *args, const char *string_arg);
void handle_enable(IRomiSerial *romiSerial, int16_t *args, const char *string_arg);
void handle_restart(IRomiSerial *romiSerial, int16_t *args, const char *string_arg);
void send_on_position(IRomiSerial *romiSerial, int16_t *args, const char *string_arg);
void send_status(IRomiSerial *romiSerial, int16_t *args, const char *string_arg);
void send_info(IRomiSerial *romiSerial, int16_t *args, const char *string_arg);

const static MessageHandler handlers[] = {
        { 'm', 1, false, handle_moveto },
        { 'P', 0, false, send_position },
        { 'p', 0, false, send_on_position },
        { 'H', 0, false, handle_homing },
        { 'E', 1, false, handle_enable },
        { 'S', 0, false, send_status },
        { 'R', 0, false, handle_restart },
        { '?', 0, false, send_info },
};

// Camera module
Uart camera_serial(&sercom1, PIN_SERIAL_CAM_RX, PIN_SERIAL_CAM_TX,
                   SERCOM_RX_PAD_3, UART_TX_PAD_2 ) ;
ArduinoSerial serial(camera_serial);
RomiSerial romiSerial(serial, serial, handlers, sizeof(handlers) / sizeof(MessageHandler));

void SERCOM1_Handler() { camera_serial.IrqHandler(); }

State state;


// Endstops
volatile bool endStopFront = false;
volatile bool endStopBack = false;
uint32_t last_endStopFront_interrupt_time = 0;
uint32_t last_endStopBack_interrupt_time = 0;

// Leds
#define NUM_LEDS 3
CRGB leds[NUM_LEDS];

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
	pinPeripheral(PIN_SERIAL_CAM_RX, PIO_SERCOM);
	pinPeripheral(PIN_SERIAL_CAM_TX, PIO_SERCOM);

	//// Odrive 
        state.set(STATE_INITIALIZING_ODRIVE);
        if (init_odrive()) {
                state.set(STATE_INITIALIZING_POSITION);
                init_position();
                
                //// Endstops interrupt setup
                state.set(STATE_INITIALIZING_SWITCHES);
                pinMode(PIN_ENDSTOP_BACK, INPUT_PULLUP);
                pinMode(PIN_ENDSTOP_FRONT, INPUT_PULLUP);
                attachInterrupt(digitalPinToInterrupt(PIN_ENDSTOP_FRONT),
                                endStopFront_irq, LOW);
                attachInterrupt(digitalPinToInterrupt(PIN_ENDSTOP_BACK),
                                endStopBack_irq, LOW);
                
                state.set(STATE_READY);
        }
}

bool init_odrive()
{
        bool success = false;
        
	if (romi_odrive.begin()) {
                if (romi_odrive.setState(RomiOdrive::AXIS_STATE_ENCODER_INDEX_SEARCH)) {
                        if (romi_odrive.isEncoderOK()) {
                                if (romi_odrive.setState(RomiOdrive::AXIS_STATE_CLOSED_LOOP_CONTROL)) {
                                        success = true;

                                } else {
                                        state.set_error(ERROR_ODRIVE,
                                                  "Closed-loop control failed");
                                }
                        } else {
                                state.set_error(ERROR_ODRIVE, "Encoder not ok");
                        }
                } else {
                        state.set_error(ERROR_ODRIVE, "Index search failed");
                }
        } else {
                state.set_error(ERROR_ODRIVE, "Odrive begin failed");
        }
        return success;
}

void stop()
{
	// This should stop the motor smoothly
}

void eStop()
{
	// This should stop as fast as possible
}

void handleEndstop()
{
	// TODO
	// Implement recover routine after endStop collision

	eStop();

	if (endStopFront) {
		Serial.println("Crashed on front!!");
	} else if (endStopBack) {
		Serial.println("Crashed on back!!");
	}

	stop();

	endStopFront = false;
	endStopBack = false;
}

void check_end_stops()
{
	if (endStopBack || endStopFront) {
		handleEndstop();
	}
}

void loop()
{
        check_end_stops();
        check_battery();
        state.handle_leds();
        romiSerial.handle_input();
}

// Interrupt for endstops
void endStopFront_irq()
{
	if ((millis() - last_endStopFront_interrupt_time) > 50)
                endStopFront = true;
	last_endStopFront_interrupt_time = millis();
}

void endStopBack_irq()
{
	if ((millis() - last_endStopBack_interrupt_time) > 50)
                endStopBack = true;
	last_endStopBack_interrupt_time = millis();
}


static inline float steps_to_position(int16_t steps)
{
        return steps / 36.0f;
}

static inline float position_to_steps(float position)
{
        return position * 36.0f;
}

static void init_position()
{
        if (state.error == ERROR_NONE) {
                start_position = romi_odrive.getPosition();
                target_position = 0.0f;
        }
}

static float get_position()
{
        return romi_odrive.getPosition() - start_position;
}

static bool set_position(float position)
{
        target_position = position;
        romi_odrive.moveToAcc(start_position + position);
        return true; // TODO
}

static float distance_to_target()
{
        float position = get_position();
        float distance = fabsf(position - target_position);
        return distance;
}

static bool moveto(float position, unsigned long timeout)
{
        Serial.print("moveto ");
        Serial.println(position);
        return set_position(position);
}

void handle_moveto(IRomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        if (state.error == ERROR_NONE) {
                int16_t steps = args[0];
                float position = steps_to_position(steps);

                // Timeout = one minute: TODO: should be based on distance x speed
                bool success = moveto(position, 60000);
                if (success)
                        romiSerial->send_ok();
                else
                        romiSerial->send_error(1, "Moveto failed");
        } else {
                romiSerial->send_error(state.error, state.error_message);
        }
}

void send_position(IRomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        if (state.error == ERROR_NONE) {
                float position = get_position();
                snprintf(reply_string, sizeof(reply_string), "[0,%0.2f]",
                         position_to_steps(position));
                romiSerial->send(reply_string); 
        } else {
                romiSerial->send_error(state.error, state.error_message);
        }
}

void handle_homing(IRomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        if (state.error == ERROR_NONE) {
                Serial.println("homing");        
                set_position(0);  // TODO
                romiSerial->send_ok();  
        } else {
                romiSerial->send_error(state.error, state.error_message);
        }
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

static void do_check_battery()
{
        float voltage = romi_odrive.getInfo(RomiOdrive::INFO_VBUS_VOLTAGE);
        state.set_battery(voltage);
        Serial.print("voltage ");
        Serial.println(voltage);
}

static void check_battery()
{
        static unsigned long last_battery_check = 0;
        unsigned long now = millis();
        if (now - last_battery_check > 5000) {
                do_check_battery();
                last_battery_check = now;
        }
}

void send_status(IRomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        int on_target = (int)(distance < kMaximumPositionError);
        float distance = distance_to_target();
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
                 state.error, on_target, distance, voltage, current_limit, current,
                 position, axis_error, ctrl_error, motor_error, version);
        romiSerial->send(reply_string);         
}

void send_info(IRomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        romiSerial->send("[0,\"CablebotMotorController\",\"0.1\",\"" __DATE__ " " __TIME__ "\"]"); 
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


