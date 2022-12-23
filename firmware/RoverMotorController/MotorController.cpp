/*
  Romi motor controller for brushed motors

  Copyright (C) 2021 Sony Computer Science Laboratories
  Author(s) Peter Hanappe

  MotorController is free software: you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as
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
#include "MotorController.h"

const uint32_t MotorController::kDefaultUpdateInterval = 20;

MotorController::MotorController(IArduino& arduino, uint32_t interval_millis)
        : arduino_(arduino),
          left_controller_(arduino.left_encoder(),
                           arduino.left_pwm()),
          right_controller_(arduino.right_encoder(),
                            arduino.right_pwm()),
          left_speed_envelope_(),
          right_speed_envelope_(),
          interval_(interval_millis),
          last_time_(0),
          state_(kCreated)
{
        //left_controller_.debug_ = true; // TODO
}

void MotorController::setup()
{
        last_time_ = arduino_.milliseconds();
        state_ = kSetUp;
}

bool MotorController::is_set_up()
{
        return state_ == kSetUp;
}

bool MotorController::is_configured()
{
        return state_ == kConfigured;
}

bool MotorController::is_enabled()
{
        return state_ == kEnabled;
}

bool MotorController::is_disabled()
{
        return state_ == kDisabled;
}

bool MotorController::enable()
{
        bool success = false;
        stop();
        if (is_configured() || is_disabled()) {
                // TODO: power on?
                state_ = kEnabled;
                success = true;
        }
        return success;
}

bool MotorController::disable()
{
        bool success = true;
        stop();
        if (is_enabled()) {
                // TODO: power off?
                state_ = kDisabled;
                success = true;
        }
        return success;
}

bool MotorController::configure(MotorControllerConfiguration& config)
{
        bool success = false;
        
        // It's okay to change the configuration when the machine is
        // disabled.
        if (is_set_up() || is_configured() || is_disabled()) {
                init_encoders(config.encoder_steps,
                              config.left_direction,
                              config.right_direction);
                init_speed_envelope(config.max_speed, config.max_acceleration);
                init_pi_controllers(config.kp_numerator, config.kp_denominator,
                                    config.ki_numerator, config.ki_denominator,
                                    config.max_amplitude);
                if (is_set_up()) {
                        state_ = kConfigured;
                }
                success = true;
                
        } else {
                // If we receive an configure command while the
                // machine is already enabled - and possibly moving -
                // stop and disable the rover. This should never
                // happen and if it does, it needs checking.
                disable();
        }
        
        return success;
}

void MotorController::init_encoders(uint16_t encoder_steps,
                          int8_t left_direction,
                          int8_t right_direction)
{
        arduino_.init_encoders(encoder_steps, left_direction, right_direction);
        left_controller_.update_encoder_values(interval_ / 1000.0);
        right_controller_.update_encoder_values(interval_ / 1000.0);
}

void MotorController::init_speed_envelope(double max_speed, double max_acceleration)
{
        left_speed_envelope_.init(max_speed,
                                  max_acceleration,
                                  interval_ / 1000.0);
        right_speed_envelope_.init(max_speed,
                                   max_acceleration,
                                   interval_ / 1000.0);
}

void MotorController::init_pi_controllers(int16_t kp_numerator, int16_t kp_denominator,
                                int16_t ki_numerator, int16_t ki_denominator,
                                int16_t max_amplitude)
{
        left_controller_.init(kp_numerator, kp_denominator,
                              ki_numerator, ki_denominator,
                              max_amplitude);
        right_controller_.init(kp_numerator, kp_denominator,
                               ki_numerator, ki_denominator,
                               max_amplitude);
}

bool MotorController::set_target_speeds(int16_t left, int16_t right)
{
        bool updated = false;
        if (is_enabled()) {
                left_speed_envelope_.set_target(left);
                right_speed_envelope_.set_target(right);
                updated = true;
        }
        return updated;
}

void MotorController::get_target_speeds(int16_t& left, int16_t& right)
{
        left = left_speed_envelope_.get_target();
        right = right_speed_envelope_.get_target();
}
        
void MotorController::get_current_speeds(int16_t& left, int16_t& right)
{
        left = left_speed_envelope_.get_current();
        right = right_speed_envelope_.get_current();
}
        
void MotorController::get_measured_speeds(int16_t& left, int16_t& right)
{
        left = left_controller_.get_speed();
        right = right_controller_.get_speed();
}

void MotorController::get_encoders(int32_t& left, int32_t& right, uint32_t& time)
{
        left = arduino_.left_encoder().get_position();
        right = arduino_.right_encoder().get_position();
        time = arduino_.milliseconds();
}

bool MotorController::update()
{
        bool updated = false;
        
        if (is_enabled()) {
                uint32_t now = arduino_.milliseconds();
                uint32_t dt = now - last_time_;
        
                if (dt >= interval_) {
                        do_update();
                        last_time_ = now;
                        updated = true;
                }
        }
        
        return updated;
}

void MotorController::stop()
{
        left_speed_envelope_.stop();
        right_speed_envelope_.stop();
        left_controller_.stop();
        right_controller_.stop();
}

PIController& MotorController::left_controller()
{
        return left_controller_;
}
        
PIController& MotorController::right_controller()
{
        return right_controller_;
}
        
SpeedEnvelope& MotorController::left_speed_envelope()
{
        return left_speed_envelope_;
}
        
SpeedEnvelope& MotorController::right_speed_envelope()
{
        return right_speed_envelope_;
}
