//// Romi cablebot MK2 motor controller

#include "wiring_private.h" // For pinPeripheral()
#include "RomiOdrive.h"

//// Pin definitions
// Input pins
#define PIN_RC_THR 13
#define PIN_ENDSTOP_BACK 9
#define PIN_ENDSTOP_FRONT 6
#define PIN_BATT_VOLTAGE 14
#define PIN_BUTTON A2
#define PIN_CHARGING_STATION 17
// Output pins
#define PIN_ADDR_LED 5

// Odrive pins
#define PIN_SERIAL_ODRIVE_TX 1
#define PIN_SERIAL_ODRIVE_RX 0
#define PIN_ODRIVE_RESET 15

// Camera module pins
#define PIN_SERIAL_CAM_TX 10
#define PIN_SERIAL_CAM_RX 12

// Accelerometer pins (I2C)
#define PIN_I2C_SDA 20
#define PIN_I2C_SCL 21

// ADNS laser sensor pins (SPI)
#define PIN_ADNS_MOT 18
#define PIN_ADNS_SS 19
#define PIN_ADNS_MI 22
#define PIN_ADNS_MO 23
#define PIN_ADNS_SC 24

// Odrive
RomiOdrive romi_odrive(Serial1, PIN_ODRIVE_RESET);

// Camera module
Uart camera_serial( &sercom1, PIN_SERIAL_CAM_RX, PIN_SERIAL_CAM_TX, SERCOM_RX_PAD_3, UART_TX_PAD_2 ) ;
void SERCOM1_Handler() { camera_serial.IrqHandler(); }

//// Debug
int debug = 1;
#define debug_print(__x)  if (debug) Serial.print(__x)
#define debug_println(__x)  if (debug) Serial.println(__x)
#define debug_print_float(__x,__y)  if (debug) Serial.print(__x,__y)
#define debug_print_name_value(__s, __v) if (debug) { Serial.print(__s); Serial.print(" "); Serial.println(__v); }


/////////////////////////////////////////////////////////////////////////
// ## Brushless motor Turnigy Aerodrive SK3 - 5055-280kv (https://hobbyking.com/es_es/turnigy-aerodrive-sk3-5055-280kv-brushless-outrunner-motor.html)
// ## with Odrive controler (https://github.com/odriverobotics/ODrive)
// ## and AMT102 encoder 8192 ticks (https://www.cuidevices.com/product/resource/digikeypdf/amt10.pdf)

// Main pulley diameter is 57.5mm (we should check how much this varys depending on tension because of rubber compression)
// one turn = 50.6 * 3.1416 = 158.96mm = 15.9cm = 0.159m
// one meter of movement = 6.28 turns
// to calculate axis0.controller.config.vel_limit = maxSpeed * turnsPerMeter * ticksPerTurn

// FOR INPUT_RC
// Ramped velocity control seems to be the way to go:
// Set axis.controller.config.control_mode = CONTROL_MODE_VELOCITY_CONTROL.
// Set the velocity ramp rate (acceleration): axis.controller.config.vel_ramp_rate = 0.5 [turn/s^2]
// Activate the ramped velocity mode: axis.controller.config.input_mode = INPUT_MODE_VEL_RAMP.
// You can now control the velocity with axis.controller.input_vel = 1 [turn/s].

// FOR INPUT_SERIAL
// Trajectory control also ramped speed can be used when the serial command states a speed
// This mode lets you smoothly accelerate, coast, and decelerate the axis from one position to another. With raw position control, the controller simply tries to go to the setpoint as quickly as possible. Using a trajectory lets you tune the feedback gains more aggressively to reject disturbance, while keeping smooth motion.
enum { INPUT_RC, INPUT_SERIAL };
enum { ERROR_NONE, ERROR_ODRIVE };

struct State {
	int inputMode = INPUT_SERIAL;
	int error = ERROR_NONE;
};

// TODO remove this...
bool monitor = false;

State state;

// ## Remote control
const float RC_ALPHA = 0.5; // 0-10 - small values -> more smooth and big values more close to original data.
//const unsigned int RC_CALIBRATION[3] = {8828, 12164, 15200}; // Min, Middle and Max values to calibrate the remote
const int RC_CALIBRATION[3] = {1102, 1480, 1798}; // Min, Middle and Max values to calibrate the remote
const int RC_TRIGGER_THRESHOLD = (RC_CALIBRATION[1] - RC_CALIBRATION[0]) / 2;
volatile int rcTimerSpeed;
int prevRcSpeed;
volatile int rcSpeed;
int rcSmoothSpeed;
volatile bool rcUpdated;


