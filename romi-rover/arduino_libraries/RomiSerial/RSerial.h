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

#include "IInputStream.h"
#include "IOutputStream.h"

#ifndef __R_SERIAL_H
#define __R_SERIAL_H

class RSerial : public IInputStream, public IOutputStream
{
protected:
        std::string _device;
        int _fd;
        float _timeout;
        int _baudrate;
        bool _reset;
        int _timeout_ms;
        
        void open_device();
        void configure_termios();
        void set_termios(struct termios *tty);
        void get_termios(struct termios *tty);
        bool can_write();
        bool poll_write();

public:
        RSerial(const char *device, int baudrate, bool reset);        
        virtual ~RSerial();

        void set_timeout(float seconds) override;
        int available() override;
        int read() override;
        int readline(std::string &line) override;
        size_t write(char c) override;
        size_t print(const char *s) override;
        size_t println(const char *s) override;
};

#endif // __R_SERIAL_H
