#include <romi.h>

#include "weeding.h"
#include "weeder.h"

enum {
        WEEDING_METHOD_BOUSTROPHEDON,
        WEEDING_METHOD_QUINCUNX
};

static workspace_t workspace;
static int initialized = 0;
static int z0;
static double quincunx_threshold = 1.0;
static database_t *db = NULL;

messagelink_t *get_messagelink_cnc();

database_t *get_database()
{
        return db;
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
        log_debug("trying to configure the workspace dimensions");
        
        json_object_t config = client_get("configuration", "weeder");
        if (json_falsy(config) || !json_isobject(config)) {
                log_err("failed to load the configuration");
                json_unref(config);
                return -1;
        }

        json_object_t w = json_object_get(config, "workspace");
        if (json_falsy(w) || !json_isarray(w)) {
                log_err("failed to load the workspace configuration");
                json_unref(config);
                return -1;
        }
        
        double theta = json_array_getnum(w, 0);
        double x0 = json_array_getnum(w, 1);
        double y0 = json_array_getnum(w, 2);
        double width = json_array_getnum(w, 3);
        double height = json_array_getnum(w, 4);
        double width_mm = json_array_getnum(w, 5);
        double height_mm = json_array_getnum(w, 6);
        if (isnan(theta) || isnan(x0) || isnan(y0)
            || isnan(width) || isnan(height)
            || isnan(width_mm) || isnan(height_mm)) {
                log_err("invalid workspace values");
                json_unref(config);
                return -1;
        }

        workspace.theta = theta;
        workspace.x0 = x0;
        workspace.y0 = y0;
        workspace.width = width;
        workspace.height = height;
        workspace.width_mm = width_mm;
        workspace.height_mm = height_mm;

        log_debug("workspace: theta %.2f, x0 %.2f, y0 %.2f, "
                  "width %.2f px / %.2f mm, height %.2f px / %.2f mm", 
                  theta, x0, y0, width, width_mm, height, height_mm);

        double z = json_object_getnum(config, "z0");
        if (isnan(z)) {
                log_err("failed to load the z0 setting");
                json_unref(config);
                return -1;
        }
        if (z > 0 || z < -1000) {
                log_err("Invalid z0 setting: z0 !in [-1000,0]");
                json_unref(config);
                return -1;
        }
        z0 = (int) z;

        /* const char *d = json_object_getstr(config, "datadir"); */
        /* if (d == NULL) { */
        /*         log_err("Invalid value for 'datadir'"); */
        /*         json_unref(config); */
        /*         return -1; */
        /* } */
        
        /* if (set_weeding_data_directory(d) != 0) { */
        /*         json_unref(config); */
        /*         return -1; */
        /* } */

        double threshold = json_object_getnum(config, "quincunx_threshold");
        if (isnan(threshold) || threshold < 0 || threshold > 1000.0) {
                log_err("Invalid value for 'quincunx_threshold'");
                json_unref(config);
                return -1;
        }
        quincunx_threshold = threshold;
        
        json_unref(config);
        return 0;
}

static int init_database()
{
        if (db != NULL)
                return 0;

        int err = -1;
        json_object_t path = json_null();
        path = client_get("configuration", "fsdb.directory");
        if (json_isstring(path)) {
                log_info("using configuration file for directory: '%s'",
                         json_string_value(path));
                db = new_database(json_string_value(path));
                err = (db == NULL);
        } else {
                log_err("Failed to obtain a valid camera-recorder.directory setting");
        }
        json_unref(path);
        return err;
}

static int init()
{
        if (initialized)
                return 0;
        
        if (init_workspace() != 0)
                return -1;
        
        if (init_database() != 0)
                return -1;
        
        initialized = 1;
        return 0;
}

int weeder_init(int argc, char **argv)
{
        for (int i = 0; i < 10; i++) {
                if (init() == 0)
                        return 0;
                log_err("workspace initialization failed: attempt %d/10", i);
                clock_sleep(0.2);
        }
        
        log_err("failed to initialize the workspace");
        return -1;
}

void weeder_cleanup()
{
        /* free_weeding_data_directory(); */
        if (db)
                delete_database(db);
}

void weeder_onmessage_cnc(void *userdata,
                          messagelink_t *link,
                          json_object_t message)
{
        json_print(message, 0);
}

static int move_away_arm()
{
        json_object_t reply;
        messagelink_t *cnc = get_messagelink_cnc();
        membuf_t *message = new_membuf();
        
        // move weeding tool up
        reply = messagelink_send_command_f(cnc, "{\"command\": \"moveto\", \"z\": 0}");
        if (status_error(reply, message)) {
                log_err("moveto returned error: %s", membuf_data(message));
                delete_membuf(message);
                return -1;
        }
        // move weeding tool to the end of the workspace
        reply = messagelink_send_command_f(cnc, "{\"command\": \"moveto\", "
                                           "\"x\": 0, \"y\": %d}",
                                           workspace.height_mm);
        if (status_error(reply, message)) {
                log_err("moveto returned error: %s", membuf_data(message));
                delete_membuf(message);
                return -1;
        }
        delete_membuf(message);
        return 0;
}

