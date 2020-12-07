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

#include "weeding.h"
#include "weeder.h"
#include "otsu.h"
#include "svm.h"
#include "quincunx.h"
#include "profiling.h"

enum {
        WEEDING_METHOD_BOUSTROPHEDON,
        WEEDING_METHOD_QUINCUNX
};

static workspace_t workspace;
static int initialized = 0;
static double z0;
static double quincunx_threshold = 1.0;
static char *directory = NULL;
static database_t *db = NULL;
static scan_t *scan = NULL;
static pipeline_t *pipeline = NULL;
static cnc_range_t _range = {{0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}};


messagelink_t *get_messagelink_cnc();
messagelink_t *get_messagelink_db();

database_t *get_database()
{
        return db;
}

static void broadcast_log(void *userdata, const char* s)
{
        messagelink_t *get_messagelink_logger();
        messagelink_t *log = get_messagelink_logger();
        if (log)
                messagelink_send_str(log, s);
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

static int init_workspace()
{
        r_debug("trying to configure the workspace dimensions");
        
        json_object_t config = client_get("configuration", "weeder");
        if (json_falsy(config) || !json_isobject(config)) {
                r_err("failed to load the configuration");
                json_unref(config);
                return -1;
        }

        json_object_t w = json_object_get(config, "workspace");
        
        if (workspace_parse(&workspace, w) != 0) {
                json_unref(config);
                return -1;
        }

        double z = json_object_getnum(config, "z0");
        if (isnan(z)) {
                r_err("failed to load the z0 setting");
                json_unref(config);
                return -1;
        }
        if (z > 0 || z < -1) {
                r_err("Invalid z0 setting: z0 !in [-1,0]");
                json_unref(config);
                return -1;
        }
        z0 = z;
        r_info("z0 %.4f", z0);

        double threshold = json_object_getnum(config, "quincunx_threshold");
        if (isnan(threshold) || threshold < 0 || threshold > 1000.0) {
                r_err("Invalid value for 'quincunx_threshold'");
                json_unref(config);
                return -1;
        }
        quincunx_threshold = threshold;
        
        json_unref(config);
        return 0;
}

static int get_range()
{
        int err = 0;
        
        json_object_t config = client_get("configuration", "cnc");
        if (json_isobject(config)) {
                
                json_object_t r = json_object_get(config, "range");
                if (!json_isarray(r)) {
                        r_err("Invalid range in configuration");
                        err = -1;
                } else {
                        err = cnc_range_parse(&_range, r);
                        if (err == 0) {
                                r_info("range set to: x[%.3f,%.3f], y[%.3f,%.3f], z[%.3f,%.3f]",
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

static void broadcast_db_message(void *userdata,
                                 database_t *db,
                                 const char *event,
                                 const char *scan_id,
                                 const char *fileset_id,
                                 const char *file_id,
                                 const char *mimetype,
                                 const char *localfile)
{
        r_debug("broadcast_db_message: %s, %s, %s, %s, %s",
                event, scan_id, fileset_id, file_id, mimetype);
        messagelink_t *bus = get_messagelink_db();
        if (file_id)
                messagelink_send_f(bus,
                                   "{\"event\": \"%s\", "
                                   "\"source\": \"weeder\", "
                                   "\"scan\": \"%s\", "
                                   "\"fileset\": \"%s\", "
                                   "\"file\": \"%s\", "
                                   "\"mimetype\": \"%s\", "
                                   "\"localfile\": \"%s\"}",
                                   event, scan_id, fileset_id,
                                   file_id, mimetype, localfile);
        else if (fileset_id)
                messagelink_send_f(bus,
                                   "{\"event\": \"%s\", "
                                   "\"source\": \"weeder\", "
                                   "\"scan\": \"%s\", "
                                   "\"fileset\": \"%s\"}",
                                   event, scan_id, fileset_id);
        else messagelink_send_f(bus,
                                "{\"event\": \"%s\", "
                                "\"source\": \"weeder\", "
                                "\"scan\": \"%s\"}",
                                event, scan_id);
}

static char *get_fsdb_directory()
{
        json_object_t fsdb = client_get("db", "directory");
        if (!json_isobject(fsdb)) {
                r_err("Failed to obtain the session information from fsdb");
                return NULL;
        }
        const char *dir = json_object_getstr(fsdb, "db");
        if (dir == NULL) {
                r_err("The fsdb's session information does not contain the 'db' field");
                return NULL;
        }
        char *copy = r_strdup(dir);
        json_unref(fsdb);
        return copy;
}

static char *get_directory()
{
        if (directory != NULL)
                return r_strdup(directory);
        else
                return get_fsdb_directory();
}

static int init_pipeline()
{
        if (pipeline != NULL)
                return 0;

        /* float a[] = {-0.04152306,  0.0472676 , -0.00709334}; */
        /* float b = 0.66209273; */
        float a[] = {-0.08727648,  0.14512232, -0.05679426};
        float b = -1.75098531;
        segmentation_module_t *segmentation = new_svm_module(a, b);
        /* segmentation_module_t *segmentation = new_otsu_module(); */
        double width = fabs(_range.x[1] - _range.x[0]);

        r_debug("init_pipeline: width=%f", width);
        r_debug("quincunx pixels/meter: %f", workspace.width / width / 3.0);
        
        path_module_t *path = new_quincunx_module(0.3, 0.3, 0.1, 0.05, quincunx_threshold,
                                                  workspace.width / width / 3.0); // FIXME!
        if (segmentation == NULL || path == NULL)
                return -1;
            
        pipeline = new_pipeline(&workspace, &_range, segmentation, path);
        if (pipeline == NULL)
                return -1;

        return 0;
}

static int init_database()
{
        if (db != NULL) 
                return 0;
        
        char *dir = get_directory();
        if (dir == NULL)
                return -1;
        db = new_database(dir);
        r_free(dir);

        database_load(db);
        database_set_listener(db, broadcast_db_message, NULL);
        
        scan = database_get_scan(db, "weeding");
        if (scan == NULL)
                scan = database_new_scan(db, "weeding");
        if (scan == NULL)
                return -1;
        
        return 0;
}

static int init()
{
        if (initialized)
                return 0;
        if (init_workspace() != 0)
                return -1;
        if (get_range() != 0)
                return -1;
        if (init_pipeline() != 0)
                return -1;        
        initialized = 1;
        return 0;
}

int weeder_init(int argc, char **argv)
{
        r_log_set_writer(broadcast_log, NULL);
        
        if (argc == 2) {
                // FIXME: needs more checking
                directory = r_strdup(argv[1]);
                r_info("using command line argument for directory: '%s'", directory);
        }
        
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
        if (db)
                delete_database(db);
        if (pipeline)
                delete_pipeline(pipeline);
}

void weeder_onmessage_cnc(void *userdata,
                          messagelink_t *link,
                          json_object_t message)
{
        json_print(message, 0);
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

static int32 _set_param(const char* key, json_object_t value, void* data)
{
        membuf_t *message = (membuf_t *) data;
        
        if (rstreq(key, "skip-execution"))
                return 0;
        if (rstreq(key, "method"))
                return 0;
        
        if (pipeline_set(pipeline, key, value, message) < 0)
                return -1;
        
        return 0;
}

static image_t *grab_camera(membuf_t *message)
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
        if (err != 0) {
                r_err("Failed to obtain the camera image");
                membuf_printf(message, "Failed to obatin the camera image");
                return NULL;
        }

        membuf_t *body = response_body(response);
        image_t *image = image_load_from_mem((const unsigned char *)membuf_data(body), membuf_len(body));
        if (image == NULL) {
                r_err("Failed to decompress the image");
                membuf_printf(message, "Failed to decompress the image");
                delete_response(response);
                return NULL;
        }
        
        delete_response(response);
        
        return image;
}

static int hoe(int method, json_object_t command, membuf_t *message)
{
        image_t *camera = NULL;
        list_t *path = NULL;
        fileset_t *fileset;
        int err = -1;
        json_object_t w;
        
        // Set the parameters of the pipeline
        if (json_object_foreach(command, _set_param, message) != 0)
                goto cleanup;
        
        // Grab the camera image
        camera = grab_camera(message);
        if (camera == NULL)
                goto cleanup;

        // Create a fileset to store the intermediary data
        fileset = create_fileset(scan, NULL);
        if (fileset == NULL)
                goto cleanup;

        w = workspace_to_json(&workspace);
        err = fileset_set_metadata(fileset, "workspace", w);
        json_unref(w);
        if (err)
                goto cleanup;

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

cleanup:
        if (err != 0)
                return_to_base();
        
        for (list_t *l = path; l != NULL; l = list_next(l)) {
                point_t *p = list_get(l, point_t);
                delete_point(p);
        }
        delete_list(path);
        delete_image(camera);

        return err;
}

int weeder_onhoe(void *userdata,
                 messagelink_t *link,
                 json_object_t command,
                 membuf_t *message)
{
	r_debug("weeder_onhoe");

        if (init_workspace() != 0) {
                membuf_printf(message, "Workspace not intialized");
                return -1;
        }
        if (init_database() != 0) {
                membuf_printf(message, "Database not intialized");
                return -1;
        }

        int weeding_method;

	const char *method = json_object_getstr(command, "method");
	if (method == NULL || rstreq(method, "boustrophedon")) {
                r_debug("Doing boustrophedon.");
                weeding_method = WEEDING_METHOD_BOUSTROPHEDON;
	} else if (rstreq(method, "quincunx")) {
                r_debug("Doing quincunx weeding method.");
                weeding_method = WEEDING_METHOD_QUINCUNX;
	} else {
                r_err("Bad weeding method: %s", method);
                membuf_printf(message, "Bad weeding method: %s", method);
                return -1;
	}

        return hoe(weeding_method, command, message); 
}

static int loosen(int method, json_object_t command, membuf_t *message)
{
        return 0;
}

int weeder_onloosen(void *userdata,
                    messagelink_t *link,
                    json_object_t command,
                    membuf_t *message)
{
	r_debug("weeder_onloosen");

        if (init_workspace() != 0) {
                membuf_printf(message, "Workspace not intialized");
                return -1;
        }
        if (init_database() != 0) {
                membuf_printf(message, "Database not intialized");
                return -1;
        }

        int weeding_method;

	const char *method = json_object_getstr(command, "method");
	if (method == NULL || rstreq(method, "boustrophedon")) {
                r_debug("Doing boustrophedon.");
                weeding_method = WEEDING_METHOD_BOUSTROPHEDON;
	} else if (rstreq(method, "quincunx")) {
                r_debug("Doing quincunx weeding method.");
                weeding_method = WEEDING_METHOD_QUINCUNX;
	} else {
                r_err("Bad weeding method: %s", method);
                membuf_printf(message, "Bad weeding method: %s", method);
                return -1;
	}

        return loosen(weeding_method, command, message); 
}
