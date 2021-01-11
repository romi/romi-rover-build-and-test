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
#ifndef _ROMI_COMMON_STATUS_H_
#define _ROMI_COMMON_STATUS_H_

#include "Status.h"
#include <string>
#include <algorithm>

namespace romi {
        
        class CommonStatus : public Status
        {
        protected:

                enum StatusCode { Initalizing, Ready, Busy, Error };

                StatusCode _code;
                std::string _message;
                double _progress;
                
        public:
                CommonStatus() : _code(Initalizing), _progress(-1.0) {}
                virtual ~CommonStatus() = default;

                bool is_ready() override {
                        return _code == Ready;
                }
                
                bool is_busy() override {
                        return _code == Busy;
                }
                
                bool is_failing() override {
                        return _code == Error;
                }
                
                void set_status(StatusCode code) {
                        _code = code;
                }
                
                bool has_message() override {
                        return _message.length() > 0;
                }
                
                const char *get_message() override {
                        return _message.c_str();
                }
                
                void set_message(const char *message) {
                        _message = message;
                }
                
                void clear_message() {
                        _message = "";
                }
                
                bool has_progess() override {
                        return _progress >= 0.0;
                }
                
                double get_progress() override {
                        return _progress;
                }
                
                void set_progress(double value) {
                        _progress = std::clamp(value, 0.0, 1.0);
                }
                
                void clear_progress() {
                        _progress = -1.0;
                }
        };
}

#endif // _ROMI_COMMON_STATUS_H_
