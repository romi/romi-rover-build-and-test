#include "RomiOdrive.h"

bool RomiOdrive::begin()
{
	// Do a hardware reset
	reset(true);

	// Start Serial port
	odrvSer.begin(config.serial_speed);
	uint32_t start_t = millis();
	while (!odrvSer) {
		if (millis() - start_t > config.serial_timeout) return false;
		delay(5);
	}

	state.serialStarted = true;
	setVersion();
	return true;
}
void RomiOdrive::reset(bool hardReset)
{
	if (hardReset) {
		digitalWrite(pin_reset, LOW);
		delay(100);
		digitalWrite(pin_reset, HIGH);
		delay(1000);
		state.serialStarted = false;
	} else {
		send("sr");
	}
}
void RomiOdrive::rawCommand(const char* rawcom)
{
	send(rawcom);
	Serial.println(getString());
}

bool RomiOdrive::setState(AxisState wichState)
{
	snprintf(obuff, OBSIZE, "w axis0.requested_state %i", wichState);
	send();

	uint32_t start_t = millis();

	int response;
	send("r axis0.current_state");
	response = getString().toInt();

	while (response != wichState) {
		if (millis() - start_t > config.idle_timeout) return false;
		delay(300);
		response = readParameter("axis0.current_state").toInt();
        }

	return true;
}
bool RomiOdrive::isEncoderOK()
{
	int response = 0;
	int counter = 0;
	while (!response) {
		send("r axis0.encoder.is_ready");
		response = getString().toInt();
		counter++;
		if (counter > 5) break;
	}
	return response;
}

float RomiOdrive::getInfo(Info wichInfo)
{
	// TODO check if we can get proper velocity with encoder.vel_estimate
	switch(wichInfo) {

		case INFO_VBUS_VOLTAGE:
			send("r vbus_voltage");
			break;

		case INFO_CURRENT_LIMIT:
			send("r axis0.motor.config.current_lim");
			break;

		case INFO_MEASURED_CURRENT:
			send("r axis0.motor.current_control.Iq_measured");
			break;

		case INFO_POSITION:
			send("r axis0.encoder.shadow_count");
			break;

		case INFO_AXIS_ERROR:
			send("r axis0.error");
			break;

		case INFO_CONTROLLER_ERROR:
			send("r axis0.controller.error");
			break;

		case INFO_MOTOR_ERROR:
			send("r axis0.motor.error");
			break;

		case INFO_VERSION:
			return config.odrvVer;

		default: return false;
	}
	float value = getString().toFloat();
	return value;
}
String RomiOdrive::readParameter(char* wichParameter)
{
	snprintf(obuff, OBSIZE, "r %s", wichParameter);
	send();
	return getString();
}
String RomiOdrive::readParameter(const char* wichParameter)
{
	snprintf(obuff, OBSIZE, "r %s", wichParameter);
	send();
	return getString();
}