// ## Endstops
volatile bool endStopFront = false;
volatile bool endStopBack = false;
uint32_t last_endStopFront_interrupt_time = 0;
uint32_t last_endStopBack_interrupt_time = 0;


// ## Leds
#include <FastLED.h>
#define NUM_LEDS 3
CRGB leds[NUM_LEDS];

// ## User button
#include "button.h"
void button_function(ButEvent butEvent);
Button button = Button(PIN_BUTTON, button_function);
void button_update() { button.update(); }

// TODO
// MOTOR-ODRIVE
// 	[X] Autodetect odrive firmware versions (2 for now)
// 	[ ] Adjust current limit during z index search
// 	[ ] Odrive calibration rutine (as automatic as possible)
// 	[ ] Fix bug that makes the motor rotate if RC is not present
// Serial control
// 	[ ] Comunication with the raspberry pi using RomiSerial
// 	Commands
// 		[X] Init
//		[ ] Homming
// 		[X] Go to position without acceleration control
//		[X] Go to position with acceleration control
// 		[X] Move at certain speed
// 		[X] stop
// 		[X] get/set config values
// Config system with
//	## General settings
// 		[ ] rc calibration values
// 		[ ] max speed
// 		[ ] acceleration values
// 	## Odrive specific settings
// 		[ ] Calibration data ?
// 		[ ] current limit
// 		[ ] Zindex search current limit
// 		[ ] check the odrive api to see what else.
// The Pi should manage solving the diferential between home geo position and the delta the motor reports.
// Implement commands to read state of
// 	[ ] speed
// 	[ ] current
// 	[ ] position
// 	[ ] batt level, voltage and charging status
// 	[ ] errors (implement error dictionary and reporting)
// Navigation
// - Implement detection of wind or external sources of inestability
// User interface
// - Led indicator
	// Think modes/events to indicate with the led
// 	* [ ] Mode indicator (Serial or RC)
// 	* [ ] When camera module is connected
// 	* [ ] When Raspberry pi wifi is OK
// 	* [ ] Charge state (lowBatt/ok/charging), maybe ok can be subdivided in levels (eg. 25-50/50-75/75-100)?
// - Design and implement button functions
// - Variant setup (I think it is not needed) test raspi serial to confirm
// - OpenOCD flashing



void setup()
{
	//// Debug USB serial port
	Serial.begin(115200);
	delay(2000);

	debug_print("Starting... ");

	//// Camera module
	// Serial port
	camera_serial.begin(115200);
	// Assign sercom functionality
	pinPeripheral(PIN_SERIAL_CAM_RX, PIO_SERCOM);
	pinPeripheral(PIN_SERIAL_CAM_TX, PIO_SERCOM);

	//// Led 
	FastLED.addLeds<NEOPIXEL, PIN_ADDR_LED>(leds, NUM_LEDS);
	fill_solid( leds, NUM_LEDS, CRGB::Blue);
	FastLED.show();

	//// Odrive 
	if ( 	!romi_odrive.begin() ||
		!romi_odrive.setState(RomiOdrive::AXIS_STATE_ENCODER_INDEX_SEARCH) ||
		!romi_odrive.isEncoderOK() ||
		!romi_odrive.setState(RomiOdrive::AXIS_STATE_CLOSED_LOOP_CONTROL)
	) {
		state.error = ERROR_ODRIVE;
		Serial.println("odrive ERROR");
	}

	//// RC setup
	attachInterrupt(digitalPinToInterrupt(PIN_RC_THR), rcTHR_irq, CHANGE);
	rcTimerSpeed = 0;
	rcUpdated = false;
	rcSpeed = rcSmoothSpeed = prevRcSpeed = RC_CALIBRATION[1];

	//// Endstops interrupt setup
	pinMode(PIN_ENDSTOP_BACK, INPUT_PULLUP);
	pinMode(PIN_ENDSTOP_FRONT, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(PIN_ENDSTOP_FRONT), endStopFront_irq, LOW);
	attachInterrupt(digitalPinToInterrupt(PIN_ENDSTOP_BACK), endStopBack_irq, LOW);

	//// Button
	attachInterrupt(digitalPinToInterrupt(button.pin), button_update, CHANGE);

	if (state.inputMode == INPUT_RC) switchToRCInput();
	else switchToSerialInput();

	debug_println("OK");
}

