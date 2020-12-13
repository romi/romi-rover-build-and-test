/*
  romi-rover

  Copyright (C) 2019 Sony Computer Science Laboratories
  Author(s) Peter Hanappe

  romi-rover is collection of applications for the Romi Rover.

  romi-rover is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see
  <http://www.gnu.org/licenses/>.

 */
#ifndef __ROMI_BRUSH_MOTORDRIVER_H
#define __ROMI_BRUSH_MOTORDRIVER_H

#include <stdexcept>
#include "IMotorDriver.h"
#include "IConfiguration.h"
#include "RomiSerialClient.h"

namespace romi {

        struct BrushMotorDriverSettings
        {
                int max_signal;
                bool use_pid;
                double kp, ki, kd;
                int dir_left;
                int dir_right;

                void parse(JSON &params) {
                        max_signal = (int) params.num("maximum_signal_amplitude");
                        use_pid = params.boolean("use_pid");
                        kp = params.get("pid").num("kp");
                        ki = params.get("pid").num("ki");
                        kd = params.get("pid").num("kd");
                        dir_left = (int) params.get("encoder_directions").num("left");
                        dir_right = (int) params.get("encoder_directions").num("right");
                }
        };
        
        class BrushMotorDriver : public IMotorDriver
        {
        protected:
                IRomiSerialClient &_serial;
                BrushMotorDriverSettings _settings;
                
                bool configure_controller(JSON &config, int steps,
                                          double max_revolutions_per_sec) {
                        
                        _settings.parse(config);

                        char command[100];
                        rprintf(command, 100, "C[%d,%d,%d,%d,%d,%d,%d,%d,%d]",
                                steps,
                                (int) (100.0 * max_revolutions_per_sec),
                                _settings.max_signal,
                                _settings.use_pid? 1 : 0,
                                (int) (1000.0 * _settings.kp),
                                (int) (1000.0 * _settings.ki),
                                (int) (1000.0 * _settings.kd),
                                _settings.dir_left,
                                _settings.dir_right);
                        
                        JSON response;
                        _serial.send(command, response);
                        return (response.num(0) == 0);
                }

                bool enable_controller() {
                        JSON response;
                        _serial.send("E[1]", response);
                        return (response.num(0) == 0);
                }

        public:

                BrushMotorDriver(IRomiSerialClient &serial,
                                 JSON &config,
                                 int encoder_steps,
                                 double max_revolutions_per_sec)
                        : _serial(serial) {
                        
                        if (!configure_controller(config, encoder_steps, max_revolutions_per_sec)
                            || !enable_controller()) {
                                throw std::runtime_error("BrushMotorDriver: "
                                                         "Initialization failed");
                        }
                }
                
                virtual ~BrushMotorDriver() override = default;

                bool stop() override {
                        JSON response;
                        _serial.send("X", response);
                        bool success = (response.num(0) == 0);
                        if (!success) {
                                r_err("BrushMotorDriver::stop: %s",
                                      response.str(1));
                        }
                        return success;
                }

                bool moveat(double left, double right) override {
                        bool success = false;
                        
                        if (left >= -1.0 && left <= 1.0
                            && right >= -1.0 && right <= 1.0) {
                                
                                int32_t ileft = (int32_t) (1000.0 * left);
                                int32_t iright = (int32_t) (1000.0 * right);
                                
                                char command[64];
                                rprintf(command, 64, "V[%d,%d]", ileft, iright);
                                
                                JSON response;
                                _serial.send(command, response);
                                success = (response.num(0) == 0);
                                if (!success) {
                                        r_err("BrushMotorDriver::moveat: %s",
                                              response.str(1));
                                }
                        }

                        return success;
                }

                bool get_encoder_values(double &left, double &right, double &timestamp) override {
                        JSON response;
                        _serial.send("e", response);
                        bool success = (response.num(0) == 0);
                        if (success) {
                                left = response.num(1);
                                right = response.num(2);
                                timestamp = response.num(3) / 1000.0;
                        } else {
                                r_err("BrushMotorDriver::get_encoder_values: %s",
                                      response.str(1));
                        }
                        return success;
                }
        };
}

#endif // __ROMI_BRUSH_MOTORDRIVER_H
