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
#ifndef __ROMISERIAL_CRC8_H
#define __ROMISERIAL_CRC8_H

#include <stdint.h>
#include <string.h>


namespace romiserial {

        class CRC8
        {
        protected:
                uint8_t crc_;
                static uint8_t table_[256];
                static bool table_initialized_;
        
        public:
        
                CRC8();
                virtual ~CRC8() = default;
        
                void init_table(uint8_t poly = 0x07);
                void do_init_table(uint8_t poly);
        
                void start(uint8_t start_value = 0) {
                        crc_ = start_value;
                }

                void update(const char c) {
                        update(uint8_t(c));
                }

                void update(uint8_t c) {
                        crc_ = table_[crc_ ^ c];
                }
        
                void update(const char *s, size_t len) {
                        while (len--) {
                                update((uint8_t) *s++);
                        }
                }
        
                void update(const char *s) {
                        update(s, strlen(s));
                }
        
                uint8_t finalize() {
                        return crc_;
                }
        
                uint8_t get() {
                        return crc_;
                }
        
                uint8_t compute(const char *s, size_t len) {
                        start();
                        for (size_t i = 0; i < len; i++)
                                update((uint8_t)s[i]);
                        return finalize();
                }
        };
}

#endif // __ROMISERIAL_CRC8_H
