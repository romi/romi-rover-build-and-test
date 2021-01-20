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

#include <r.h>
#include "DefaultNavigation.h" 

namespace romi {

        double DefaultNavigation::compute_timeout(double distance, double speed)
        {
                double timeout = 0.0;
                if (speed != 0.0) {
                        double relative_speed = _settings.maximum_speed * speed;
                        double time = distance / fabs(relative_speed);
                        timeout = 1.5 * time;
                }
                return timeout;
        }

        bool DefaultNavigation::wait_travel(WheelOdometry &odometry, double distance,
                                     double timeout)
        {
                bool success = false;
                double left, right, timestamp;
                double start_time = clock_time();

                _stop = false;
                
                while (!_stop) {
                        
                        _driver.get_encoder_values(left, right, timestamp);
                        odometry.set_encoders(left, right, timestamp);

                        // r_debug("get_encoder_values: %f %f", left, right);

                        double x, y;
                        odometry.get_location(x, y);
                                
                        double distance_travelled = sqrt(x * x + y * y);

                        // r_debug("distance_travelled: %f, distance: %f", distance_travelled, distance);

                        if (distance_travelled >= distance) {
                                _driver.moveat(0, 0);
                                clock_sleep(0.100); // FIXME
                                success = true;
                                _stop = true;
                        }

                        double now = clock_time();
                        if (now - start_time >= timestamp) {
                                r_err("DefaultNavigation::wait_travel: time out (%f s)", timeout);
                                _driver.moveat(0, 0);
                                success = false;
                                _stop = true;
                        }

                        clock_sleep(0.010);
                }

                return success;
        }
        
        bool DefaultNavigation::do_move2(double distance, double speed)
        {
                bool success = false;
                double left, right, timestamp;
                
                if (_driver.get_encoder_values(left, right, timestamp)) {

                        // r_debug("get_encoder_values: %f %f", left, right);
                        
                        WheelOdometry odometry(_settings, left, right, timestamp);
                        
                        if (_driver.moveat(speed, speed)) {
                                
                                success = wait_travel(odometry, distance,
                                                        compute_timeout(distance, speed));
                                
                        } else {
                                r_err("DefaultNavigation::do_move2: moveat failed");
                        }
                } else {
                        r_err("DefaultNavigation::do_move2: get_encoder_values failed");
                }
                                        
                return success;
        }
        
        bool DefaultNavigation::do_move(double distance, double speed)
        {
                bool success = false;
                
                if (distance == 0.0) {
                        success = true;
                        
                } else if (speed != 0.0
                           && speed >= -1.0
                           && speed <= 1.0
                           && distance >= -50.0 // 50 meters max!
                           && distance <= 50.0) {

                        if (distance > 0.0 && speed > 0.0) {
                                // All is well, moving forward
                        } else {
                                // Moving backwards. Make sur the
                                // distance is positive and the speed
                                // negative.
                                distance = fabs(distance);
                                speed = -fabs(speed);
                        }
                        
                        if (_driver.stop()) {
                                
                                success = do_move2(distance, speed);
                                
                        } else {
                                r_err("DefaultNavigation::do_move: stop failed");
                        }
                        
                } else {
                        r_err("DefaultNavigation::do_move: invalid speed or distance: "
                              "speed=%f, distance=%f", speed, distance);
                }
                
                // A bit of a hack: in any case, make sure that the
                // rover comes to a complete standstill after 100 ms.
                _driver.stop();

                return success;
        }


        bool DefaultNavigation::moveat(double left, double right)
        {
                SynchronizedCodeBlock sync(_mutex);
                bool success = false;
                if (_status == MOVEAT_CAPABLE) 
                        success = _driver.moveat(left, right);
                else
                        r_warn("DefaultNavigation::moveat: still moving");
                return success;
        }
        
        bool DefaultNavigation::move(double distance, double speed)
        {
                bool success = false;
                {
                        SynchronizedCodeBlock sync(_mutex);
                        if (_status == MOVEAT_CAPABLE) {
                                _status = MOVING;
                        } else {
                                r_warn("DefaultNavigation::move: already moving");
                        }
                }
                        
                if (_status == MOVING)
                        success = do_move(distance, speed);
                        
                _status = MOVEAT_CAPABLE;
                        
                return success;
        }
        
        bool DefaultNavigation::stop()
        {
                // This flag should assure that we break out
                // the wait_travel loop.
                _stop = true;
                bool success = _driver.stop();
                _status = MOVEAT_CAPABLE;
                return success;
        }

        bool DefaultNavigation::pause_activity()
        {
                r_warn("*** DefaultNavigation::pause_activity NOT YET IMPLEMENTED");
                stop();
                return true;
        }
        
        bool DefaultNavigation::continue_activity()
        {
                r_warn("*** DefaultNavigation::continue_activity NOT YET IMPLEMENTED");
                return true;
        }
        
        bool DefaultNavigation::reset_activity()
        {
                stop();
                return true;
        }
}
