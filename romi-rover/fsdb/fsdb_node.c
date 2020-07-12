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
#include "fsdb_node.h"

static database_t *db = NULL;

streamer_t *get_streamer_camera();
messagehub_t *get_messagehub_db();
static list_t *parse_uri(const char *uri);

static void broadcast_log(void *userdata, const char* s)
{
        messagelink_t *get_messagelink_logger();
        messagelink_t *log = get_messagelink_logger();
        if (log)
                messagelink_send_str(log, s);
}

int fsdb_node_init(int argc, char **argv)
{
        json_object_t path = json_null();
        const char *dir = NULL;
        char buffer[1024];
        
        r_log_set_writer(broadcast_log, NULL);

        if (argc >= 2) {
                dir = argv[1];
        } else if (app_get_session() != NULL) {
                rprintf(buffer, sizeof(buffer), "%s/db", app_get_session());
                dir = buffer;
        } else {
                path = client_get("configuration", "fsdb/directory");
                if (!json_isstring(path)) {
                        r_err("The value of fsdb.directory is not a string");
                        json_unref(path);
                        return -1;
                }
                dir = json_string_value(path);
        }

        if (dir == NULL) {
                r_err("No directory specified");
                json_unref(path);
                return -1;
        }

        db = new_database(dir);
        if (db == NULL) {
                r_err("Failed to create the database");
                json_unref(path);
                return -1;
        }
        if (database_load(db) != 0) {
                r_err("Failed to load the database");
                delete_database(db);
                json_unref(path);
                return -1;
        }
        //database_print(db);
        
        json_unref(path);
        
        return 0;
}

void fsdb_node_cleanup()
{
        if (db)
                delete_database(db);
}

// FIXME: use fs.h's path_break()
static list_t *parse_uri(const char *uri)
{
        list_t *list = NULL;
        if (uri[0] != '/')
                return NULL;
        int size = strlen(uri);
        const char *start = uri + 1;
        
        while (start < uri + size) {
                const char *end = strchr(start, '/');
                if (end == NULL) {
                        if (start < uri + size - 1) {
                                int len = uri + size - start;
                                char *s = r_alloc(len+1);
                                memcpy(s, start, len);
                                s[len] = 0;
                                list = list_append(list, s);
                        }
                        return list;
                        
                } else {
                        if (end == start) {
                                start = end + 1;
                        } else {
                                int len = end - start;
                                char *s = r_alloc(len+1);
                                memcpy(s, start, len);
                                s[len] = 0;
                                list = list_append(list, s);
                                start = end + 1;
                        }
                }
        }
        return list;
}

static int fsdb_get_db_metadata(response_t *response)
{
        int add_comma = 0;
        response_printf(response, "{ \"scans\": [");
        for (int i = 0; i < database_count_scans(db); i++) {
                scan_t *scan = database_get_scan_at(db, i);
                if (add_comma)
                        response_printf(response, ",");
                if (scan) {
                        response_printf(response, "{\"id\": \"%s\"}", scan_id(scan));
                        add_comma = 1;
                }
        }
        response_printf(response, "]}");
        response_set_mimetype(response, "application/json");
        return 0;
}

static int fsdb_get_scan_metadata(response_t *response, const char *scan_id)
{
        scan_t *scan = database_get_scan(db, scan_id);
        if (scan == NULL) {
                r_warn("Could not find scan '%s'", scan_id);
                response_set_status(response, HTTP_Status_Not_Found);
                return -1;
        }
        
        int add_comma = 0;
        response_printf(response, "{ \"id\": \"%s\", ", scan_id);
        response_printf(response, "\"filesets\": [");
        for (int i = 0; i < scan_count_filesets(scan); i++) {
                fileset_t *fileset = scan_get_fileset_at(scan, i);
                if (add_comma)
                        response_printf(response, ",");
                if (fileset) {
                        response_printf(response, "{\"id\": \"%s\"}",
                                             fileset_id(fileset));
                        add_comma = 1;
                }
        }
        response_printf(response, "]}");
        response_set_mimetype(response, "application/json");
        return 0;
}

