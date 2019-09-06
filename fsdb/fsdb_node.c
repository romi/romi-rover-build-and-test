#include <romi.h>
#include "fsdb_node.h"

static membuf_t *buffer = NULL;
static database_t *db = NULL;
static scan_t *session = NULL;

streamer_t *get_streamer_camera();
messagehub_t *get_messagehub_db();
list_t *parse_uri(const char *uri);

int fsdb_node_init(int argc, char **argv)
{
        json_object_t path = json_null();
        const char *dir = NULL;
        
        if (argc >= 2) {
                dir = argv[1];
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

        session = database_new_scan(db, NULL);
        r_info("Created new session directory '%s'", scan_id(session));
        
        json_unref(path);
        
        buffer = new_membuf();
        if (buffer == NULL) {
                json_unref(path);
                return -1;
        }
        
        return 0;
}

void fsdb_node_cleanup()
{
        if (buffer)
                delete_membuf(buffer);
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

int fsdb_get_db_metadata(request_t *request)
{
        int add_comma = 0;
        request_reply_printf(request, "{ \"scans\": [");
        for (int i = 0; i < database_count_scans(db); i++) {
                scan_t *scan = database_get_scan_at(db, i);
                if (add_comma)
                        request_reply_printf(request, ",");
                if (scan) {
                        request_reply_printf(request, "{\"id\": \"%s\"}", scan_id(scan));
                        add_comma = 1;
                }
        }
        request_reply_printf(request, "]}");
        request_set_mimetype(request, "application/json");
        return 0;
}

int fsdb_get_scan_metadata(request_t *request, const char *scan_id)
{
        scan_t *scan = database_get_scan(db, scan_id);
        if (scan == NULL) {
                r_warn("Could not find scan '%s'", scan_id);
                request_set_status(request, 404);
                return -1;
        }
        
        int add_comma = 0;
        request_reply_printf(request, "{ \"id\": \"%s\", ", scan_id);
        request_reply_printf(request, "\"filesets\": [");
        for (int i = 0; i < scan_count_filesets(scan); i++) {
                fileset_t *fileset = scan_get_fileset_at(scan, i);
                if (add_comma)
                        request_reply_printf(request, ",");
                if (fileset) {
                        request_reply_printf(request, "{\"id\": \"%s\"}",
                                             fileset_id(fileset));
                        add_comma = 1;
                }
        }
        request_reply_printf(request, "]}");
        request_set_mimetype(request, "application/json");
        return 0;
}

int fsdb_get_fileset_metadata(request_t *request,
                              const char *scan_id,
                              const char *fileset_id)
{
        scan_t *scan = database_get_scan(db, scan_id);
        if (scan == NULL) {
                r_warn("Could not find scan '%s'", scan_id);
                request_set_status(request, 404);
                return -1;
        }
        
         fileset_t *fileset = scan_get_fileset(scan, fileset_id);
        if (fileset == NULL) {
                r_warn("Could not find fileset '%s'", fileset_id);
                request_set_status(request, 404);
                return -1;
        }
        
        int add_comma = 0;
        request_reply_printf(request, "{ \"id\": \"%s\", ", fileset_id);
        request_reply_printf(request, "\"scan_id\": \"%s\", ", scan_id);
        request_reply_printf(request, "\"files\": [");
        for (int i = 0; i < fileset_count_files(fileset); i++) {
                file_t *file = fileset_get_file_at(fileset, i);
                if (add_comma)
                        request_reply_printf(request, ",");
                if (file && file_mimetype(file) != NULL) {
                        request_reply_printf(request,
                                             "{\"id\": \"%s\", \"mimetype\": \"%s\"}",
                                             file_id(file), file_mimetype(file));
                        add_comma = 1;
                }
        }
        request_reply_printf(request, "]}");
        request_set_mimetype(request, "application/json");
        return 0;
}
        
int fsdb_get_file_metadata(request_t *request,
                           const char *scan_id,
                           const char *fileset_id,
                           const char *file_id)
{
        scan_t *scan = database_get_scan(db, scan_id);
        if (scan == NULL) {
                r_warn("Could not find scan '%s'", scan_id);
                request_set_status(request, 404);
                return -1;
        }
        
        fileset_t *fileset = scan_get_fileset(scan, fileset_id);
        if (fileset == NULL) {
                r_warn("Could not find fileset '%s'", fileset_id);
                request_set_status(request, 404);
                return -1;
        }
        
        file_t *file = fileset_get_file(fileset, file_id);
        if (file == NULL) {
                r_warn("Could not find file '%s'", file_id);
                request_set_status(request, 404);
                return -1;
        }
        
        request_set_mimetype(request, "application/json");
        return request_reply_json(request, file_get_metadata(file));
}

int fsdb_get_metadata(request_t *request, list_t *query)
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
                request_set_status(request, 400);
                return -1;
        }
        if (!rstreq(db_id, "db")) {
                r_warn("Invalid metadata id: '%s'", db_id);
                request_set_status(request, 404);
                return -1;
        }

        if (scan_id == NULL)
                return fsdb_get_db_metadata(request);
        if (fileset_id == NULL)
                return fsdb_get_scan_metadata(request, scan_id);
        if (file_id == NULL)
                return fsdb_get_fileset_metadata(request, scan_id, fileset_id);
        else 
                return fsdb_get_file_metadata(request, scan_id, fileset_id, file_id);
}

