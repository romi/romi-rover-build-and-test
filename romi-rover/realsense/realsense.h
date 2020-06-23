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
#ifndef _ROMI_REALSENSE_H_
#define _ROMI_REALSENSE_H_

#ifdef __cplusplus
extern "C" {
#endif

int realsense_init(int argc, char **argv);
void realsense_cleanup();
void realsense_broadcast();
void realsense_still(void *data, request_t *request, response_t *response);

streamer_t *get_streamer_camera();
streamer_t *get_streamer_depthsensor();
messagelink_t *get_messagelink_logger();

#ifdef __cplusplus
}
#endif

#endif // _ROMI_REALSENSE_H_
