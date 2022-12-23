/*
  romi-rover

  Copyright (C) 2019-2020 Sony Computer Science Laboratories
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

#include <Arduino.h>
#include "IInputStream.h"
#include "IOutputStream.h"

#ifndef __ROMISERIAL_ARDUINOSERIAL_H
#define __ROMISERIAL_ARDUINOSERIAL_H

namespace romiserial {

        class ArduinoSerial : public IInputStream, public IOutputStream
        {
        protected:
                Stream& stream_;
        
        public:
        ArduinoSerial(Stream& stream) : stream_(stream) {}
        
                virtual ~ArduinoSerial() = default;

                void set_timeout(double seconds) override {
                        stream_.setTimeout((int) (seconds * 1000.0f));                
                }
        
                bool available() override {
                        return stream_.available() > 0;
                }
        
                bool read(char& c) override {
                        size_t n = stream_.readBytes(&c, 1);
                        return (n == 1)? true : false;
                }

                bool write(char c) {
                        return stream_.write(c) == 1;
                }
        };
}

#endif // __ROMISERIAL_ARDUINOSERIAL_H