int fsdb_get_data(request_t *request, list_t *query)
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
                request_set_status(request, 400);
                return -1;
        }
        if (scan_id == NULL) {
                r_warn("Missing scan ID");
                request_set_status(request, 400);
                return -1;
        }
        if (fileset_id == NULL) {
                r_warn("Missing fileset ID");
                request_set_status(request, 400);
                return -1;
        }
        if (file_id == NULL) {
                r_warn("Missing file ID");
                request_set_status(request, 400);
                return -1;
        }
        
        scan_t *scan = database_get_scan(db, scan_id);
        if (scan == NULL) {
                r_warn("Could not find scan '%s'", scan_id);
                request_set_status(request, 404);
                return -1;
        }

        fileset_t *fileset = scan_get_fileset(scan, fileset_id);
        if (fileset == NULL) {
                r_warn("Could not find fileset '%s'", fileset_id);
                request_set_status(request, 404);
                return -1;
        }

        file_t *file = fileset_get_file(fileset, file_id);
        if (file == NULL) {
                r_warn("Could not find file '%s'", file_id);
                request_set_status(request, 404);
                return -1;
        }

        char path[1024];
        if (file_path(file, path, sizeof(path)) != 0) {
                r_warn("Failed to get the path of file '%s'", file_id);
                request_set_status(request, 500);
                return -1;
        }

        const char *mimetype = filename_to_mimetype(path);
        if (mimetype == NULL) {
                r_warn("Failed to determine mimetype file '%s'", path);
                request_set_status(request, 500);
                return -1;
        }
        request_set_mimetype(request, mimetype);

        char buffer[1024];
        FILE *fp = fopen(path, "rb");

        while (!feof(fp) && !ferror(fp)) {
                int n = fread(buffer, 1, sizeof(buffer), fp);
                request_reply_append(request, buffer, n);
        }

        if (ferror(fp)) {
                r_err("An error occured reading file '%s'", path);
                request_set_status(request, 500);
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

int fsdb_get(void *data, request_t *request)
{
        int err;
        const char *uri = request_uri(request);
        list_t *query = parse_uri(uri);
        if (query == NULL) 
                return -1;

        database_print(db);
        
        char *s = list_get(query, char);
        if (rstreq(s, "metadata"))
                err = fsdb_get_metadata(request, list_next(query));
        else if (rstreq(s, "data"))
                err = fsdb_get_data(request, list_next(query));
        else err = -1;
        
        for (list_t *l = query; l != NULL; l = list_next(l))
                r_free(list_get(l, char));
        delete_list(query);
        
        return err;
}

int fsdb_get_session(void *data, request_t *request)
{
        request_reply_printf(request, "{\"db\": \"%s\", \"session\": \"%s\"}",
                             database_path(db), scan_id(session));
}

int fsdb_onmessage(void *userdata, messagelink_t *link, json_object_t message)
{
        r_debug("fsdb_onmessage");
        const char *event = json_object_getstr(message, "event");

        messagehub_t *hub = get_messagehub_db();
        messagehub_broadcast_obj(hub, link, message);
}