static int fsdb_get_fileset_metadata(response_t *response,
                                     const char *scan_id,
                                     const char *fileset_id)
{
        scan_t *scan = database_get_scan(db, scan_id);
        if (scan == NULL) {
                r_warn("Could not find scan '%s'", scan_id);
                response_set_status(response, HTTP_Status_Not_Found);
                return -1;
        }
        
         fileset_t *fileset = scan_get_fileset(scan, fileset_id);
        if (fileset == NULL) {
                r_warn("Could not find fileset '%s'", fileset_id);
                response_set_status(response, HTTP_Status_Not_Found);
                return -1;
        }
        
        int add_comma = 0;
        response_printf(response, "{ \"id\": \"%s\", ", fileset_id);
        response_printf(response, "\"scan_id\": \"%s\", ", scan_id);
        response_printf(response, "\"files\": [");
        for (int i = 0; i < fileset_count_files(fileset); i++) {
                file_t *file = fileset_get_file_at(fileset, i);
                if (add_comma)
                        response_printf(response, ",");
                if (file && file_mimetype(file) != NULL) {
                        response_printf(response,
                                             "{\"id\": \"%s\", \"mimetype\": \"%s\"}",
                                             file_id(file), file_mimetype(file));
                        add_comma = 1;
                }
        }
        response_printf(response, "]}");
        response_set_mimetype(response, "application/json");
        return 0;
}
        
static int fsdb_get_file_metadata(response_t *response,
                                  const char *scan_id,
                                  const char *fileset_id,
                                  const char *file_id)
{
        scan_t *scan = database_get_scan(db, scan_id);
        if (scan == NULL) {
                r_warn("Could not find scan '%s'", scan_id);
                response_set_status(response, HTTP_Status_Not_Found);
                return -1;
        }
        
        fileset_t *fileset = scan_get_fileset(scan, fileset_id);
        if (fileset == NULL) {
                r_warn("Could not find fileset '%s'", fileset_id);
                response_set_status(response, HTTP_Status_Not_Found);
                return -1;
        }
        
        file_t *file = fileset_get_file(fileset, file_id);
        if (file == NULL) {
                r_warn("Could not find file '%s'", file_id);
                response_set_status(response, HTTP_Status_Not_Found);
                return -1;
        }
        
        response_set_mimetype(response, "application/json");
        return response_json(response, file_get_metadata(file));
}

static int fsdb_get_metadata(response_t *response, list_t *query)
{
        const char *db_id = list_get(query, char);

        query = list_next(query);
        const char *scan_id = list_get(query, char);

        query = list_next(query);
        const char *fileset_id = list_get(query, char);
        
        query = list_next(query);
        const char *file_id = list_get(query, char);
        
        if (list_next(query) != NULL) {
                r_warn("URI too long");
                response_set_status(response, HTTP_Status_Bad_Request);
                return -1;
        }
        if (!rstreq(db_id, "db")) {
                r_warn("Invalid metadata id: '%s'", db_id);
                response_set_status(response, HTTP_Status_Not_Found);
                return -1;
        }

        if (scan_id == NULL)
                return fsdb_get_db_metadata(response);
        if (fileset_id == NULL)
                return fsdb_get_scan_metadata(response, scan_id);
        if (file_id == NULL)
                return fsdb_get_fileset_metadata(response, scan_id, fileset_id);
        else 
                return fsdb_get_file_metadata(response, scan_id, fileset_id, file_id);
}

static int fsdb_get_file(request_t *request, response_t *response, const char *path)
{
        char buffer[1024];
        FILE *fp = fopen(path, "rb");

        while (!feof(fp) && !ferror(fp)) {
                int n = fread(buffer, 1, sizeof(buffer), fp);
                response_append(response, buffer, n);
        }

        if (ferror(fp)) {
                r_err("An error occured reading file '%s'", path);
                response_set_status(response, HTTP_Status_Internal_Server_Error);
                return -1;
        }
        
        fclose(fp);
        return 0;
}