void RomiOdrive::stop()
{
	// Find out in wich control mode we are
	String ctrlMoStr = readParameter("axis0.controller.config.control_mode");
	uint8_t ctrlMo = ctrlMoStr.toInt();

	if (ctrlMo == CONTROL_MODE_POSITION_CONTROL) {

		String currPosStr = readParameter("axis0.encoder.shadow_count");
		float currPos = currPosStr.toFloat();
		moveTo(currPos /= config.encoderTicks);

	} else if (ctrlMo == CONTROL_MODE_VELOCITY_CONTROL) moveAt(0);
}
void RomiOdrive::moveTo(float posInTurns)
{
	posInTurns *= ticksPerTurn();

	// Position control mode CONTROL_MODE_POSITION_CONTROL  —  3
	snprintf(obuff, OBSIZE, "w axis0.controller.config.control_mode %i", CONTROL_MODE_POSITION_CONTROL);
	send();

	// Input mode → INPUT_MODE_PASSTHROUGH  —  1
	snprintf(obuff, OBSIZE, "w axis0.controller.config.input_mode %i", INPUT_MODE_PASSTHROUGH);
	send();

	snprintf(obuff, OBSIZE, "w axis0.controller.input_pos %f", posInTurns);
	send();
}
void RomiOdrive::moveToAcc(float posInTurns)
{
	// Settings for trapezoidal movement can be adjusted with:
	// axis0.trap_traj.config.vel_limit
	// axis0.trap_traj.config.accel_limit
	// axis0.trap_traj.config.decel_limit
	// config.inertia

	posInTurns *= ticksPerTurn();

	// Position control mode = CONTROL_MODE_POSITION_CONTROL (3)
	snprintf(obuff, OBSIZE, "w axis0.controller.config.control_mode %i", CONTROL_MODE_POSITION_CONTROL);
	send();

	// Input mode = INPUT_MODE_TRAP_TRAJ (5) (not needed since it is already in command code https://github.com/odriverobotics/ODrive/blob/master/Firmware/communication/ascii_protocol.cpp#L263)
	snprintf(obuff, OBSIZE, "w axis0.controller.config.input_mode %i", INPUT_MODE_TRAP_TRAJ);
	send();
	
	snprintf(obuff, OBSIZE, "w axis0.controller.input_pos %f", posInTurns);
	send();
}
void RomiOdrive::moveAt(float velocity)
{

	// Velocity should be in turns/second
	velocity *= ticksPerTurn();

	//// Velocity control
	// Set axis.controller.config.control_mode = CONTROL_MODE_VELOCITY_CONTROL (2).
	snprintf(obuff, OBSIZE, "w axis0.controller.config.control_mode %i", CONTROL_MODE_VELOCITY_CONTROL);
	send();

	// Input mode = INPUT_MODE_PASSTHROUGH (1)
	snprintf(obuff, OBSIZE, "w axis0.controller.config.input_mode %i", INPUT_MODE_PASSTHROUGH);
	send();

	// Send velocity in turns/sec
	snprintf(obuff, OBSIZE, "w axis0.controller.input_vel %f", velocity);
	send();
}
void RomiOdrive::moveAtNorm(float velocity)
{
	float maxSpeedInTurns  = config.maxSpeed * config.turnsPerMeter;
	velocity = velocity * maxSpeedInTurns;

	// Velocity should be in turns/second
	velocity *= ticksPerTurn();

	//// Velocity control
	// Set axis.controller.config.control_mode = CONTROL_MODE_VELOCITY_CONTROL (2).
	snprintf(obuff, OBSIZE, "w axis0.controller.config.control_mode %i", CONTROL_MODE_VELOCITY_CONTROL);
	send();

	// Input mode = INPUT_MODE_PASSTHROUGH (1)
	snprintf(obuff, OBSIZE, "w axis0.controller.config.input_mode %i", INPUT_MODE_PASSTHROUGH);
	send();

	// Send velocity in turns/sec
	snprintf(obuff, OBSIZE, "w axis0.controller.input_vel %f", velocity);
	send();
}
void RomiOdrive::moveAttAcc(float velocity, float ramp_rate)
{

	// Velocity should be in turns/second
	velocity *= ticksPerTurn();

	//// Velocity control
	// Set axis.controller.config.control_mode = CONTROL_MODE_VELOCITY_CONTROL (2).
	snprintf(obuff, OBSIZE, "w axis0.controller.config.control_mode %i", CONTROL_MODE_VELOCITY_CONTROL);
	send();

	// Activate the ramped velocity mode
	// Set axis.controller.config.input_mode = INPUT_MODE_VEL_RAMP. (2)
	snprintf(obuff, OBSIZE, "w axis0.controller.config.input_mode %i", INPUT_MODE_VEL_RAMP);
	send();

	//// Ramped velocity control
	// Set the velocity ramp rate in turn/sec^2
	snprintf(obuff, OBSIZE, "w axis0.controller.config.vel_ramp_rate %f", ramp_rate);
	send();

	// Send velocity in turns/sec
	snprintf(obuff, OBSIZE, "w axis0.controller.input_vel %f", velocity);
	send();
}
void RomiOdrive::moveAttAccNorm(float velocity, float ramp_rate)
{
	float maxSpeedInTurns  = config.maxSpeed * config.turnsPerMeter;
	velocity = velocity * maxSpeedInTurns;

	// Velocity should be in turns/second
	velocity *= ticksPerTurn();

	Serial.println(velocity);

	//// Velocity control
	// Set axis.controller.config.control_mode = CONTROL_MODE_VELOCITY_CONTROL (2).
	snprintf(obuff, OBSIZE, "w axis0.controller.config.control_mode %i", CONTROL_MODE_VELOCITY_CONTROL);
	send();

	// Activate the ramped velocity mode
	// Set axis.controller.config.input_mode = INPUT_MODE_VEL_RAMP. (2)
	snprintf(obuff, OBSIZE, "w axis0.controller.config.input_mode %i", INPUT_MODE_VEL_RAMP);
	send();

	//// Ramped velocity control
	// Set the velocity ramp rate in turn/sec^2
	snprintf(obuff, OBSIZE, "w axis0.controller.config.vel_ramp_rate %f", ramp_rate);
	send();

	// Send velocity in turns/sec
	snprintf(obuff, OBSIZE, "w axis0.controller.input_vel %f", velocity);
	send();
}

void RomiOdrive::send(const char* msg)
{
	snprintf(obuff, OBSIZE, "%s", msg);
	send();
}
void RomiOdrive::send()
{
	odrvSer.println(obuff);
}
String RomiOdrive::getString()
{
	String response = "";
	uint32_t start_t = millis();
	while (true) {
		char c = odrvSer.read();
		if ((c == '\n' || c == '\r') && response.length() > 0) break;
		response.concat(c);
		while (!odrvSer.available()) {
			if (millis() - start_t >= config.serial_timeout) return response;
			delay(1);
		}
	}
	return response;
}

bool RomiOdrive::setVersion(float fallback)
{
	// Depending on the odrive board version some settings change
	// To find out your version you can check odrv0.fw_version_mayor odrv0.fw_version_minor odrv0.fw_version_revision
	// We have only tested 0.4.11 and 0.5.10 (these boards report 0.0 version) boards
	// More info on https://docs.odriverobotics.com/migration

	// Returns true if version detection succeed, false if setting fallback version

	// Try to get version
	send("r fw_version_major");
	String mayor = getString();
	send("r fw_version_minor");
	String minor = getString();

	String strVer = "0." + mayor + minor;
	config.odrvVer = strVer.toFloat();

	if (config.odrvVer <= 0) {
		config.odrvVer = fallback;
		return false;
	}

	return true;
}
int RomiOdrive::ticksPerTurn()
{
	if (config.odrvVer < 5) return config.encoderTicks;
	else return 1;
}

