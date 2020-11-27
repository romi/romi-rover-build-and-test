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
#include <romi.h>
#include <memory>
#include "Image.h"
#include "weeder.h"

static int initialized = 0;
static cnc_range_t _range = {{0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}};

messagelink_t *get_messagelink_cnc();

namespace romi {
        class Weeder {


        public:

                bool configure_pipeline(json_object_t config);
                
                
        };
}

static int status_error(json_object_t reply, membuf_t *message)
{
        const char *status = json_object_getstr(reply, "status");
        if (status == NULL) {
                membuf_printf(message, "Invalid status message");
                return -1;
                
        } else if (rstreq(status, "ok")) {
                return 0;
                
        } else if (rstreq(status, "error")) {
                const char *s = json_object_getstr(reply, "message");
                membuf_printf(message, "%s", s);
                return -1;
        }
        return -1; // ??
}

static int parse_range(json_object_t config)
{
        int err = 0;
        
        json_object_t cnc = json_object_get(config, "cnc");
        if (json_isobject(cnc)) {
                
                json_object_t r = json_object_get(cnc, "range");
                if (!json_isarray(r)) {
                        r_err("Invalid range in configuration");
                        err = -1;
                } else {
                        err = cnc_range_parse(&_range, r);
                        if (err == 0) {
                                r_info("range set to: x[%.3f,%.3f], "
                                       "y[%.3f,%.3f], z[%.3f,%.3f]",
                                       _range.x[0], _range.x[1],
                                       _range.y[0], _range.y[1],
                                       _range.z[0], _range.z[1]);
                        } // else: error message alreay printed by cnc_range_parse()
                }
        } else {
                r_err("Couldn't find CNC configuration");
                err = -1;
        }
        json_unref(config);
        return err;
}

static int parse_configuration()
{
        int r = -1;
        json_object_t config;
        
        config = client_get("configuration", "/");
        if (json_isobject(config)) {

                if (parse_range(config) == 0) {
                        r = 0;
                }
                
        } else {
                r_err("Failed to download the configuration");                
        }
        
        return r;
}

static int init()
{
        int r = 0;
        
        if (!initialized) {
                if (parse_configuration() == 0) {
                        initialized = 1;
                } else {
                        r = -1;
                }
        } else {
                r = 0;
        }
        
        return r;
}

int weeder_init(int argc, char **argv)
{
        for (int i = 0; i < 10; i++) {
                if (init() == 0)
                        return 0;
                r_err("workspace initialization failed: attempt %d/10", i);
                clock_sleep(0.2);
        }
        
        r_err("failed to initialize the workspace");
        return -1;
}

void weeder_cleanup()
{
}

static int move_away_arm()
{
        json_object_t reply = json_null();
        messagelink_t *cnc = get_messagelink_cnc();
        membuf_t *message = new_membuf();
        
        // move weeding tool up
        reply = messagelink_send_command_f(cnc, "{\"command\": \"moveto\", \"z\": 0}");
        if (status_error(reply, message)) {
                r_err("moveto returned error: %s", membuf_data(message));
                delete_membuf(message);
                json_unref(reply);
                return -1;
        }
        json_unref(reply);
        
        // move weeding tool to the end of the workspace
        reply = messagelink_send_command_f(cnc, "{\"command\": \"moveto\", "
                                           "\"x\": 0, \"y\": %.4f}",
                                           _range.y[1]);
        if (status_error(reply, message)) {
                r_err("moveto returned error: %s", membuf_data(message));
                delete_membuf(message);
                json_unref(reply);
                return -1;
        }
        delete_membuf(message);
        json_unref(reply);
        return 0;
}

static void return_to_base()
{
        messagelink_t *cnc = get_messagelink_cnc();
        json_object_t reply = json_null();
        membuf_t *message = new_membuf();
                
        // stop the spindle
        reply = messagelink_send_command_f(cnc, "{\"command\": \"spindle\", \"speed\": 0}");
        if (status_error(reply, message)) {
                r_err("spindle returned error: %s", membuf_data(message));
        }
        json_unref(reply);
        
        // home
        reply = messagelink_send_command_f(cnc, "{\"command\": \"homing\"}");
        if (status_error(reply, message)) {
                r_err("homing returned error: %s", membuf_data(message));
        }
        
        json_unref(reply);
        delete_membuf(message);
}

