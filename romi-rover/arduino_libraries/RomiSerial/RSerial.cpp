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
#include <r.h>
#include "RSerial.h"

RSerial::RSerial(const char *device, int baudrate, bool reset)
        : _device(device),
          _fd(-1),
          _timeout(0.1f),
          _baudrate(baudrate),
          _reset(reset)
{
        _timeout_ms = (int) (_timeout * 1000.0f);
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

void RSerial::set_timeout(float seconds)
{
        _timeout = seconds;
        _timeout_ms = (int) (_timeout * 1000.0f);
}

int RSerial::available()
{
        //printf("RSerial::available %d\n", _timeout_ms);
        int retval = -1;
        struct pollfd fds[1];
        fds[0].fd = _fd;
        fds[0].events = POLLIN;

        int pollrc = poll(fds, 1, _timeout_ms);
        if (pollrc < 0) {
                r_err("serial_read_timeout poll error %d on %s", errno, _device.c_str());
                retval = 0;
                
        } else if ((pollrc > 0) && (fds[0].revents & POLLIN)) {
                retval = 1;
        } else{
                //r_warn("serial_read_timeout poll timed out on %s", _device.c_str());
                retval = 0;
        }
        
        return retval;
}

int RSerial::read()
{
        char c;
        int retval = -1;
        ssize_t rc = ::read(_fd, &c, 1);
        if (rc == 1) {
                retval = c;
                //printf("%c 0x%02x\n", c, (int) c);
        } 
        return retval;
}

int RSerial::readline(std::string &line)
{
        enum { READ, NL, DONE };
        
        int state = READ;
        
        line = "";
        while (available()) {
                int c = read();
                switch (state) {
                case READ:
                        line += (char) c;
                        if (c == '\r')
                                state = NL;
                        break;
                case NL:
                        if (c == '\n') {
                                line += (char) c;
                                state = DONE;
                        } else {
                                state = READ;
                        }
                        break;
                }
                if (state == DONE)
                        break;
        }
        return 0;
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

size_t RSerial::write(char c)
{
        size_t n = 0;
        if (can_write()) {
                ssize_t m = ::write(_fd, &c, 1);
                //printf("%c %d\n", c, (int) m);
                if (m == 1)
                        n = 1;
                if (m < 0)
                        r_err("serial_put");
        }
        return n;
}

size_t RSerial::print(const char *s)
{
        size_t n = 0;
        while (*s) {
                if (write(*s++) != 1)
                        break;
                n++;
        }
        return n;
}

size_t RSerial::println(const char *s)
{        
        size_t len = strlen(s);
        size_t n = print(s);
        if (n == len) {
                size_t m = print("\r\n");
                if (m > 0)
                        n += m;
        }
        return n;
}

void RSerial::open_device()
{
        _fd = open_wrapper(_device.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
        if (_fd < 0) {
                r_err("open_serial: error %d opening %s: %s",
                      errno, _device.c_str(), strerror(errno));
                _fd = -1;
                throw std::runtime_error("Failed to open the serial device");
        }
        // FIXME: the connection resets the Arduino and it can take
        // some time before the serial on the board is up and running. 
        clock_sleep(3.0); 
}

void RSerial::configure_termios()
{
        struct termios tty;
        int speed_constant;

        switch (_baudrate) {
        case 9600: speed_constant = B9600; break;
        case 19200: speed_constant = B19200; break;
        case 38400: speed_constant = B38400; break;
        case 57600: speed_constant = B57600; break;
        case 115200: speed_constant = B115200; break;
        default:
                r_err("open_serial: only the following speeds are valid: "
                      "9600, 19200, 38400, 57600, 115200");
                throw std::runtime_error("Invalid baudrate");
        }

        get_termios(&tty);

        tty.c_cflag |= CLOCAL | CREAD;
        tty.c_cflag &= ~CSIZE;
        tty.c_cflag |= CS8;         /* 8-bit characters */
        tty.c_cflag &= ~PARENB;     /* no parity bit */
        tty.c_cflag &= ~CSTOPB;     /* only need 1 stop bit */
        tty.c_cflag &= ~CRTSCTS;    /* no hardware flowcontrol */
        tty.c_cflag &= ~HUPCL;
        if (_reset)
                tty.c_cflag |= HUPCL;

        tty.c_lflag |= ICANON | ISIG;  /* canonical input */
        tty.c_lflag &= ~(ECHO | ECHOE | ECHONL | IEXTEN);

        tty.c_iflag &= ~IGNCR;   /* Preserve carriage return (= don't ignore CR)*/
        //tty.c_iflag |= IGNCR;  /* preserve carriage return */
        tty.c_iflag &= ~INPCK;   /* Disable input parity checking. */
        tty.c_iflag &= ~INLCR;   /* Don't translate newline to carriage return */
        tty.c_iflag &= ~ICRNL;   /* Don't translate carriage return to newline */
        //tty.c_iflag &= ~(IUCLC | IMAXBEL);  (not in POSIX)
        tty.c_iflag &= ~(IXON | IXOFF | IXANY);   /* no SW flowcontrol */

//    tty.c_oflag &= ~OPOST;
        tty.c_oflag = 0;

//
//    // ToDo: Whats going on here, Shouldn't these values be != into the c_cflag?
//    // cflag: 8n1 (8bit, no parity, 1 stopbit) CLOCAL: ignore modem controls CREAD: enable reading CS8: 8-bit characters
//    tty.c_cflag = CLOCAL | CREAD | CS8;
//
//    // disable hang-up-on-close to avoid reset
//    if(reset)
//        tty.c_cflag |= HUPCL;
////    tty.c_cflag &= ~HUPCL;
//
//    // lflag: no signaling chars; no echo; ...
//    tty.c_lflag = ICANON;    // enable canonical processing
//
//    // iflag
//    // no parity check; no break processing; don't map NL to CR,
//    // CR to NL, uppercase to lowercase; ring bell; shut off
//    // xon/xoff ctrl; ...
//    tty.c_iflag = IGNCR;    // ignore carriage-return '\r'
//
//    // oflag: no remapping; no delays; no post-processing
//    tty.c_oflag = 0;

        // Use for non-canonical input
        /* tty.c_cc[VMIN]  = 0; */
        /* tty.c_cc[VTIME] = 5; */

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

#endif
