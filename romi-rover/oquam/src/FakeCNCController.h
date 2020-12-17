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
#ifndef _OQUAM_FAKE_CNC_CONTROLLER_HPP_
#define _OQUAM_FAKE_CNC_CONTROLLER_HPP_

namespace romi {

        class FakeCNCController : public ICNCController
        {
        protected:
                int32_t _pos[3];
                
        public:
                FakeCNCController() {
                        homing();
                }
                        
                virtual ~FakeCNCController() override = default;
                
                bool get_position(int32_t *pos) override {
                        for (int i = 0; i < 3; i++)
                                pos[i] = _pos[i];
                        return true;
                }

                bool homing() override {
                        for (int i = 0; i < 3; i++)
                                _pos[i] = 0;
                        return true;
                }
                
                bool synchronize(double timeout) {
                        return true;
                }
                
                bool move(int16_t millis, int16_t steps_x,
                          int16_t steps_y, int16_t steps_z) override {
                        _pos[0] += steps_x;
                        _pos[1] += steps_y;
                        _pos[2] += steps_z;
                        return true;
                }
                
                bool stop_execution() override {
                        return true;
                }
                
                bool continue_execution() override {
                        return true;
                }
                
                bool reset() override {
                        return homing();
                }
        };
}

#endif // _OQUAM_FAKE_CNC_CONTROLLER_HPP_