static list_t *compute_quincunx(membuf_t *message, double start_time)
{
        // Move away the arm before taking a picture of the workspace.
        if (move_away_arm() != 0) {
                log_err("Failed to move the arm");
                membuf_printf(message, "Failed to move the arm");
                return NULL;
        }

        membuf_t *buf = new_membuf();
        int err = client_get_data("camera", "camera.jpg", buf);
        if (err != 0) {
                log_err("Failed to obtain the camera image");
                membuf_printf(message, "Failed to obatin the camera image");
                delete_membuf(buf);
                return NULL;
        }

        log_info("Done grabbing image: %f s", clock_time() - start_time);

        image_t *image = image_load_from_mem(membuf_data(buf), membuf_len(buf));
        if (image == NULL) {
                log_err("Failed to decompress the image");
                membuf_printf(message, "Failed to decompress the image");
                delete_membuf(buf);
                return NULL;
        }
        log_info("Done decompressing image: %f s", clock_time() - start_time);

        list_t *path = compute_path(image, &workspace, 0.3f, z0, quincunx_threshold,
                                    start_time);
        
        log_info("Done computing path: %f s", clock_time() - start_time);
        
        delete_membuf(buf);
        delete_image(image);
        return path;
}

static int send_path(list_t *path, membuf_t *message)
{
        messagelink_t *cnc = get_messagelink_cnc();
        json_object_t reply;

        // move weeding tool up
        reply = messagelink_send_command_f(cnc, "{\"command\": \"moveto\", \"z\": 0}");
        if (status_error(reply, message)) {
                log_err("moveto returned error: %s", membuf_data(message));
                return -1;
        }

        // start the spindle
        reply = messagelink_send_command_f(cnc, "{\"command\": \"spindle\", \"speed\": 1}");
        if (status_error(reply, message)) {
                log_err("spindle returned error: %s", membuf_data(message));
                return -1;
        }

        // convert path to json
        membuf_t *buf = new_membuf();
        membuf_printf(buf, "{\"command\": \"travel\", \"path\": [");
        for (list_t *l = path; l != NULL; l = list_next(l)) {
                point_t *p = list_get(l, point_t);
                membuf_printf(buf, "[%d,%d,%d]", (int) p->x, (int) p->y, (int) p->z);
                if (list_next(l)) membuf_printf(buf, ",");
        }
        membuf_printf(buf, "]}");
        membuf_append_zero(buf);

        log_debug("path: %s", membuf_data(buf));
        
        // send path to cnc
        messagelink_send_text(cnc, membuf_data(buf), membuf_len(buf));
        reply = messagelink_read(cnc);
        
        if (status_error(reply, message)) {
                log_err("travel returned error: %s", membuf_data(message));
                delete_membuf(buf);
                return -1;
        }
        delete_membuf(buf);

        // stop spindle
        reply = messagelink_send_command_f(cnc, "{\"command\": \"spindle\", \"speed\": 0}");
        if (status_error(reply, message)) {
                log_err("spindle returned error: %s", membuf_data(message));
                return -1;
        }

        return 0;
}

static int hoe(int method, membuf_t *message)
{
        list_t *path = NULL;
        int err;
        double start_time = clock_time();

        if (method == WEEDING_METHOD_QUINCUNX) {
                path = compute_quincunx(message, start_time);
        } else {
                path = compute_boustrophedon(&workspace, z0);
        }
                
        if (path == NULL) {
                log_err("Failed to compute the path");
                membuf_printf(message, "Failed to compute the path");
                return -1;
        }
        
        err = send_path(path, message);

        log_info("Finished executing path: %f s", clock_time() - start_time);
        
        for (list_t *l = path; l != NULL; l = list_next(l)) {
                point_t *p = list_get(l, point_t);
                delete_point(p);
        }
        delete_list(path);

        return err;
}

int weeder_onhoe(void *userdata,
                 messagelink_t *link,
                 json_object_t command,
                 membuf_t *message)
{
	log_debug("weeder_onhoe");

        if (init_workspace() != 0) {
                membuf_printf(message, "workspace not intialized");
                return -1;
        }

        int weeding_method;

	const char *method = json_object_getstr(command, "method");
	if (method == NULL || rstreq(method, "boustrophedon")) {
                log_debug("Doing boustrophedon.");
                weeding_method = WEEDING_METHOD_BOUSTROPHEDON;
	} else if (rstreq(method, "quincunx")) {
                log_debug("Doing quincunx weeding method.");
                weeding_method = WEEDING_METHOD_QUINCUNX;
	} else {
                log_err("Bad weeding method: %s", method);
                membuf_printf(message, "Bad weeding method: %s", method);
                return -1;
	}

        return hoe(weeding_method, message); 
}
