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

// TMP
#include <unistd.h>


#include <romi.h>
#include "camera_recorder.h"

//static int initialized = 0;
static mutex_t *recording_mutex;
static mutex_t *position_mutex;
static char *directory = NULL;

static database_t *db = NULL;
static scan_t *scan = NULL;
static fileset_t *fileset = NULL;

static int recording_count = 0;
static int recording = 0;
static multipart_parser_t *parser = NULL;
static vector_t position = { 0.0, 0.0, 0.0 };
static vector_t last_position = { 0.0, 0.0, 0.0  };
static double minimum_displacement = -1.0f;

streamerlink_t *get_streamerlink_camera();
messagelink_t *get_messagelink_db();

static void broadcast_db_message(void *userdata,
                                 database_t *db,
                                 const char *event,
                                 const char *scan_id,
                                 const char *fileset_id,
                                 const char *file_id,
                                 const char *mimetype,
                                 const char *localfile)
{
        /* r_debug("broadcast_db_message: %s, %s, %s, %s, %s, %s", */
        /*         event, scan_id, fileset_id, file_id, localfile, mimetype); */
        messagelink_t *bus = get_messagelink_db();
        if (file_id)
                messagelink_send_f(bus,
                                   "{\"event\": \"%s\", "
                                   "\"source\": \"camera_recorder\", "
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
                                   "\"source\": \"camera_recorder\", "
                                   "\"scan\": \"%s\", "
                                   "\"fileset\": \"%s\"}",
                                   event, scan_id, fileset_id);
        else messagelink_send_f(bus,
                                "{\"event\": \"%s\", "
                                "\"source\": \"camera_recorder\", "
                                "\"scan\": \"%s\"}",
                                event, scan_id);
}

static void broadcast_log(void *userdata, const char* s)
{
        messagelink_t *get_messagelink_logger();
        messagelink_t *log = get_messagelink_logger();
        if (log)
                messagelink_send_str(log, s);
}

void camera_recorder_onpose(void *userdata,
                            datalink_t *link,
                            data_t *input,
                            data_t *output)
{
        json_object_t data = datalink_parse(link, input);
        if (json_isnull(data)) {
                r_warn("camera_recorder_onpose: failed to parse data");
                json_unref(data);
                return;
        }

        json_object_t a = json_object_get(data, "position");
        if (!json_isarray(a)) {
                r_warn("camera_recorder_onpose: 'position' is not an array");
                json_unref(data);
                return;
        }

        double px, py, pz;
        px = json_array_getnum(a, 0);
        py = json_array_getnum(a, 1);
        pz = json_array_getnum(a, 2);
        if (isnan(px) || isnan(py) || isnan(pz)) {
                r_warn("camera_recorder_onpose: invalid position values");
                json_unref(data);
                return;
        }

        mutex_lock(position_mutex);
        position.x = px;
        position.y = py;
        position.z = pz;
        mutex_unlock(position_mutex);

        json_unref(data);
}

static int camera_recorder_onimage(void *userdata,
				   const unsigned char *image, int len,
				   const char *mimetype,
				   double timestamp)
{
        double displacement;
        vector_t p;

        mutex_lock(position_mutex);
        p = position;
        mutex_unlock(position_mutex);
        
        displacement = vector_distance(last_position, p);
        last_position = p;
        r_debug("displacement %.3f, minimum %.3f",
                displacement, minimum_displacement);

        mutex_lock(recording_mutex);

        if (!recording)
                r_debug("camera_recorder_onimage: not recording!");
        if (fileset == NULL) 
                r_debug("camera_recorder_onimage: fileset is null!");
        
        if (recording
            && fileset != NULL
            && displacement >= minimum_displacement) {
                // Check for valid JPEG signature
                if ((image[0] != 0xff) || (image[1] != 0xd8)  || (image[2] != 0xff)) {
                        r_warn("Image has a bad signature.");
                        goto unlock;
                }
                if ((image[len-2] != 0xff) || (image[len-1] != 0xd9)) {
                        r_warn("Image has a bad signature (2).");
                        goto unlock;
                }

                double lag = clock_time() - timestamp;
                //r_debug("lag %f, timestamp %f", lag, timestamp);
                if (lag < 1.0f) {
 //                       vector_t p;
                        mutex_lock(position_mutex);
                        p = position;
                        mutex_unlock(position_mutex);

                        file_t *file = fileset_new_file(fileset);
                        if (file) {
                                file_set_timestamp(file, timestamp);
                                file_set_position(file, p);
                                file_import_jpeg(file, (const char*)image, len);
                        } else {
                                r_warn("camera_recorder_onimage: "
                                       "Failed to create the file");
                        }

                } else {
                        r_info("Too much lag. Skipping image.");
                }
        }

unlock:
        mutex_unlock(recording_mutex);
        return 0;
}

