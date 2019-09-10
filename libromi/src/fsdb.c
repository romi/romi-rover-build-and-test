#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <r.h>
#include "romi.h"

static int database_create_scan_dir(database_t *db, scan_t *scan);
static int database_get_scan_directory(database_t *db, scan_t *scan,
                                       char *buffer, int len);
static int database_get_scan_json_file(database_t *db, scan_t *scan,
                                       char *buffer, int len);
static int database_get_scan_metadata_path(database_t *db, scan_t *scan,
                                           char *buffer, int len);

static int database_create_fileset_dir(database_t *db, scan_t *scan, const char *id);
static int database_get_fileset_metadata_path(database_t *db, scan_t *scan,
                                              fileset_t *fileset, char *buffer, int len);

static int database_get_file_path(database_t *db, scan_t *scan, fileset_t *fileset,
                                  file_t *file, char *buffer, int len);
static int database_get_file_metadata_path(database_t *db, scan_t *scan, fileset_t *fileset,
                                           file_t *file, char *buffer, int len);


static void database_broadcast(database_t *db,
                               const char *event,
                               scan_t *scan,
                               fileset_t *fileset,
                               file_t *file);

static void scan_broadcast(scan_t *scan,
                           const char *event,
                           fileset_t *fileset,
                           file_t *file);

static void fileset_broadcast(fileset_t *fileset,
                              const char *event,
                              file_t *file);
static void file_broadcast(file_t *file, const char *event);

// FIXME
static int file_update_files(file_t *file);
static int fileset_update_files(fileset_t *fileset);
static int scan_store_filesets(scan_t *scan);

/**************************************************************/

struct _file_t {
        fileset_t *fileset;
        char *id;
        char *localfile;
        char *mimetype;
        json_object_t metadata;
};

file_t *new_file(fileset_t *fileset,
                 const char *id,
                 const char *localfile,
                 const char *mimetype)
{
        file_t *file = r_new(file_t);
        if (file == NULL) return NULL;

        file->fileset = fileset;
        file->id = r_strdup(id);
        file->localfile = localfile? r_strdup(localfile) : NULL;
        file->mimetype = mimetype? r_strdup(mimetype) : NULL;
        file->metadata = json_object_create();
        
        if (file->id == NULL
            || (localfile != NULL && file->localfile == NULL)
            || (mimetype != NULL && file->mimetype == NULL)) {
                r_delete(file);
                return NULL;
        }
        return file;
}

void delete_file( file_t *file)
{
        if (file) {
                if (file->id)
                        r_free(file->id);
                if (file->localfile)
                        r_free(file->localfile);
                json_unref(file->metadata);
                r_delete(file);
        }
}

const char *file_id(file_t *file)
{
        return file->id;
}

const char *file_localfile(file_t *file)
{
        return file->localfile;
}

const char *file_mimetype(file_t *file)
{
        return file->mimetype;
}

fileset_t *file_get_fileset(file_t *file)
{
        return file->fileset;
}

int file_path(file_t *file, char *buffer, int len)
{
        fileset_t *fileset = file_get_fileset(file);
        scan_t *scan = fileset_get_scan(fileset);
        database_t *db = scan_get_database(scan);
        return database_get_file_path(db, scan, fileset, file, buffer, len);
}

int file_get_metadata_path(file_t *file, char *buffer, int len)
{
        fileset_t *fileset = file_get_fileset(file);
        scan_t *scan = fileset_get_scan(fileset);
        database_t *db = scan_get_database(scan);
        return database_get_file_metadata_path(db, scan, fileset, file, buffer, len);
}

static void file_broadcast(file_t *file, const char *event)
{
        r_debug("file_broadcast %s", event);
        fileset_broadcast(file->fileset, event, file);
}

static int file_update_files(file_t *file)
{
        fileset_t *fileset = file_get_fileset(file);
        return fileset_update_files(fileset);
}

static int file_store_metadata(file_t *file)
{
        char path[1024];
        file_get_metadata_path(file, path, sizeof(path));
        return json_tofile(file->metadata, 0, path);
}

