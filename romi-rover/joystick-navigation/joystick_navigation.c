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
#include <rcom.h>
#include "joystick_navigation.h"

int _drive = 0;
int _sent_stop = 0;
double _speed = 0.0;
double _direction = 0.0;

messagelink_t *get_messagelink_motorcontroller();

int joystick_navigation_init(int argc, char **argv)
{
        return 0;
}

void joystick_navigation_cleanup()
{
}

static void parse_drive(json_object_t obj)
{
        const char *state = json_object_getstr(obj, "state");
        _drive = rstreq(state, "pressed");
}

static void parse_speed(json_object_t obj)
{
        double speed = json_object_getnum(obj, "value");
        if (speed >= -1.0 && speed <= 1.0)
                _speed = speed;
}

static void parse_direction(json_object_t obj)
{
        double direction = json_object_getnum(obj, "value");
        if (direction >= -1.0 && direction <= 1.0)
                _direction = direction;
}

static void parse_json(json_object_t obj)
{
        if (json_isobject(obj)) {
                const char *type = json_object_getstr(obj, "type");
                if (rstreq(type, "axis")) {
                        int axis = (int) json_object_getnum(obj, "axis");
                        if (axis == 4) 
                                parse_speed(obj);
                        if (axis == 3) 
                                parse_direction(obj);
                } else if (rstreq(type, "button")) {
                        int button = (int) json_object_getnum(obj, "button");
                        if (button == 5) 
                                parse_drive(obj);
                }
        }
}

static void parse_data(datalink_t *link, data_t *input)
{
        json_object_t json = datalink_parse(link, input);
        parse_json(json);
        json_unref(json);
}

static void send_moveat_command(messagelink_t *controller,
                               double left, double right)
{
        int left_i = (int) (1000.0 * left);
        int right_i = (int) (1000.0 * right);
        messagelink_send_f(controller, "{'command':'moveat','speed':[%d,%d]}",
                           left_i, right_i);
        _sent_stop = (left_i == 0 && right_i == 0);
}

static void send_motor_command(messagelink_t *controller)
{
        double left = _speed + _direction;
        double right = _speed - _direction;
        send_moveat_command(controller, left, right);
}

static void send_stop_command(messagelink_t *controller)
{
        if (_sent_stop == 0) {
                send_moveat_command(controller, 0.0, 0.0);
        }
}

void joystick_navigation_onevent(void *userdata, datalink_t *link,
                                 data_t *input, data_t *output)
{
        messagelink_t *controller = get_messagelink_motorcontroller();
        
        parse_data(link, input);
        
        if (_drive) {
                send_motor_command(controller);
                                
        } else {
                send_stop_command(controller);
        }
}