static int scale_image(const char *path, const char *alt, int width, int height)
{
        image_t *orig = image_load(path);
        if (orig == NULL)
                return -1;

        int h = (int)((float) height * (float) orig->width / (float) width);
        if (h < orig->height) {
                // The original image is taller. Cut away a border at
                // the top and bottom.
                image_t *cropped = FIXME_image_crop(orig,
                                                    0, (orig->height - h) / 2,
                                                    orig->width, h);
                delete_image(orig);
                orig = cropped;
        } else if (h > orig->height) {
                // The original image is wider. Cut away a border on
                // the left and right.
                int w = (int)((float) width * (float) orig->height / (float) height);
                image_t *cropped = FIXME_image_crop(orig,
                                                    (orig->width - w) / 2, 0,
                                                    w, orig->height);
                delete_image(orig);
                orig = cropped;
        }
        
        image_t *scaled = image_scale(orig, width, height);
        int err = image_store(scaled, alt, image_type(path));

        delete_image(orig);
        delete_image(scaled);
        
        return err;
}

/*
 * thumbnail:  100x75        
 * square:     150x150
 * small:      320x240        
 * medium:     800x600
 * large:      1920x1080
 * original
*/
static int fsdb_get_image(request_t *request, response_t *response, const char *path)
{
        const char *args = request_args(request);
        if (args == NULL)
                return fsdb_get_file(request, response, path);

        char alt[2048];
        int width = 0;
        int height = 0;
        if (rstreq(args, "original")) {
                return fsdb_get_file(request, response, path);
        } else if (rstreq(args, "thumbnail")) {
                rprintf(alt, sizeof(alt), "%s.thumbnail", path);
                width = 100;
                height = 75;
        } else if (rstreq(args, "square")) {
                rprintf(alt, sizeof(alt), "%s.square", path);
                width = 150;
                height = 150;
        } else if (rstreq(args, "small")) {
                rprintf(alt, sizeof(alt), "%s.small", path);
                width = 320;
                height = 240;
        } else if (rstreq(args, "medium")) {
                rprintf(alt, sizeof(alt), "%s.medium", path);
                width = 800;
                height = 600;
        } else if (rstreq(args, "large")) {
                rprintf(alt, sizeof(alt), "%s.large", path);
                width = 1920;
                height = 1080;
        } else {
                return fsdb_get_file(request, response, path);
        }

        if (path_exists(alt))
                return fsdb_get_file(request, response, alt);
        else if (scale_image(path, alt, width, height) == 0)
                return fsdb_get_file(request, response, alt);
        else
                return fsdb_get_file(request, response, path);
}

static int fsdb_get_data(request_t *request, response_t *response, list_t *query)
{
        const char *db_id = list_get(query, char);

        query = list_next(query);
        const char *scan_id = list_get(query, char);

        query = list_next(query);
        const char *fileset_id = list_get(query, char);
        
        query = list_next(query);
        const char *file_id = list_get(query, char);

        if (db_id == NULL || !rstreq(db_id, "db")) {
                r_warn("Missing DB prefix");
                response_set_status(response, 400);
                return -1;
        }
        if (scan_id == NULL) {
                r_warn("Missing scan ID");
                response_set_status(response, 400);
                return -1;
        }
        if (fileset_id == NULL) {
                r_warn("Missing fileset ID");
                response_set_status(response, 400);
                return -1;
        }
        if (file_id == NULL) {
                r_warn("Missing file ID");
                response_set_status(response, 400);
                return -1;
        }
        
        scan_t *scan = database_get_scan(db, scan_id);
        if (scan == NULL) {
                r_warn("Could not find scan '%s'", scan_id);
                response_set_status(response, HTTP_Status_Not_Found);
                return -1;
        }

        fileset_t *fileset = scan_get_fileset(scan, fileset_id);
        if (fileset == NULL) {
                r_warn("Could not find fileset '%s'", fileset_id);
                response_set_status(response, HTTP_Status_Not_Found);
                return -1;
        }

        file_t *file = fileset_get_file(fileset, file_id);
        if (file == NULL) {
                r_warn("Could not find file '%s'", file_id);
                response_set_status(response, HTTP_Status_Not_Found);
                return -1;
        }

        char path[1024];
        if (file_path(file, path, sizeof(path)) != 0) {
                r_warn("Failed to get the path of file '%s'", file_id);
                response_set_status(response, HTTP_Status_Internal_Server_Error);
                return -1;
        }

        const char *mimetype = filename_to_mimetype(path);
        if (mimetype == NULL) {
                r_warn("Failed to determine mimetype file '%s'", path);
                response_set_status(response, HTTP_Status_Internal_Server_Error);
                return -1;
        }
        response_set_mimetype(response, mimetype);

        if (rstreq(mimetype, "image/jpeg")
            || rstreq(mimetype, "image/png"))
                return fsdb_get_image(request, response, path);
        else
                return fsdb_get_file(request, response, path);
}

