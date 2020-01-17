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

#ifndef _OQUAM_ACTION_H_
#define _OQUAM_ACTION_H_

#ifdef __cplusplus
extern "C" {
#endif

enum {
        ACTION_WAIT = 0,
        ACTION_MOVE,
        ACTION_DELAY,
        ACTION_TRIGGER
};

typedef struct _action_t action_t;

struct _action_t {
        unsigned char type;
        // move
        double p[3];
        double v;
        // delay
        double delay;
        // trigger
        void *callback;
        void *userdata;
        int arg;
};

action_t *new_action(int type);
action_t *action_clone(action_t *a);
action_t *new_wait();
action_t *new_delay(double delay);
action_t *new_move(double x, double y, double z, double v);
action_t *new_trigger(void *callback, void *userdata, int arg);
void delete_action(action_t *action);

#ifdef __cplusplus
}
#endif

#endif // _OQUAM_ACTION_H_