static int send_path(list_t *path, double z0, membuf_t *message)
{
        messagelink_t *cnc = get_messagelink_cnc();
        json_object_t reply = json_null();

        // move weeding tool up
        reply = messagelink_send_command_f(cnc, "{\"command\": \"moveto\", \"z\": 0}");
        if (status_error(reply, message)) {
                r_err("moveto returned error: %s", membuf_data(message));
                json_unref(reply);
                return -1;
        }
        json_unref(reply);

        // move to the first point on the path
        point_t *p = list_get(path, point_t);
        reply = messagelink_send_command_f(cnc,
                                           "{\"command\": \"moveto\", "
                                           "\"x\": %.4f, \"y\": %.4f}",
                                           p->x, p->y);
        if (status_error(reply, message)) {
                r_err("moveto returned error: %s", membuf_data(message));
                json_unref(reply);
                return -1;
        }
        json_unref(reply);

        // start the spindle
        reply = messagelink_send_command_f(cnc, "{\"command\": \"spindle\", \"speed\": 1}");
        if (status_error(reply, message)) {
                r_err("spindle returned error: %s", membuf_data(message));
                json_unref(reply);
                return -1;
        }
        json_unref(reply);

        // move weeding tool down
        reply = messagelink_send_command_f(cnc, "{\"command\": \"moveto\", \"z\": %.4f}",
                                           z0);
        if (status_error(reply, message)) {
                r_err("moveto returned error: %s", membuf_data(message));
                json_unref(reply);
                return -1;
        }
        json_unref(reply);

        // convert path to json
        membuf_t *buf = new_membuf();
        membuf_printf(buf, "{\"command\": \"travel\", \"path\": [");
        for (list_t *l = path; l != NULL; l = list_next(l)) {
                point_t *p = list_get(l, point_t);
                double x = p->x;
                double y = p->y;
                double z = z0;

                if (!cnc_range_is_valid(&_range, x, y, z)) {
                        
                        r_err("Point: %.3f, %.3f, %.3f: "
                              "*** Out of range ***: (%.3f, %.3f), (%.3f, %.3f), (%.3f, %.3f)",
                              x, y, z,
                              _range.x[0], _range.x[1],
                              _range.y[0], _range.y[1],
                              _range.z[0], _range.z[1]);
                
                        
                        if (cnc_range_error(&_range, x, y, z) < 0.01) {
                                // Small error: clip
                                // TODO: call cnc_range to do this
                                if (x < _range.x[0])
                                        x = _range.x[0];
                                if (x > _range.x[1])
                                        x = _range.x[1];
                                if (y < _range.y[0])
                                        y = _range.y[0];
                                if (y > _range.y[1])
                                        y = _range.y[1];
                                if (z < _range.z[0])
                                        z = _range.z[0];
                                if (z > _range.z[1])
                                        z = _range.z[1];
                        } else {
                                r_err("Computed point out of range: %.3f, %.3f, %.3f", x, y, z);
                                delete_membuf(buf);
                                return -1;
                        } 
                } else {
                        r_err("Point: %.3f, %.3f, %.3f: "
                              "In range: (%.3f, %.3f), (%.3f, %.3f), (%.3f, %.3f)",
                              x, y, z,
                              _range.x[0], _range.x[1],
                              _range.y[0], _range.y[1],
                              _range.z[0], _range.z[1]);
                }
                
                membuf_printf(buf, "[%.4f,%.4f,%.4f]", x, y, z);
                if (list_next(l))
                        membuf_printf(buf, ",");
        }
        membuf_printf(buf, "]}");
        membuf_append_zero(buf);

        /* r_debug("path 1"); */
        /* r_debug("path: %s", membuf_data(buf)); */
        /* printf("%s\n", membuf_data(buf)); */
        /* r_debug("path 2"); */
        
        // send path to cnc
        messagelink_send_text(cnc, membuf_data(buf), membuf_len(buf));
        reply = messagelink_read(cnc);
        
        if (status_error(reply, message)) {
                r_err("travel returned error: %s", membuf_data(message));
                delete_membuf(buf);
                json_unref(reply);
                return -1;
        }
        json_unref(reply);
        delete_membuf(buf);

        // move weeding tool up
        reply = messagelink_send_command_f(cnc, "{\"command\": \"moveto\", \"z\": 0}");
        if (status_error(reply, message)) {
                r_err("moveto returned error: %s", membuf_data(message));
                json_unref(reply);
                return -1;
        }
        json_unref(reply);

        // stop spindle
        reply = messagelink_send_command_f(cnc, "{\"command\": \"spindle\", \"speed\": 0}");
        if (status_error(reply, message)) {
                r_err("spindle returned error: %s", membuf_data(message));
                json_unref(reply);
                return -1;
        }
        json_unref(reply);

        // move to the top-left corner
        reply = messagelink_send_command_f(cnc,
                                           "{\"command\": \"moveto\", "
                                           "\"x\": 0, \"y\": %.4f}",
                                           _range.y[1]);
        if (status_error(reply, message)) {
                r_err("moveto returned error: %s", membuf_data(message));
                json_unref(reply);
                return -1;
        }

        json_unref(reply);
        return 0;
}

static unique_ptr<Image> grab_camera(membuf_t *message)
{
        // Move away the arm before taking a picture of the workspace.
        if (move_away_arm() != 0) {
                r_err("Failed to move the arm");
                membuf_printf(message, "Failed to move the arm");
                return NULL;
        }

        // Get the camera image
        response_t *response = NULL;
        int err = client_get_data("camera", "camera.jpg", &response);
        if (err == 0) {
                membuf_t *body = response_body(response);
                
                make_unique<Image> retval;
                retval = make_unique<Image>((const unsigned char *)membuf_data(body),
                                            membuf_len(body));
                delete_response(response);
                
        } else {
                r_err("Failed to obtain the camera image");
        }
        
        return retval;
}

static int hoe(membuf_t *message)
{
        image_t *camera = NULL;
        list_t *path = NULL;
        fileset_t *fileset;
        int err = -1;
        json_object_t w;
        Image image;

        try {
        
                // Grab the camera image
                grab_camera(image, message);

                // Run the pipeline to compute the path
                path = pipeline_run(pipeline, camera, fileset, message);
                if (path == NULL)
                        goto cleanup;

                // Execute the path
                if (json_object_has(command, "skip-execution")) {
                        r_info("Skipping execution of path)");
                        err = 0;
                } else {
                        err = send_path(path, z0, message);
                }
        
                r_info("Finished executing path");

                err = 0;
                
        } catch (const std::exception& e) {
                
        }

        return_to_base();

        
        return err;
}

int weeder_onhoe(void *userdata,
                 messagelink_t *link,
                 json_object_t command,
                 membuf_t *message)
{
}
