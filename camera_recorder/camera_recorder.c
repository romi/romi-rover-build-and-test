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

void camera_recorder_onpose(void *userdata,
                            datalink_t *link,
                            data_t *input,
                            data_t *output)
{
        json_object_t data = datalink_parse(link, input);
        if (json_isnull(data)) {
                log_warn("camera_recorder_onpose: failed to parse data");
                return;
        }

        json_object_t a = json_object_get(data, "position");
        if (!json_isarray(a)) {
                log_warn("camera_recorder_onpose: 'position' is not an array");
                return;
        }
        
        double px, py, pz;
        px = json_array_getnum(a, 0);
        py = json_array_getnum(a, 1);
        pz = json_array_getnum(a, 2);
        if (isnan(px) || isnan(py) || isnan(pz)) {
                log_warn("camera_recorder_onpose: invalid position values"); 
                return;
        }
        
        mutex_lock(position_mutex);
        position.x = px;
        position.y = py;
        position.z = pz;
        mutex_unlock(position_mutex);
}

static int *camera_recorder_onimage(void *userdata,
                                    const unsigned char *image, int len,
                                    const char *mimetype,
                                    double timestamp)
{
        log_debug("camera_recorder_onimage.");
        log_debug("camera_recorder_onimage @1");
        
        mutex_lock(recording_mutex);
        log_debug("camera_recorder_onimage @2");
        
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
        
        log_debug("camera_recorder_onimage @3");
        
                if (lag < 1.0f) {
                        vector_t p;
                        
        log_debug("camera_recorder_onimage @4");
        
                        mutex_lock(position_mutex);
                        p = position;
                        mutex_unlock(position_mutex);

         log_debug("camera_recorder_onimage @5");
        
                       
                        file_t *file = fileset_new_file(fileset, timestamp, p);
                        if (file) {
                                file_import_jpeg(file, image, len);
                        }

        log_debug("camera_recorder_onimage @6");
        
                } else {
                        log_info("Too much lag. Skipping image.");
                }
        }
        log_debug("camera_recorder_onimage @7");
        
unlock:
        mutex_unlock(recording_mutex);
        log_debug("camera_recorder_onimage @8");
        
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

        int err = -1;
        json_object_t path = json_null();
        path = client_get("configuration", "camera-recorder.directory");
        if (json_isstring(path)) {
                set_directory(json_string_value(path));
                log_info("using configuration file for directory: '%s'",
                         json_string_value(path));
                err = 0;
        } else {
                log_err("Failed to obtain a valid camera-recorder.directory setting");
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

                log_debug("camera_recorder_onstart @1: link=%p", link);

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

                log_debug("camera_recorder_onstart @2");
                
                scan = database_new_scan(db);
                if (scan == NULL) {
                        membuf_printf(message, "Failed to create a new scan");
                        mutex_unlock(recording_mutex);
                        return -1;
                }
        
                log_debug("camera_recorder_onstart @3");
                
                fileset = database_new_fileset(db, scan, "images");
                if (fileset == NULL) {
                        membuf_printf(message, "Failed to create a new fileset");
                        delete_scan(scan);
                        scan = NULL;
                        mutex_unlock(recording_mutex);
                        return -1;
                }

                log_debug("camera_recorder_onstart @4");
                
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
        log_debug("camera_recorder_onstop @1");

        if (recorder_init() != 0) {
                membuf_printf(message, "Recorder not initialized");
                return 0;
        }
        
        mutex_lock(recording_mutex);
        recording = 0;
        
        log_debug("camera_recorder_onstop @2");
        
        if (scan) {
                database_store_scan(db, scan);
                delete_scan(scan);
                scan = NULL;
                fileset = NULL;
        }
        
        log_debug("camera_recorder_onstop @3");
        
        if (parser != NULL) {
                delete_multipart_parser(parser);
                parser = NULL;
        }
        
        log_debug("camera_recorder_onstop @4");
        
        mutex_unlock(recording_mutex);

        
        log_debug("camera_recorder_onstop @5");

        // Don't call streamerlink_disconnect() inside the
        // mutex-locked section. The handler may be calling ondata
        // which results in a deadlock.
        int err = streamerlink_disconnect(get_streamerlink_camera());
        if (err != 0) {
                membuf_printf(message, "Failed to disconnect");
                return -1;
        }

        log_debug("camera_recorder_onstop @6");
        
        return 0;
}