int camera_recorder_ondata(void *userdata, response_t *response, const char *buf, int len)
{
        if (parser == NULL)
                return 0;
        return multipart_parser_process(parser, response, buf, len);
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

static int init_database()
{
        if (db != NULL) 
                return 0;
        
        char *dir = get_directory();
        if (dir == NULL)
                return -1;
        
        db = new_database(dir);
        database_set_listener(db, broadcast_db_message, NULL);
        
        r_free(dir);
        return 0;
}

int camera_recorder_init(int argc, char **argv)
{
        r_log_set_writer(broadcast_log, NULL);
        
        recording_mutex = new_mutex();
        position_mutex = new_mutex();
        if (recording_mutex == NULL || position_mutex == NULL)
                return -1;

        if (argc == 2) {
                // FIXME: needs more checking
                directory = r_strdup(argv[1]);
                r_info("using command line argument for directory: '%s'", directory);
        }
        return 0;
}

void camera_recorder_cleanup()
{
        if (recording_mutex)
                delete_mutex(recording_mutex);
        if (position_mutex)
                delete_mutex(position_mutex);
        if (parser)
                delete_multipart_parser(parser);
        if (directory)
                r_free(directory);
        if (db)
                delete_database(db);
}

static scan_t *make_scan(database_t *database)
{
        char id[64];

        database_load(database);
        
        while (!app_quit()) {
                rprintf(id, sizeof(id), "camera-%04d", recording_count++);
                scan_t *scan = database_get_scan(database, id);
                if (scan == NULL)
                        break;
        }
        database_unload(database);
        return database_new_scan(database, id);
}

int camera_recorder_onstart(void *userdata,
                            messagelink_t *link,
                            json_object_t command,
                            membuf_t *message)
{
        int err = 0;

        double dist = -1.0;
        if (json_object_has(command, "minimum-displacement")) {
                dist = json_object_getnum(command, "minimum-displacement");
                if (isnan(dist)) {
                        membuf_printf(message,
                                      "Invalid value for minimum displacement");
                        return -1;
                }
        }
        minimum_displacement = dist;
        
        if (init_database() != 0) {
                membuf_printf(message, "Failed to initialize the database");
                return -1;
        }

        mutex_lock(recording_mutex);

        if (recording == 0) {
                if (parser != NULL) {
                        delete_multipart_parser(parser);
                        parser = NULL;
                }

                parser = new_multipart_parser(NULL, NULL,
                                              (multipart_onpart_t) camera_recorder_onimage);

                int err = streamerlink_connect(get_streamerlink_camera());
                if (err != 0) {
                        membuf_printf(message, "Failed to connect");
                        err = -1;
                        goto unlock_and_exit;
                }
                
                recording = 1;
        }
        
        scan = make_scan(db);
        if (scan == NULL) {
                recording = 0;
                database_unload(db);
                membuf_printf(message, "Failed to create a new scan");
                err = -1;
                goto unlock_and_exit;
        }
        
        fileset = scan_new_fileset(scan, "images");
        if (fileset == NULL) {
                recording = 0;
                database_unload(db);
                scan = NULL;
                membuf_printf(message, "Failed to create a new fileset");
                err = -1;
                goto unlock_and_exit;
        }
        
unlock_and_exit:
        mutex_unlock(recording_mutex);
        return err;
}

int camera_recorder_onstop(void *userdata,
                           messagelink_t *link,
                           json_object_t command,
                           membuf_t *message)
{
        mutex_lock(recording_mutex);
        recording = 0;

        if (scan) {
                database_unload(db);
                scan = NULL;
                fileset = NULL;
        }

        if (parser != NULL) {
                delete_multipart_parser(parser);
                parser = NULL;
        }

        mutex_unlock(recording_mutex);

        // Don't call streamerlink_disconnect() inside the
        // mutex-locked section. The handler may be calling ondata
        // which results in a deadlock.
        int err = streamerlink_disconnect(get_streamerlink_camera());
        if (err != 0) {
                membuf_printf(message, "Failed to disconnect");
                return -1;
        }

        return 0;
}
