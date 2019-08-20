
// TMP
#include <unistd.h>


#include <romi.h>
#include "camera_recorder.h"

static int initialized = 0;
static mutex_t *recording_mutex;
static mutex_t *position_mutex;
static char *directory = NULL;

static database_t *db = NULL;
static scan_t *scan = NULL;
static fileset_t *fileset = NULL;

static int recording = 0;
static multipart_parser_t *parser = NULL;
static vector_t position = {0};

streamerlink_t *get_streamerlink_camera();
messagelink_t *get_messagelink_db();

void broadcast_db_message(const char *event,
                          const char *scan_id,
                          const char *fileset_id,
                          const char *file_id)
{
        log_debug("broadcast_db_message: %s, %s, %s, %s",
                  event, scan_id, fileset_id, file_id);
        messagelink_t *bus = get_messagelink_db();
        if (file_id)
                messagelink_send_f(bus,
                                   "{\"event\": \"%s\", "
                                   "\"source\": \"camera_recorder\", "
                                   "\"scan\": \"%s\", "
                                   "\"fileset\": \"%s\", "
                                   "\"file\": \"%s\"}",
                                   event, scan_id, fileset_id, file_id);
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

void camera_recorder_onpose(void *userdata,
                            datalink_t *link,
                            data_t *input,
                            data_t *output)
{
        json_object_t data = datalink_parse(link, input);
        if (json_isnull(data)) {
                log_warn("camera_recorder_onpose: failed to parse data");
                json_unref(data);
                return;
        }

        json_object_t a = json_object_get(data, "position");
        if (!json_isarray(a)) {
                log_warn("camera_recorder_onpose: 'position' is not an array");
                json_unref(data);
                return;
        }
        
        double px, py, pz;
        px = json_array_getnum(a, 0);
        py = json_array_getnum(a, 1);
        pz = json_array_getnum(a, 2);
        if (isnan(px) || isnan(py) || isnan(pz)) {
                log_warn("camera_recorder_onpose: invalid position values"); 
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

static int *camera_recorder_onimage(void *userdata,
                                    const unsigned char *image, int len,
                                    const char *mimetype,
                                    double timestamp)
{
        mutex_lock(recording_mutex);
        
        if (recording) {
                // Check for valid JPEG signature
                if ((image[0] != 0xff) || (image[1] != 0xd8)  || (image[2] != 0xff)) {
                        log_warn("Image has a bad signature.");
                        goto unlock;
                }
                if ((image[len-2] != 0xff) || (image[len-1] != 0xd9)) {
                        log_warn("Image has a bad signature (2).");
                        goto unlock;
                }

                double lag = clock_time() - timestamp;
        
                if (lag < 1.0f) {
                        vector_t p;
                        mutex_lock(position_mutex);
                        p = position;
                        mutex_unlock(position_mutex);
                       
                        file_t *file = fileset_new_file(fileset);
                        if (file) {
                                file_set_timestamp(file, timestamp);
                                file_set_position(file, p);
                                file_import_jpeg(file, image, len);
                                broadcast_db_message("new", scan_id(scan),
                                                     fileset_id(fileset), file_id(file));
                        }
        
                } else {
                        log_info("Too much lag. Skipping image.");
                }
        }
        
unlock:
        mutex_unlock(recording_mutex);
}

int camera_recorder_ondata(void *userdata, const char *buf, int len)
{
        if (parser == NULL)
                return 0;
        multipart_parser_process(parser, buf, len);
        return 0;
}

static int set_directory(const char *path)
{
        if (directory != NULL) {
                mem_free(directory);
                directory = NULL;
        }
        directory = mem_strdup(path);
        if (directory == NULL) {
                log_err("Out of memory");
                return -1;
        }
        return 0;
}

static int get_configuration()
{
        if (directory != NULL)
                return 0;

        char buffer[1024];
        log_err("Current working directory: %s", getcwd(buffer, 1024));
        
        int err = -1;
        json_object_t path = json_null();
        path = client_get("configuration", "fsdb.directory");
        if (json_isstring(path)) {
                set_directory(json_string_value(path));
                log_info("using configuration file for directory: '%s'",
                         json_string_value(path));
                err = 0;
        } else {
                log_err("Failed to obtain a valid fsdb.directory setting");
        }
        json_unref(path);
        return err;
}

static int recorder_init()
{
        if (initialized)
                return 0;
        
        if (get_configuration() != 0)
                return -1;
        
        db = new_database(directory);
        if (db == NULL)
                return -1;
        
        initialized = 1;
        
        return 0;
}

int camera_recorder_init(int argc, char **argv)
{
        recording_mutex = new_mutex();
        position_mutex = new_mutex();
        if (recording_mutex == NULL
            || position_mutex == NULL)
                return -1;
        
        if (argc == 2) {
                // FIXME: needs more checking
                set_directory(argv[1]);
                log_info("using command line argument for directory: '%s'", argv[1]);
                
        }
        
        for (int i = 0; i < 10; i++) {
                if (recorder_init() == 0)
                        return 0;
                log_err("recorder_init failed: attempt %d/10", i);
                clock_sleep(0.2);
        }

        log_err("failed to initialize the camera recorder");        
        return -1;
}

void camera_recorder_cleanup()
{
        if (db)
                delete_database(db);
        if (recording_mutex)
                delete_mutex(recording_mutex);
        if (position_mutex)
                delete_mutex(position_mutex);
        if (parser)
                delete_multipart_parser(parser);
        if (directory)
                mem_free(directory);
}

int camera_recorder_onstart(void *userdata,
                            messagelink_t *link,
                            json_object_t command,
                            membuf_t *message)
{
        if (recorder_init() != 0) {
                membuf_printf(message, "Recorder not initialized");
                return 0;
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
                        mutex_unlock(recording_mutex);
                        return -1;
                }
                
                scan = database_new_scan(db);
                if (scan == NULL) {
                        database_unload(db);
                        membuf_printf(message, "Failed to create a new scan");
                        mutex_unlock(recording_mutex);
                        return -1;
                }
                broadcast_db_message("new", scan_id(scan), NULL, NULL);
                
                fileset = scan_new_fileset(scan, "images");
                if (fileset == NULL) {
                        database_unload(db);
                        scan = NULL;
                        membuf_printf(message, "Failed to create a new fileset");
                        mutex_unlock(recording_mutex);
                        return -1;
                }
                broadcast_db_message("new", scan_id(scan), fileset_id(fileset), NULL);
                
                recording = 1;
        }
        
        mutex_unlock(recording_mutex);
        return 0;
}

int camera_recorder_onstop(void *userdata,
                           messagelink_t *link,
                           json_object_t command,
                           membuf_t *message)
{
        if (recorder_init() != 0) {
                membuf_printf(message, "Recorder not initialized");
                return 0;
        }
        
        mutex_lock(recording_mutex);
        recording = 0;
        
        if (scan) {
                scan_store(scan);
                broadcast_db_message("store", scan_id(scan), NULL, NULL);
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
