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
#ifndef _MOTORCONTROLLER_AZHOO_H
#define _MOTORCONTROLLER_AZHOO_H

#include "IMotorController.h"
#include "IArduino.h"
#include "SpeedEnvelope.h"
#include "PIController.h"

class MotorController : public IMotorController
{
public:
        static const uint32_t kDefaultUpdateInterval;

        /*
          States: [created] -> [set-up] -> [configured] -> [enabled] <-> [disabled]
        */
        enum MotorControllerState {
                kCreated = 0,
                kSetUp,
                kConfigured,
                kEnabled,
                kDisabled
        };
        
        IArduino& arduino_;

public:
        // FIXME: made public for unit testing
        PIController left_controller_;
        PIController right_controller_;
        SpeedEnvelope left_speed_envelope_;
        SpeedEnvelope right_speed_envelope_;
        uint32_t interval_;
        uint32_t last_time_;
        MotorControllerState state_;

        bool is_set_up();
        bool is_configured();
        bool is_enabled();
        bool is_disabled();
        
protected:
        
        void init_encoders(uint16_t encoder_steps,
                           int8_t left_direction,
                           int8_t right_direction);

        void init_speed_envelope(double max_speed, double max_acceleration);

        void init_pi_controllers(int16_t kp_numerator, int16_t kp_denominator,
                                 int16_t ki_numerator, int16_t ki_denominator,
                                 int16_t max_amplitude);

        

public:
        
        MotorController(IArduino& arduino, uint32_t interval_millis);
        ~MotorController() override = default;
        
        void setup() override;
        bool configure(MotorControllerConfiguration& config) override;
        bool enable() override;
        bool disable() override;
        
        // Set the new target angular speeds. The left and right
        // values are the angular speed in revolutions/s, multiplied
        // by 1000. So for an angular speed of 0.5 rev/s, the value is
        // 500. Returns true is all went well, and false if the
        // initialization wasn't completed.
        bool set_target_speeds(int16_t left, int16_t right) override;

        void get_target_speeds(int16_t& left, int16_t& right) override;
        void get_current_speeds(int16_t& left, int16_t& right) override;
        void get_measured_speeds(int16_t& left, int16_t& right) override;
        
        // Returns true if an update was performed, false otherwise. 
        bool update() override;

        // Stops the rover immediately.
        void stop() override;

        void get_encoders(int32_t& left, int32_t& right, uint32_t& time) override;

        PIController& left_controller() override;
        PIController& right_controller() override;
        SpeedEnvelope& left_speed_envelope() override;
        SpeedEnvelope& right_speed_envelope() override;
        
        void do_update() {
                int16_t left_speed = left_speed_envelope_.update();
                int16_t right_speed = right_speed_envelope_.update();
                left_controller_.update(left_speed);
                right_controller_.update(right_speed);
        }
};

#endif // _MOTORCONTROLLER_AZHOO_H