int file_import_data(file_t *file,
                     const char *data, int len,
                     const char *mimetype,
                     const char *file_extension)
{
        char localfile[256];
        char path[1024];
        int new_file = 1;

        r_debug("file_import_data");
        
        rprintf(localfile, sizeof(localfile), "%s.%s", file->id, file_extension);

        if (file->localfile != NULL) {
                r_free(file->localfile);
                file->localfile = NULL;
                new_file = 0;
        }
        file->localfile = r_strdup(localfile);
        if (file->localfile == NULL)
                return -1;

        if (file->mimetype != NULL)
                r_free(file->mimetype);        
        file->mimetype = r_strdup(mimetype);
        if (file->mimetype == NULL)
                return -1;

        file_path(file, path, sizeof(path));
        
        int fd = open(path,
                      O_WRONLY | O_CREAT,
                      S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
        if (fd == -1) {
                r_warn("Failed to open '%s': %s.", path, strerror(errno));
                return -1;
        }
        
        ssize_t n, wrote = 0;
        while (wrote < len) {
                n = write(fd, data + wrote, len - wrote);
		//r_debug("file_import_jpeg: %s: len=%d, wrote=%d, n=%d", path, len, wrote, n);
                if (n == -1) {
                        r_warn("Failed to write '%s': %s.", path, strerror(errno));
                        close(fd);
                        return -1;
                }
                wrote += n;
        }
        close(fd);

        if (file_update_files(file) != 0) {
                // FIXME: then what?
        }

        int err = file_set_metadata_str(file, "mimetype", mimetype);

        if (new_file)
                file_broadcast(file, "new");
        else
                file_broadcast(file, "update");
        
        return err;
}

int file_import_jpeg(file_t *file, const char *image, int len)
{
        return file_import_data(file, image, len, "image/jpeg", "jpg");
}

int file_import_png(file_t *file, const char *image, int len)
{
        return file_import_data(file, image, len, "image/png", "png");
}

int file_import_svg(file_t *file, const char *svg, int len)
{
        return file_import_data(file, svg, len, "image/svg+xml", "svg");
}

int file_import_image(file_t *file, image_t*image, const char *format)
{
        file_set_timestamp(file, clock_time());
        
        membuf_t *buffer = new_membuf();
        if (buffer == NULL)
                return -1;

        if (image_store_to_mem(image, buffer, format) != 0) {
                delete_membuf(buffer);
                return -1;
	}
        
        file_import_data(file, membuf_data(buffer), membuf_len(buffer),
                         image_mimetype(format), format);

	delete_membuf(buffer);
        return 0;
}

void file_set_timestamp(file_t *file, double timestamp)
{
        file_set_metadata_num(file, "timestamp", timestamp);
}

double file_get_timestamp(file_t *file)
{
        return json_object_getnum(file->metadata, "timestamp");
}

void file_set_position(file_t *file, vector_t position)
{
        json_object_t a = json_array_create();
        json_array_setnum(a, 0, position.x);
        json_array_setnum(a, 1, position.y);
        json_array_setnum(a, 2, position.z);
        file_set_metadata(file, "position", a);
        json_unref(a);
}

vector_t file_get_position(file_t *file)
{
        vector_t p = vector(0, 0, 0);
        json_object_t a = json_object_get(file->metadata, "position");
        if (!json_falsy(a)) {
                p.x = json_array_getnum(a, 0);
                p.y = json_array_getnum(a, 1);
                p.z = json_array_getnum(a, 2);
        }
        return p;
}

static int file_load_metadata(file_t *file)
{
        char path[1024];
        file_get_metadata_path(file, path, sizeof(path));

        if (!is_file(path))
                return 0;

        int err;
        char errmsg[128];
        json_object_t obj = json_load(path, &err, errmsg, sizeof(errmsg));
        if (err != 0) {
                r_warn("Failed to load the metadata for file '%s': %s",
                         file->id, errmsg);
                return -1;
        }

        json_unref(file->metadata);
        file->metadata = obj;
        return 0;
}

static int file_load(file_t *file, json_object_t obj)
{
        const char *localfile = json_object_getstr(obj, "file");
        if (localfile == NULL) {
                r_warn("File has no localfile! (id %s)", file->id);
                return -1;
        }
        if (file->localfile)
                r_free(file->localfile);
        file->localfile = r_strdup(localfile);
        if (file->localfile == NULL)
                return -1;

        return file_load_metadata(file);
}

void file_print(file_t *file)
{
        printf("        File '%s', '%s'\n", file->id, file->localfile);
}

json_object_t file_get_metadata(file_t *file)
{
        // TODO: return a clone?
        return file->metadata;
}

int file_set_metadata_str(file_t *file, const char *key, const char *value)
{
        json_object_setstr(file->metadata, key, value);
        return file_store_metadata(file);
}

int file_set_metadata_num(file_t *file, const char *key, double value)
{
        json_object_setnum(file->metadata, key, value);
        return file_store_metadata(file);
}

int file_set_metadata(file_t *file, const char *key, json_object_t value)
{
        json_object_set(file->metadata, key, value);
        return file_store_metadata(file);
}

const char *file_get_metadata_str(file_t *file, const char *key)
{
        return json_object_getstr(file->metadata, key);
}

/**************************************************************/

struct _fileset_t {
        scan_t *scan;
        char *id;
        list_t *files;
        int num_files;
        json_object_t metadata;
};

static int fileset_unload(fileset_t *fileset);
static int fileset_load(fileset_t *fileset, json_object_t obj);

fileset_t *new_fileset(scan_t *scan, const char *id)
{
        fileset_t *fileset = r_new(fileset_t);
        if (fileset == NULL) return NULL;
        
        fileset->scan = scan;
        fileset->id = r_strdup(id);
        fileset->files = NULL;
        fileset->num_files = 0;
        fileset->metadata = json_object_create();
        
        if (fileset->id == NULL) {
                r_delete(fileset);
                return NULL;
        }
        
        return fileset;
}

void delete_fileset(fileset_t *fileset)
{
        if (fileset) {
                if (fileset->id) r_free(fileset->id);
                json_unref(fileset->metadata);
                r_delete(fileset);
        }
}

static void fileset_broadcast(fileset_t *fileset,
                              const char *event,
                              file_t *file)
{
        r_debug("fileset_broadcast");
        scan_broadcast(fileset->scan, event, fileset, file);
}

static int fileset_update_files(fileset_t *fileset)
{
        scan_t *scan = fileset_get_scan(fileset);
        return scan_store_filesets(scan);
}

static int fileset_add_file(fileset_t *fileset, file_t *file)
{
        fileset->num_files++;
        fileset->files = list_prepend(fileset->files, file);
        return (fileset->files == NULL)? -1 : 0;
}

static int fileset_remove_file(fileset_t *fileset, file_t *file)
{
        fileset->files = list_remove(fileset->files, file);
        return 0;
}

static int fileset_unload(fileset_t *fileset)
{
        if (fileset->files) {
                for (list_t *l = fileset->files; l != NULL; l = list_next(l)) {
                        file_t* f = list_get(l, file_t);
                        delete_file(f);
                }
                delete_list(fileset->files);
                fileset->files = NULL;
        }
}

static int fileset_get_metadata_path(fileset_t *fileset, char *buffer, int len)
{
        scan_t *scan = fileset_get_scan(fileset);
        database_t *db = scan_get_database(scan);
        return database_get_fileset_metadata_path(db, scan, fileset, buffer, len);
}

static int fileset_store_metadata(fileset_t *fileset)
{
        char path[1024];
        fileset_get_metadata_path(fileset, path, sizeof(path));
        return json_tofile(fileset->metadata, 0, path);
}

static int fileset_store_file(fileset_t *fileset, file_t *file, FILE* fp)
{
        //r_debug("fileset_store_file @1");
        if (file->localfile == NULL || file->mimetype == NULL) {
                //r_debug("fileset_store_file @1.1");
                //r_warn("File '%s' has no file localfile. Skipping.", file->id);
        } else {
                //r_debug("fileset_store_file @1.2");
                fprintf(fp, "{\"id\": \"%s\", \"file\": \"%s\", \"mimetype\": \"%s\"}",
                        file->id, file->localfile, file->mimetype);
        }
        //r_debug("fileset_store_file @2");
        return 0;
}

static int fileset_store_files(fileset_t *fileset, FILE* fp)
{
        //r_debug("fileset_store_files @1");
        fprintf(fp, "{\"id\": \"%s\", \"files\": [", fileset->id);

        list_t *files = fileset->files;
        while (files) {
                //r_debug("fileset_store_files @2");
                file_t *file = list_get(files, file_t);;
                fileset_store_file(fileset, file, fp);
                files = list_next(files);
                if (files) fprintf(fp, ", ");
        } 
        fprintf(fp, "]}");
        //r_debug("fileset_store_files @3");
        return 0;
}

static int fileset_load_metadata(fileset_t *fileset)
{
        char path[1024];
        fileset_get_metadata_path(fileset, path, sizeof(path));

        if (!is_file(path))
                return 0;

        int err;
        char errmsg[128];
        json_object_t obj = json_load(path, &err, errmsg, sizeof(errmsg));
        if (err != 0) {
                r_warn("Failed to load the metadata for file '%s': %s",
                       fileset->id, errmsg);
                return -1;
        }

        json_unref(fileset->metadata);
        fileset->metadata = obj;
        return 0;
}

static int fileset_load(fileset_t *fileset, json_object_t obj)
{
        if (fileset_unload(fileset) != 0) {
                r_warn("fileset_load: Unload failed");
                return -1;
        }
        
        json_object_t files = json_object_get(obj, "files");
        if (!json_isarray(files)) {
                r_warn("Failed to load fileset '%s': "
                         "the files field is supposed to be a JSON array",
                         fileset->id);
                return -1;
        }
        for (int i = 0; i < json_array_length(files); i++) {
                json_object_t f = json_array_get(files, i);
                const char *id = json_object_getstr(f, "id");
                if (id == NULL) {
                        r_warn("While loading fileset '%s/%s': "
                                 "file %d has no id!",
                                 scan_id(fileset->scan), fileset->id, i);
                        continue;
                }
                const char *localfile = json_object_getstr(f, "file");
                if (localfile == NULL) {
                        r_warn("While loading fileset '%s/%s': file %d has no localfile.",
                                 scan_id(fileset->scan), fileset->id, i);
                }
                const char *mimetype = json_object_getstr(f, "mimetype");
                if (mimetype == NULL) {
                        r_warn("While loading fileset '%s/%s': file %d has no mimetype.",
                                 scan_id(fileset->scan), fileset->id, i);
                }
                file_t *file = new_file(fileset, id, localfile, mimetype);
                if (file == NULL)
                        return -1;
                if (file_load(file, f) != 0) {
                        delete_file(file);
                } else if (fileset_add_file(fileset, file) != 0) {
                        delete_file(file);
                        return -1;
                }
        }
        return fileset_load_metadata(fileset);
}

scan_t *fileset_get_scan(fileset_t *fileset)
{
        return fileset->scan;
}

file_t *fileset_new_file(fileset_t *fileset)
{
        char id[32];
        int num = fileset->num_files;
        
        //r_debug("fileset_new_file @1");
        while (num < 1000000) {
                //r_debug("fileset_new_file @2");
                snprintf(id, 32, "%05d", num);
                id[31] = 0;
                if (fileset_get_file(fileset, id) == NULL)
                        break;
                num++;
        }
        
        //r_debug("fileset_new_file @3");
        file_t *file = new_file(fileset, id, NULL, NULL);
        if (file == NULL) return NULL;
        //r_debug("fileset_new_file @4");
        fileset_add_file(fileset, file);
        //r_debug("fileset_new_file @5");

        if (fileset_update_files(fileset) != 0) {
                fileset_remove_file(fileset, file);
                delete_file(file);
                return NULL;
        }
        
        //fileset_broadcast(fileset, "new", file);

        return file;
}

const char *fileset_id(fileset_t *fileset)
{
        return fileset->id;
}

int fileset_count_files(fileset_t *fileset)
{
        //r_debug("fileset_count_files: list_size=%d", list_size(fileset->files));
        return list_size(fileset->files);
}

file_t *fileset_get_file_at(fileset_t *fileset, int i)
{
        list_t *l = list_nth(fileset->files, i);
        return l? list_get(l, file_t) : NULL;
}

file_t *fileset_get_file(fileset_t *fileset, const char *id)
{
        for (list_t *l = fileset->files; l != NULL; l = list_next(l)) {
                file_t *file = list_get(l, file_t);
                if (rstreq(file->id, id))
                        return file;
        }
        return NULL;
}

/* file_t *fileset_get_file_by_name(fileset_t *fileset, const char *name) */
/* { */
/*         for (list_t *l = fileset->files; l != NULL; l = list_next(l)) { */
/*                 file_t *file = list_get(l, file_t); */
/*                 if (file->name != NULL */
/*                     && rstreq(file->name, name)) */
/*                         return file; */
/*         } */
/*         return NULL; */
/* } */

void fileset_print(fileset_t *fileset)
{
        printf("    Fileset '%s'\n", fileset->id);
        for (list_t *l = fileset->files; l != NULL; l = list_next(l)) {
                file_t *file = list_get(l, file_t);
                file_print(file);
        }
}

int fileset_set_metadata_str(fileset_t *fileset, const char *key, const char *value)
{
        json_object_setstr(fileset->metadata, key, value);
        return fileset_store_metadata(fileset);
}

int fileset_set_metadata_num(fileset_t *fileset, const char *key, double value)
{
        json_object_setnum(fileset->metadata, key, value);
        return fileset_store_metadata(fileset);
}

int fileset_set_metadata(fileset_t *fileset, const char *key, json_object_t value)
{
        json_object_set(fileset->metadata, key, value);
        return fileset_store_metadata(fileset);
}

file_t* fileset_store_image(fileset_t *fileset, const char *name,
                            image_t *image, const char *format)
{
        if (fileset == NULL)
                return NULL;
        
        file_t *file = fileset_new_file(fileset);
        if (file == NULL)
                return NULL;
        
        file_set_metadata_str(file, "name", name);
        if (file_import_image(file, image, format) != 0)
                return NULL;
        
        return file;
}

/**************************************************************/

struct _scan_t {
        database_t *db;
        char *id;
        list_t *filesets;
        json_object_t metadata;
};

static int scan_unload(scan_t *scan);

scan_t *new_scan(database_t *db, const char *id)
{
        scan_t *scan = r_new(scan_t);
        if (scan == NULL) return NULL;

        scan->db = db;
        scan->id = r_strdup(id);
        scan->filesets = NULL;
        scan->metadata = json_object_create();
        
        if (scan->id == NULL) {
                r_delete(scan);
                return NULL;
        }
        
        return scan;
}

void delete_scan(scan_t *scan)
{
        if (scan) {
                scan_unload(scan);
                if (scan->id)
                        r_free(scan->id);
                r_delete(scan);
                json_unref(scan->metadata);
        }
}

static void scan_broadcast(scan_t *scan,
                           const char *event,
                           fileset_t *fileset,
                           file_t *file)
{
        r_debug("scan_broadcast");
        database_broadcast(scan->db, event, scan, fileset, file);
}

const char *scan_id(scan_t *scan)
{
        return scan->id;
}

int scan_count_filesets(scan_t *scan)
{
        return list_size(scan->filesets);
}

fileset_t *scan_get_fileset_at(scan_t *scan, int i)
{
        list_t *l = list_nth(scan->filesets, i);
        return l? list_get(l, fileset_t) : NULL;
}

fileset_t *scan_get_fileset(scan_t *scan, const char *id)
{
        for (list_t *l = scan->filesets; l != NULL; l = list_next(l)) {
                fileset_t *fileset = list_get(l, fileset_t);
                if (rstreq(fileset->id, id))
                        return fileset;
        }
        return NULL;
}

static int scan_get_directory(scan_t *scan, char *buffer, int len)
{
        database_t *db = scan_get_database(scan);
        return database_get_scan_directory(scan->db, scan, buffer, len);
}

static int scan_add_fileset(scan_t *scan, fileset_t *fileset)
{
        scan->filesets = list_prepend(scan->filesets, fileset);
        return (scan->filesets == NULL)? -1 : 0;
}

fileset_t *scan_new_fileset(scan_t *scan, const char *id)
{
        if (database_create_fileset_dir(scan->db, scan, id) != 0)
                return NULL;
        // FIXME: should be check that the if is unique?
        fileset_t *fileset = new_fileset(scan, id);
        if (fileset == NULL)
                return NULL;
        if (scan_add_fileset(scan, fileset) != 0) {
                delete_fileset(fileset);
                return NULL;
	}
        if (scan_store_filesets(scan) != 0) {
                delete_fileset(fileset);
                return NULL;
        }
        scan_broadcast(scan, "new", fileset, NULL);
	return fileset;
}

static int scan_get_fileset_listing(scan_t *scan, char *buffer, int len)
{
        return database_get_scan_json_file(scan->db, scan, buffer, len);
}

static int scan_unload(scan_t *scan)
{
        if (scan->filesets) {
                list_t *l = scan->filesets;
                while (l) {
                        fileset_t* f = list_get(l, fileset_t);
                        delete_fileset(f);
                        l = list_next(l);
                }
                delete_list(scan->filesets);
                scan->filesets = NULL;
        }
}

database_t *scan_get_database(scan_t *scan)
{
        return scan->db;
}

static int scan_get_metadata_path(scan_t *scan, char *buffer, int len)
{
        return database_get_scan_metadata_path(scan->db, scan, buffer, len);
}

static int scan_store_metadata(scan_t *scan)
{
        int err = 0;
        char path[1024];
        
        err = scan_get_metadata_path(scan, path, sizeof(path));
        if (err != 0) return err;
        
        return json_tofile(scan->metadata, 0, path);
}

static int scan_store_filesets(scan_t *scan)
{
        char path[1024];
        char backup[1024];

        //r_debug("scan_store_filesets @1");
	if (scan_get_fileset_listing(scan, path, sizeof(path)) != 0)
                return -1;

        rprintf(backup, sizeof(backup), "%s.backup", path);
        if (rename(path, backup) != 0) {
                if (errno != ENOENT)
                        r_warn("Failed to create a backup file");
        }
        
        //r_info("scan_store: Storing filesets to '%s'", path);
        
        //r_debug("scan_store_filesets @2");
        FILE* fp = fopen(path, "w");
        if (fp == NULL) {
                r_warn("Failed to open file '%s'", path);
                return -1;
        }
        
        //r_debug("scan_store_filesets @3");
        fprintf(fp, "{\"filesets\": [");
        for (list_t *l = scan->filesets; l != NULL; l = list_next(l)) {
                //r_debug("scan_store_filesets @3.1");
                fileset_t *fileset = list_get(l, fileset_t);
                fileset_store_files(fileset, fp);
                if (list_next(l)) fprintf(fp, ", ");
        }
        
        fprintf(fp, "]}");
        fclose(fp);
        //r_debug("scan_store_filesets @4");

        unlink(backup);
        return 0;
}

/* int scan_store(scan_t *scan) */
/* { */
/*         int err = 0; */
/*         //r_debug("scan_store @1"); */
/*         if (scan_store_filesets(scan) != 0) */
/*                 err--; */
/*         //r_debug("scan_store @2"); */
/*         if (scan_store_metadata(scan) != 0) */
/*                 err--; */
/*         //r_debug("scan_store @3"); */
/*         return 0; */
/* } */

static int scan_load_metadata(scan_t *scan)
{
        char path[1024];
        scan_get_metadata_path(scan, path, sizeof(path));

        if (!is_file(path))
                return 0;
        
        int err;
        char errmsg[128];
        json_object_t obj = json_load(path, &err, errmsg, sizeof(errmsg));
        if (err != 0) {
                r_warn("Failed to load the metadata for scan '%s': %s",
                         scan->id, errmsg);
                return -1;
        }

        json_unref(scan->metadata);
        scan->metadata = obj;
        return 0;
}

static int scan_load_files(scan_t *scan)
{
        //r_info("Loading scan '%s'", scan->id);

        char buffer[1024];
        scan_get_fileset_listing(scan, buffer, sizeof(buffer));

        int err;
        char errmsg[128];
        json_object_t obj = json_load(buffer, &err, errmsg, sizeof(errmsg));
        if (json_falsy(obj)) {
                r_warn("Failed to load scan '%s'", scan->id);
                return -1;
        }

        json_object_t filesets = json_object_get(obj, "filesets");
        if (!json_isarray(filesets)) {
                r_warn("Failed to load scan '%s': "
                         "the filesets field is supposed to be a JSON array",
                         scan->id);
                return -1;
        }

        for (int i = 0; i < json_array_length(filesets); i++) {
                json_object_t fs = json_array_get(filesets, i);
                const char *id = json_object_getstr(fs, "id");
                if (id == NULL) {
                        r_warn("While loading scan '%s': "
                                 "fileset %d has no id!",
                                 scan->id, i);
                        continue;
                }
                fileset_t *fileset = new_fileset(scan, id);
                if (fileset_load(fileset, fs) != 0) {
                        delete_fileset(fileset);
                } else if (scan_add_fileset(scan, fileset) != 0) {
                        delete_fileset(fileset);
                        json_unref(obj);
                        return -1;
                }
                /* for (int i = 0; i < fileset_count_files(fileset); i++) { */
                /*         file_t *file = fileset_get_file_at(fileset, i); */
                /*         r_debug("%s", file_id(file));; */
                /* } */
        }
        
        json_unref(obj);

        return 0;
}

static int scan_load(scan_t *scan)
{
        if (scan_unload(scan) != 0) {
                r_warn("scan_load: Unload failed");
                return -1;
        }

        if (scan_load_files(scan) != 0)
                return -1;
        return scan_load_metadata(scan);
}

void scan_print(scan_t *scan)
{
        printf("Scan '%s'\n", scan->id);
        for (list_t *l = scan->filesets; l != NULL; l = list_next(l)) {
                fileset_t *fileset = list_get(l, fileset_t);
                fileset_print(fileset);
        }
}

int scan_set_metadata_str(scan_t *scan, const char *key, const char *value)
{
        json_object_setstr(scan->metadata, key, value);
        return scan_store_metadata(scan);
}

int scan_set_metadata_num(scan_t *scan, const char *key, double value)
{
        json_object_setnum(scan->metadata, key, value);
        return scan_store_metadata(scan);
}

int scan_set_metadata(scan_t *scan, const char *key, json_object_t value)
{
        json_object_set(scan->metadata, key, value);
        return scan_store_metadata(scan);
}

/**************************************************************/

struct _database_t {
        char *path;
        list_t *scans;
        database_listener_t listener;
        void *userdata;
};

database_t *new_database(const char *path)
{
        database_t *db = r_new(database_t);
        if (db == NULL) return NULL;

        db->userdata = NULL;
        db->listener = NULL;
        db->scans = NULL;
        db->path = r_strdup(path);
        if (db->path == NULL) {
                r_delete(db);
                return NULL;
        }
        
        return db;
}

void delete_database(database_t *db)
{
        if (db) {
                database_unload(db);
                if (db->path)
                        r_free(db->path);
                r_delete(db);
        }
}

static int database_add_scan(database_t *db, scan_t *scan)
{
        db->scans = list_prepend(db->scans, scan);
        return (db->scans == NULL)? -1 : 0;
}

int database_count_scans(database_t *db)
{
        return list_size(db->scans);
}

scan_t *database_get_scan_at(database_t *db, int i)
{
        list_t* l = list_nth(db->scans, i);
        return l? list_get(l, scan_t) : NULL;
}

scan_t *database_get_scan(database_t *db, const char *id)
{
        for (list_t *l = db->scans; l != NULL; l = list_next(l)) {
                scan_t *scan = list_get(l, scan_t);
                if (rstreq(scan->id, id))
                        return scan;
        }
        return NULL;
}

int database_unload(database_t *db)
{
        //r_debug("database_unload @1");
        if (db->scans) {
                //r_debug("database_unload @2");
                for (list_t *l = db->scans; l != NULL; l = list_next(l)) {
                        //r_debug("database_unload @2.0");
                        scan_t *scan = list_get(l, scan_t);
                        //r_debug("database_unload @2.1");
                        delete_scan(scan);
                }
                //r_debug("database_unload @2.2");
                delete_list(db->scans);
                db->scans = NULL;
        }
        //r_debug("database_unload @3");
        return 0;
}
        
int database_load(database_t *db)
{
        int err = 0;
        if (database_unload(db) != 0) {
                r_warn("database_unload: Unload failed");
                return -1;
        }

        list_t *ids = directory_list(db->path);
        for (list_t *l = ids; l != NULL; l = list_next(l)) {
                char *id = list_get(l, char);

                char files[1024];
                rprintf(files, sizeof(files), "%s/%s/files.json", db->path, id);
                if (!is_file(files))
                        continue;
                
                scan_t *scan = new_scan(db, id);
                if (scan == NULL) {
                        err = -1;
                        break;
                }
                if (scan_load(scan) != 0) {
                        delete_scan(scan);
                } else if (database_add_scan(db, scan) != 0) {
                        delete_scan(scan);
                        err = -1;
                        break;
                }
        }
        for (list_t *l = ids; l != NULL; l = list_next(l))
                r_free(list_get(l, char));

        return err;
}

scan_t *database_new_scan(database_t *db, const char *id)
{
        char datetime[256];
        const char *s = id;

        if (s == NULL) {
                clock_datetime(datetime, sizeof(datetime), '-', '_', '-');
                s = datetime;
        }
        scan_t *scan = new_scan(db, s);
	if (scan == NULL) return NULL;
	
        if (database_create_scan_dir(db, scan) != 0) {
                delete_scan(scan);
                return NULL;
        }
        
	if (database_add_scan(db, scan) != 0) {
                delete_scan(scan);
                return NULL;
	}
        database_broadcast(db, "new", scan, NULL, NULL);
        return scan;
}

// --

static int database_get_file_path(database_t *db,
                                  scan_t *scan,
                                  fileset_t *fileset,
                                  file_t *file,
                                  char *buffer, int len)
{
        if (rprintf(buffer, len, "%s/%s/%s/%s",
                    db->path, scan->id,
                    fileset->id, file->localfile) == NULL)
                return -1;
        return 0;
}

static int database_get_file_metadata_path(database_t *db,
                                           scan_t *scan,
                                           fileset_t *fileset,
                                           file_t *file,
                                           char *buffer, int len)
{
        if (rprintf(buffer, len, "%s/%s/metadata/%s/%s.json",
                    db->path, scan->id, fileset->id, file->id) == NULL)
                return -1;
        return 0;
}

// --

static int database_create_fileset_dir(database_t *db, scan_t *scan, const char *id)
{
        char path[1024];
        
        snprintf(path, 1024, "%s/%s/%s", db->path, scan->id, id);
        path[1023] = 0;
        
	if (directory_create(path) == -1) {
                r_err("Failed to create directory '%s'", path);
                return -1;
	}

        snprintf(path, 1024, "%s/%s/metadata/%s", db->path, scan->id, id);
        path[1023] = 0;
	if (directory_create(path) == -1) {
                r_err("Failed to create directory '%s'", path);
                return -1;
	}
        
        return 0;
}

static int database_get_fileset_metadata_path(database_t *db,
                                              scan_t *scan,
                                              fileset_t *fileset,
                                              char *buffer, int len)
{
        if (rprintf(buffer, len, "%s/%s/metadata/%s.json",
                     db->path, scan->id, fileset->id) == NULL)
                return -1;
        return 0;
}

//

static int database_get_scan_directory(database_t *db,
                                       scan_t *scan,
                                       char *buffer, int len)
{
        if (rprintf(buffer, len, "%s/%s", db->path, scan->id) == NULL)
                return -1;
        return 0;
}

static int database_create_scan_dir(database_t *db, scan_t *scan)
{
        char path[1024];

        if (database_get_scan_directory(db, scan, path, sizeof(path)) != 0)
                return -1;

        if (path_exists(path)) {
                if (is_directory(path))
                        return 0;
                else {
                        r_err("Scan path exists but is not a directory: '%s'", path);
                        return -1;
                }
        }
        
	if (directory_create(path) == -1) {
                r_err("Failed to create directory '%s'", path);
                return -1;
	}

        r_info("new scan '%s' in directory '%s'", scan->id, path);

        if (rprintf(path, sizeof(path), "%s/%s/metadata", db->path, scan->id) == NULL)
                return -1;

	if (directory_create(path) == -1) {
                r_err("Failed to create directory '%s'", path);
                return -1;
	}
        return 0;
}

static int database_get_scan_metadata_path(database_t *db,
                                    scan_t *scan,
                                    char *buffer, int len)
{
        if (rprintf(buffer, len, "%s/%s/metadata/metadata.json",
                    db->path, scan->id) == NULL)
                return -1;
        return 0;
}

static int database_get_scan_json_file(database_t *db,
                                       scan_t *scan,
                                       char *buffer, int len)
{
        if (rprintf(buffer, len, "%s/%s/files.json", db->path, scan->id) == NULL)
                return -1;
        return 0;
}

//

void database_print(database_t *db)
{
        for (list_t *l = db->scans; l != NULL; l = list_next(l)) {
                scan_t *scan = list_get(l, scan_t);
                scan_print(scan);
        }
}

void database_set_listener(database_t *db, database_listener_t listener, void *userdata)
{
        db->userdata = userdata;
        db->listener = listener;
}

void database_broadcast(database_t *db,
                        const char *event,
                        scan_t *scan,
                        fileset_t *fileset,
                        file_t *file)
{
        const char *scan_id = scan? scan->id : NULL;
        const char *fileset_id = fileset? fileset->id : NULL;
        const char *file_id = file? file->id : NULL;
        const char *mimetype = file? file->mimetype : NULL;
        r_debug("database_broadcast");
        if (db->listener) {
                db->listener(db->userdata, db, event, scan_id,
                             fileset_id, file_id, mimetype);
                r_debug("database_broadcast: done");
        }
}

const char *database_path(database_t *db)
{
        return db->path;
}