// scans:   /metadata/db
// scan:    /metadata/db/<scan-id>
// fileset: /metadata/db/<scan-id>/<fileset-id>
// file:    /metadata/db/<scan-id>/<fileset-id>/<file-id>

// file:    /data/db/<scan-id>/<fileset-id>/<file-id>

void fsdb_get(void *data, request_t *request, response_t *response)
{
        const char *uri = request_uri(request);
        list_t *query = parse_uri(uri);
        if (query == NULL) {
                response_set_status(response, HTTP_Status_Internal_Server_Error);
                return;
        }

        char *s = list_get(query, char);
        if (rstreq(s, "metadata")) {
            fsdb_get_metadata(response, list_next(query));
        } else if (rstreq(s, "data")) {
            fsdb_get_data(request, response, list_next(query));
        } else {
            response_set_status(response, HTTP_Status_Bad_Request);
        }

        for (list_t *l = query; l != NULL; l = list_next(l))
                r_free(list_get(l, char));
        delete_list(query);
}

void fsdb_get_directory(void *data, request_t *request, response_t *response)
{
        response_printf(response, "{\"db\": \"%s\"}", database_path(db));
}

void fsdb_handle_new_scan(database_t *database, json_object_t message)
{
        const char *scan_id = json_object_getstr(message, "scan");
        if (scan_id == NULL) {
                r_warn("fsdb_handle_new_scan: Missing scan ID");
                return;
        }
        database_import_scan(database, scan_id);
}

void fsdb_handle_new_fileset(database_t *database, json_object_t message)
{
        const char *scan_id = json_object_getstr(message, "scan");
        const char *fileset_id = json_object_getstr(message, "fileset");

        if (scan_id == NULL || fileset_id == NULL) {
                r_warn("fsdb_handle_new_fileset: Missing info for new file");
                return;
        }

        scan_t *scan = database_get_scan(database, scan_id);
        if (scan == NULL) {
                r_warn("Could not find scan '%s'", scan_id);
                return;
        }
        scan_import_fileset(scan, fileset_id);
}

void fsdb_handle_new_file(database_t *database, json_object_t message)
{
        const char *scan_id = json_object_getstr(message, "scan");
        const char *fileset_id = json_object_getstr(message, "fileset");
        const char *file_id = json_object_getstr(message, "file");
        const char *localfile = json_object_getstr(message, "localfile");
        const char *mimetype = json_object_getstr(message, "mimetype");

        if (scan_id == NULL
            || fileset_id == NULL
            || file_id == NULL
            || localfile == NULL
            || mimetype == NULL) {
                r_warn("fsdb_handle_new_file: Missing info for new file");
                return;
        }
        
        scan_t *scan = database_get_scan(database, scan_id);
        if (scan == NULL) {
                r_warn("Could not find scan '%s'", scan_id);
                return;
        }
        fileset_t *fileset = scan_get_fileset(scan, fileset_id);
        if (fileset == NULL) {
                r_warn("Could not find fileset '%s'", fileset_id);
                return;
        }
        fileset_import_file(fileset, file_id, localfile, mimetype);
}

void fsdb_handle_message(database_t *database, json_object_t message)
{
        const char *event = json_object_getstr(message, "event");

        if (event == NULL) {
                r_warn("fsdb_handle_message: received a message without an event type");
                return;
        } else if (rstreq(event, "new-file")) {
                fsdb_handle_new_file(database, message);
        } else if (rstreq(event, "new-fileset")) {
                fsdb_handle_new_fileset(database, message);
        } else if (rstreq(event, "new-scan")) {
                fsdb_handle_new_scan(database, message);
        }
}

void fsdb_onmessage(void *userdata, messagelink_t *link, json_object_t message)
{
        fsdb_handle_message(db, message);
        messagehub_t *hub = get_messagehub_db();
        messagehub_broadcast_obj(hub, link, message);
}