void stop()
{
	// This shoul stop the motor smoothly
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

void switchToSerialInput()
{
	Serial.println("Switching to Serial Input");
        stop();
        state.inputMode = INPUT_SERIAL;
}

void switchToRCInput()
{
        stop();
	debug_println("Switching to Remote control input...");

        state.inputMode = INPUT_RC;
}

void handleRemoteControl()
{
	if (rcUpdated) {

		// TODO
		// Clean warnings about types

		// Sanity check
		if (rcSpeed > RC_CALIBRATION[2]) rcSpeed = RC_CALIBRATION[2];
		else if (rcSpeed < RC_CALIBRATION[0]) rcSpeed = RC_CALIBRATION[0];

		rcUpdated = false;

		// Ignore changes that don't persist
		if (abs(rcSpeed - prevRcSpeed) > RC_TRIGGER_THRESHOLD) {
			prevRcSpeed = rcSpeed;
			return;
		}

		// Exponential smoothing
		rcSmoothSpeed = rcSmoothSpeed + (RC_ALPHA * (rcSpeed - rcSmoothSpeed) / 10);


		float speed;
		// Map the RC inputs to [-1, 0]
		if (rcSmoothSpeed < RC_CALIBRATION[1]) speed = map(rcSmoothSpeed, RC_CALIBRATION[0], RC_CALIBRATION[1], -1000, 0) / 1000.0f;
                // Or map the RC inputs to [0, 1]
		else speed = map(rcSmoothSpeed, RC_CALIBRATION[1], RC_CALIBRATION[2], 0, 1000) / 1000.0f;

                // Filter out small noise in the PWM around zero
                float delta1 = 0.05f;
                if (-delta1 < speed && speed < delta1) speed = 0.0f;

		/* debug_println(rcSpeed); */
		/* debug_print(","); */
		/* debug_print(rcSmoothSpeed); */
		/* debug_print(","); */
		/* debug_println((speed + 1) * 1480); */
		/* debug_print_name_value("speed: ", speed); */

		romi_odrive.moveAtNorm(speed);

		prevRcSpeed = rcSpeed;
	}
}

void handleSerialInput()
{
	// TODO 
	// Use RomiSerial for this
	// Port user-input commands here and handle both serial ports from here

	if (camera_serial.available() <= 0) return;
}

void handleUserInput()
{
	char c = Serial.read();

	switch(c) {

		case 'w': {
			switchToSerialInput();
			break;
		} case 'e': {
			// Encoder search z-index
			Serial.print("Encoder search z-index... ");
			if (romi_odrive.setState(RomiOdrive::AXIS_STATE_ENCODER_INDEX_SEARCH)) Serial.println("OK");
			else Serial.println("ERROR");
			break;

		} case 'c': {
			// Closed loop control (only if encoder is ready)
			if (romi_odrive.isEncoderOK()) {
				Serial.print("Entering closed loop control... ");
				if (romi_odrive.setState(RomiOdrive::AXIS_STATE_CLOSED_LOOP_CONTROL)) Serial.println("OK");
				else Serial.println("ERROR");
			} else {
				Serial.println("Please do the encoder index search first (remember that the motor will move during the search)");
			}
			break;

		} case 'z': {
			// Idle state
			Serial.print("Entering idle state...");
			if (romi_odrive.setState(RomiOdrive::AXIS_STATE_IDLE)) Serial.println("OK");
			else Serial.println("ERROR");
			break;

		} case 'i': {
			// Info
			Serial.print("Vbus voltage: ");
			Serial.println(romi_odrive.getInfo(RomiOdrive::INFO_VBUS_VOLTAGE));

			Serial.print("Current limit: ");
			Serial.println(romi_odrive.getInfo(RomiOdrive::INFO_CURRENT_LIMIT));

			Serial.print("Measured current: ");
			Serial.println(romi_odrive.getInfo(RomiOdrive::INFO_MEASURED_CURRENT));

			Serial.print("Position: ");
			Serial.println(romi_odrive.getInfo(RomiOdrive::INFO_POSITION));

			Serial.print("Axis error: ");
			Serial.println((int)romi_odrive.getInfo(RomiOdrive::INFO_AXIS_ERROR));

			Serial.print("Controller error: ");
			Serial.println((int)romi_odrive.getInfo(RomiOdrive::INFO_CONTROLLER_ERROR));

			Serial.print("Motor error: ");
			Serial.println((int)romi_odrive.getInfo(RomiOdrive::INFO_MOTOR_ERROR));

			Serial.print("Odrive version: ");
			Serial.println(romi_odrive.getInfo(RomiOdrive::INFO_VERSION));
			break;

		} case 's': {
			// Stop
			Serial.println("Stoping...");
			romi_odrive.stop();
			break;

		} case 'm': {
			// Move in turns
			float newPos = getFloat("Enter new position in turns: ");
			Serial.print("Moving to ");
			Serial.print(newPos);
			Serial.println(" turns");
			romi_odrive.moveTo(newPos);
			break;
		
		} case 't': {
			// Move in turns with acceleration ramps
			float newPos = getFloat("Enter new position in turns: ");
			Serial.print("Moving to ");
			Serial.print(newPos);
			Serial.println(" turns, with acceleration ramps");
			romi_odrive.moveToAcc(newPos);
			break;

		} case 'v': {
			
			// Move at specific velocity in turns/sec
			float vel = getFloat("Enter new velocity in turns/sec: ");
			Serial.print("Moving at ");
			Serial.print(vel);
			Serial.println(" turns per second");
			romi_odrive.moveAt(vel);
			break;

		} case 'y': {
			
			// Move at specific velocity in turns/sec with acceleration ramps 
			float vel = getFloat("Enter new velocity in turns/sec: ");
			Serial.print("Moving at ");
			Serial.print(vel);
			Serial.println(" turns per second");
			romi_odrive.moveAttAcc(vel);
			break;

		} case 'r': {
			// Odrive software reset
			Serial.println("Software reseting Odrive");
			romi_odrive.reset(false);
			break;

		 } case 'x': {
		 	// Odrive hardware reset
			Serial.println("Hardware reseting Odrive");
			romi_odrive.reset(true);
			break;

		} case 'u': {
			// monitor position and current
			monitor = !monitor;
			break;

		} case 'g': {
			// Get odrive parameter (eg. axis0.encoder.is_ready)
			char param[128];
			getString("Enter the parameter (eg. axis0.encoder.is_ready):", param);
			Serial.println(romi_odrive.readParameter(param));
			break;

		} case 'k': {
			// Input raw Odrive commands
			char command[128];
			getString("Enter Odrive RAW command:", command);
			romi_odrive.rawCommand(command);
			break;

		} case 'h': {
			// Print help
			print_help();
			break;
		
		} case 'b': {
			// Reset Feather
			NVIC_SystemReset();
			break;
		
		} default: break;
	}
}

void print_help()
{

	Serial.println("Commands: ");
	Serial.println("w - switch to serial input");
	Serial.println("e - Encoder index search");
	Serial.println("c - Enter closed_loop mode");
	Serial.println("z - Enter idle_state");
	Serial.println("m - Move to XX position in turns");
	Serial.println("t - Move to XX positionn in turns with acceleration ramps");
	Serial.println("v - Move at XX velocity (turns/sec)");
	Serial.println("y - Move at XX velocity (turns/sec) with acceleration ramps");
	Serial.println("s - Stop odrive");
	Serial.println("i - Get info");
	Serial.println("p - Get position");
	Serial.println("r - Odrive software reset");
	Serial.println("x - Odrive hardware reset");
	Serial.println("u - Toggle monitor mode: position,current");
	Serial.println("g - Get odrive parameter");
	Serial.println("k - Send raw command");	// https://docs.odriverobotics.com/api/odrive
	Serial.println("b - Reset (this) feather");
	Serial.println("h - Help");
}

void error() 
{
	Serial.println("Error!!");
}

bool odriveConfig()
{
	// TODO 
	// When user changes some settings in the feather stored config, we check if the odrive has the same values and sync only if needed.
	// Implement get/set all the supported config parameters from odrive

	return true;
}

void odriveCalibration()
{
	// TODO reimplement this with the Odrive object and test it
	// Make it work from zero (with a newly installed motor and no previous callibration)


	// in ASCII format
	//
	// MOTOR CALIBRATION
	// w axis0.motor.config.calibration_current 40

	// w axis0.requested_state 3 (AXIS_STATE_FULL_CALIBRATION_SEQUENCE)
	// Wait until..
	// w axis0.motor.config.pre_calibrated 1
	// w axis0.config.startup_closed_loop_control 1
	// 
	// ENCODER CALIBRATION
	// w axis0.encoder.config.use_index 1
	// w axis0.requested_state 6 (AXIS_STATE_ENCODER_INDEX_SEARCH)
	// Wait until... 
	// w axis0.requested_state 7 (AXIS_STATE_ENCODER_OFFSET_CALIBRATION)
	// Wait until...
	// CHECK IF IT WORKED
	// axis0.error → should be 0.
	// axis0.encoder.config.offset → This should print a number, like -326 or 1364.
	// axis0.motor.config.direction → This should print 1 or -1.
	// 
	// w axis0.encoder.config.pre_calibrated 1
	// w axis0.config.startup_encoder_index_search 1
	//
	// SAVE CONFIG
	// ss

	// NOTE
	// If there is too much mechanical load to do the callibration you can modify the current used in calibration:
	// w axis0.motor.config.calibration_current XX (default is 10A)

	// MOTOR TUNING
	// w axis0.motor.config.current_lim 40
	// w axis0.controller.config.vel_gain 0.00075 -> new driver: 0.125
	// w axis0.controller.config.pos_gain 130 -> new driver: 130
	// w axis0.controller.config.vel_integrator_gain 0.00375 -> new driver 0.8
	// ss

	// ADJUST VELOCITY LIMIT (TO AVOID OVERSPEED ERRORS)
	// w axis0.controller.config.vel_limit 50000
	// w axis0.controller.config.vel_limit_tolerance 0 ->
	// ss

}

void loop()
{
	if (endStopBack || endStopFront) {
		handleEndstop();
		return;
	}

	switch (state.inputMode) {

		case INPUT_RC:

			// if we receive an 's' via camera_serial we switch to serial control
			if (camera_serial.available() && camera_serial.read() == 115) {
				debug_println("Switching to serial control");
				switchToSerialInput();
			}

			handleRemoteControl();
			break;

		case INPUT_SERIAL:

			// As a safety measure, if someone is pulling the RC sticks, give the control back to the RC emitter.
			// TODO FIX: Sometimes it is jumping to RC control without RC connected
			if (
				abs(rcSpeed - RC_CALIBRATION[1]) > RC_TRIGGER_THRESHOLD &&
				rcSpeed != 0 &&
				rcSpeed < RC_CALIBRATION[2] &&
				rcSpeed > RC_CALIBRATION[0])
			{
				debug_println("Switching to remote control");

				switchToRCInput();

			} else {

				handleSerialInput();
			}

			break;
	}

	if (Serial.available()) handleUserInput();

	button.update();

	if (monitor) {
		Serial.print(romi_odrive.getInfo(RomiOdrive::INFO_POSITION));
		Serial.print(",");
		Serial.println(romi_odrive.getInfo(RomiOdrive::INFO_MEASURED_CURRENT));
	}
}

// Interrupt for endstops
void endStopFront_irq()
{
	if ((millis() - last_endStopFront_interrupt_time) > 50) endStopFront = true;
	last_endStopFront_interrupt_time = millis();
}
void endStopBack_irq()
{
	if ((millis() - last_endStopBack_interrupt_time) > 50) endStopBack = true;
	last_endStopBack_interrupt_time = millis();
}

// Interrupt for remote control throttle
void rcTHR_irq()
{
	if (digitalRead(PIN_RC_THR)) {

		rcTimerSpeed = micros();

	} else {

		if (rcTimerSpeed != 0) {
			// Record the pulse time
			rcSpeed = micros() - rcTimerSpeed;
			// Restart the timer
			rcTimerSpeed = 0;
			rcUpdated = true;
		}
	}
}

void button_function(ButEvent butEvent)
{
	switch(butEvent) {
		
		case BUT_PRESS:
			// This is always triggered, even on double click
			/* Serial.println("button pressed!"); */
			break;
		case BUT_RELEASE:
			/* Serial.println("button released!"); */
			break;
		case BUT_CLICK:
			// Click is detected on release, so if you need to react to press go with BUT_PRESS
			Serial.println("button click!");
			break;
		case BUT_DOUBLE_CLICK:
			Serial.println("button double click!");
			break;
		case BUT_LONG_CLICK:
			Serial.println("button long click!");
			break;
	}
}

void getString(const char* msg, char* buff)
{
	Serial.println(msg);

	char c;
	uint32_t i = 0;

	while (true) {
		if (Serial.available()) {
			c = Serial.read();
			Serial.print(c);
			if (c == '\n' || c == '\r') {
				buff[i] = '\0';
				break;
			}
			buff[i] = c;
			i++;
		}
	}
}

float getFloat(const char* msg)
{
	char theString[16];
	getString(msg, theString);
	return atof(theString);
}
