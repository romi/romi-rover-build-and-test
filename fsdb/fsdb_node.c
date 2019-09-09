#include <romi.h>
#include "fsdb_node.h"

static database_t *db = NULL;

streamer_t *get_streamer_camera();
messagehub_t *get_messagehub_db();
list_t *parse_uri(const char *uri);

int fsdb_node_init(int argc, char **argv)
{
        json_object_t path = json_null();
        const char *dir = NULL;
        char buffer[1024];
        
        if (argc >= 2) {
                dir = argv[1];
        } else if (app_get_session() != NULL) {
                rprintf(buffer, sizeof(buffer), "%s/db", app_get_session());
                dir = buffer;
        } else {
                path = client_get("configuration", "fsdb.directory");
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

list_t *parse_uri(const char *uri)
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

int fsdb_get_db_metadata(response_t *response)
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

int fsdb_get_scan_metadata(response_t *response, const char *scan_id)
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

int fsdb_get_fileset_metadata(response_t *response,
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
        
int fsdb_get_file_metadata(response_t *response,
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

int fsdb_get_metadata(response_t *response, list_t *query)
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
                response_set_status(response, 400);
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

int fsdb_get_data(response_t *response, list_t *query)
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

// scans:   /metadata/db
// scan:    /metadata/db/<scan-id>
// fileset: /metadata/db/<scan-id>/<fileset-id>
// file:    /metadata/db/<scan-id>/<fileset-id>/<file-id>

// file:    /data/db/<scan-id>/<fileset-id>/<file-id>

void fsdb_get(void *data, request_t *request, response_t *response)
{
        int err;
        const char *uri = request_uri(request);
        list_t *query = parse_uri(uri);
        if (query == NULL) {
                response_set_status(response, HTTP_Status_Internal_Server_Error);
                return;
        }
        
        database_print(db);
        
        char *s = list_get(query, char);
        if (rstreq(s, "metadata"))
                err = fsdb_get_metadata(response, list_next(query));
        else if (rstreq(s, "data"))
                err = fsdb_get_data(response, list_next(query));
        else
                err = -1;
        
        for (list_t *l = query; l != NULL; l = list_next(l))
                r_free(list_get(l, char));
        delete_list(query);
}

void fsdb_get_directory(void *data, request_t *request, response_t *response)
{
        response_printf(response, "{\"db\": \"%s\"}", database_path(db));
}

int fsdb_onmessage(void *userdata, messagelink_t *link, json_object_t message)
{
        r_debug("fsdb_onmessage");
        const char *event = json_object_getstr(message, "event");

        messagehub_t *hub = get_messagehub_db();
        messagehub_broadcast_obj(hub, link, message);
}
