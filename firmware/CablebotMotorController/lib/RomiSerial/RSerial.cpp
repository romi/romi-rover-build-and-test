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

#if !defined(ARDUINO)

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/poll.h>
#include <stdexcept>
#include <log.h>
#include <termios.h>
#include "RSerial.h"
#include <ClockAccessor.h>

namespace romiserial {

        RSerial::RSerial(const std::string& device, uint32_t baudrate, bool reset)
                : _device(device),
                  _fd(-1),
                  _timeout(0.1f),
                  _baudrate(baudrate),
                  _reset(reset),
                  _timeout_ms((int) (_timeout * 1000.0f))
        {
                open_device();
                configure_termios();
        }

        RSerial::~RSerial()
        {
                if (_fd >= 0) {
                        close(_fd);
                        _fd = -1;
                }
        }

        void RSerial::set_timeout(double seconds)
        {
                _timeout = seconds;
                _timeout_ms = (int) (_timeout * 1000.0);
        }

        bool RSerial::available()
        {
                //printf("RSerial::available %d\n", _timeout_ms);
                bool retval = false;
                struct pollfd fds[1];
                fds[0].fd = _fd;
                fds[0].events = POLLIN;

                int pollrc = poll(fds, 1, _timeout_ms);
                if (pollrc < 0) {
                        r_err("serial_read_timeout poll error %d on %s", errno, _device.c_str());
                
                } else if ((pollrc > 0) && (fds[0].revents & POLLIN)) {
                        retval = true;
                } else{
                        //r_warn("serial_read_timeout poll timed out on %s", _device.c_str());
                        //retval = 0;
                }
        
                return retval;
        }

        bool RSerial::read(char& c)
        {
                bool retval = true;
                ssize_t rc = ::read(_fd, &c, 1);
                if (rc != 1) {
                        retval = false;
                }
                return retval;
        }

        bool RSerial::poll_write()
        {
                bool retval = false;
                struct pollfd fds = { _fd, POLLOUT, 0 };

                int pollrc = poll(&fds, 1, _timeout_ms);
                if (pollrc < 0) {
                        r_err("serial_read_timeout poll error %d on %s", errno, _device.c_str());
                
                } else if ((pollrc > 0) && (fds.revents & POLLOUT)) {
                        retval = true;
                } else{
                        r_warn("serial_read_timeout poll timed out on %s", _device.c_str());
                }
                return retval;
        }

        bool RSerial::can_write()
        {
                if (_timeout_ms == 0)
                        return true;
                else 
                        return poll_write();
        }

        bool RSerial::write(char c)
        {
                bool success = false;
                // if (can_write()) {
                ssize_t m = ::write(_fd, &c, 1);
                if (m != 1)
                        r_err("RSerial::write");
                else
                        success = true;
                // }
                return success;
        }

        void RSerial::open_device()
        {
                _fd = open(_device.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
                if (_fd < 0) {
                        r_err("open_serial: error %d opening %s: %s",
                              errno, _device.c_str(), strerror(errno));
                        _fd = -1;
                        throw std::runtime_error("Failed to open the serial device");
                }
                // FIXME: the connection resets the Arduino and it can take
                // some time before the serial on the board is up and running. 
                rpp::ClockAccessor::GetInstance()->sleep(3.0);
        }

        void RSerial::configure_termios()
        {
                struct termios tty;
                speed_t speed_constant;

                switch (_baudrate) {
                case 9600: speed_constant = B9600; break;
                case 19200: speed_constant = B19200; break;
                case 38400: speed_constant = B38400; break;
                case 57600: speed_constant = B57600; break;
                case 115200: speed_constant = B115200; break;
                case 230400: speed_constant = B230400; break;
                case 460800: speed_constant = B460800; break;
                default:
                        r_warn("open_serial: Unknown baudrate. Standard values are "
                               "9600, 19200, 38400, 57600, 115200, 230400, 460800. "
                               "I will try anyway.");
                        speed_constant = (speed_t) _baudrate;
                        break;
                        //throw std::runtime_error("Invalid baudrate");
                }

                get_termios(&tty);
        
                tty.c_cflag |= CLOCAL | CREAD;
                tty.c_cflag &= (tcflag_t) ~CSIZE;
                tty.c_cflag |= CS8;                 /* 8-bit characters */
                tty.c_cflag &= (tcflag_t) ~PARENB;  /* no parity bit */
                tty.c_cflag &= (tcflag_t) ~CSTOPB;  /* only need 1 stop bit */
                tty.c_cflag &= (tcflag_t) ~CRTSCTS; /* no hardware flowcontrol */
                tty.c_cflag &= (tcflag_t) ~HUPCL;
                if (_reset)
                        tty.c_cflag |= HUPCL;

        
                tty.c_lflag &= (tcflag_t) ~ICANON;   /* No canonical input (line editing) */
                tty.c_lflag &= (tcflag_t) ~(ECHO | ECHOE | ECHONL); /* No echo */
                tty.c_lflag &= (tcflag_t) ~ISIG;     /* Don't send signals */
                tty.c_lflag &= (tcflag_t) ~IEXTEN;   /* No input processing */

        
                tty.c_iflag &= (tcflag_t) ~IGNCR;    /* Preserve carriage return */
                tty.c_iflag &= (tcflag_t) ~INPCK;    /* Disable input parity checking. */
                tty.c_iflag &= (tcflag_t) ~INLCR;    /* Don't translate NL to CR */
                tty.c_iflag &= (tcflag_t) ~ICRNL;    /* Don't translate CR to NL */
                tty.c_iflag &= (tcflag_t) ~(IXON | IXOFF | IXANY); /* no SW flowcontrol */

        
                tty.c_oflag = 0; /* no remapping; no delays; no post-processing */

                /* Use for non-canonical input.
                 * See http://unixwiz.net/techtips/termios-vmin-vtime.html
                 */
                tty.c_cc[VMIN]  = 1; /* Wait for one character */
                tty.c_cc[VTIME] = 0; /* No timing: calls are blocking */
        
                cfsetspeed(&tty, speed_constant);

                set_termios(&tty);
        }

        void RSerial::set_termios(struct termios *tty)
        {
                // Flush port, then apply attributes
                tcflush(_fd, TCIOFLUSH);

                if (tcsetattr(_fd, TCSANOW, tty) != 0) {
                        r_err("Could not set terminal attributes for %s", _device.c_str());
                        throw std::runtime_error("tcsetattr failed");
                }
        }

        void RSerial::get_termios(struct termios *tty)
        {
                memset(tty, 0, sizeof(struct termios));
                if (tcgetattr(_fd, tty) != 0) {
                        r_err("Could not get terminal attributes for %s", _device.c_str());
                        throw std::runtime_error("tcgetattr failed");
                }
        }
}

#endif
