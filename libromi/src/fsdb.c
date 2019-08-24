#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <rcom.h>

#include "romi.h"

/**************************************************************/

int database_get_scan_directory(database_t *db, scan_t *scan, char *buffer, int len);
int database_get_scan_json_file(database_t *db, scan_t *scan, char *buffer, int len);

int database_get_file_path(database_t *db, scan_t *scan,
                           fileset_t *fileset, file_t *file,
                           char *buffer, int len);
int database_get_file_metadata_path(database_t *db,
                                    scan_t *scan,
                                    fileset_t *fileset,
                                    file_t *file,
                                    char *buffer, int len);
int database_create_fileset_dir(database_t *db, scan_t *scan, const char *id);

/**************************************************************/

struct _file_t {
        fileset_t *fileset;
        char *id;
        char *name;
        char *mimetype;
        json_object_t metadata;
};

file_t *new_file(fileset_t *fileset,
                 const char *id,
                 const char *name,
                 const char *mimetype)
{
        file_t *file = new_obj(file_t);
        if (file == NULL) return NULL;

        file->fileset = fileset;
        file->id = mem_strdup(id);
        file->name = name? mem_strdup(name) : NULL;
        file->mimetype = mimetype? mem_strdup(mimetype) : NULL;
        file->metadata = json_object_create();
        
        if (file->id == NULL
            || (name != NULL && file->name == NULL)
            || (mimetype != NULL && file->mimetype == NULL)) {
                delete_obj(file);
                return NULL;
        }
        return file;
}

void delete_file( file_t *file)
{
        if (file) {
                if (file->id)
                        mem_free(file->id);
                if (file->name)
                        mem_free(file->name);
                json_unref(file->metadata);
                delete_obj(file);
        }
}

const char *file_id(file_t *file)
{
        return file->id;
}

const char *file_name(file_t *file)
{
        return file->name;
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

static int file_store_metadata(file_t *file)
{
        char path[1024];
        file_get_metadata_path(file, path, sizeof(path));
        return json_tofile(file->metadata, 0, path);
}

int file_import_jpeg(file_t *file, const unsigned char *image, int len)
{
        char name[256];
        snprintf(name, 256, "%s.jpg", file->id);
        name[255] = 0;

        file->name = mem_strdup(name);
        if (file->name == NULL)
                return -1;

        char path[1024];
        file_path(file, path, 1024);
        
        int fd = open(path,
                      O_WRONLY | O_CREAT,
                      S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
        if (fd == -1) {
                log_warn("Failed to open '%s': %s.", path, strerror(errno));
                return -1;
        }
        
        ssize_t n, wrote = 0;
        while (wrote < len) {
                n = write(fd, image + wrote, len - wrote);
		//log_debug("file_import_jpeg: %s: len=%d, wrote=%d, n=%d", path, len, wrote, n);
                if (n == -1) {
                        log_warn("Failed to write '%s': %s.", path, strerror(errno));
                        close(fd);
                        return -1;
                }
                wrote += n;
        }
        close(fd);

        json_object_setstr(file->metadata, "mimetype", "image/jpeg");
        return file_store_metadata(file);
}

void file_set_timestamp(file_t *file, double timestamp)
{
        json_object_setnum(file->metadata, "timestamp", timestamp);
}

double file_get_timestamp(file_t *file)
{
        return json_object_getnum(file->metadata, "timestamp");
}

void file_set_position(file_t *file, vector_t position)
{
        json_object_t a = json_object_get(file->metadata, "position");
        if (json_falsy(a)) {
                a = json_array_create();
                json_object_set(file->metadata, "position", a);
                json_unref(a);
        }
        json_array_setnum(a, 0, position.x);
        json_array_setnum(a, 1, position.y);
        json_array_setnum(a, 2, position.z);
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

        int err;
        char errmsg[128];
        json_object_t obj = json_load(path, &err, errmsg, sizeof(errmsg));
        if (err != 0) {
                log_warn("Failed to load the metadata for file '%s': %s",
                         file->id, errmsg);
                return -1;
        }

        json_unref(file->metadata);
        file->metadata = obj;
        return 0;
}

static int file_load(file_t *file, json_object_t obj)
{
        json_print(obj, 0);
        
        const char *name = json_object_getstr(obj, "file");
        if (name == NULL) {
                log_warn("File has no name! (id %s)", file->id);
                return -1;
        }
        if (file->name)
                mem_free(file->name);
        file->name = mem_strdup(name);
        if (file->name == NULL)
                return -1;

        return file_load_metadata(file);
}

void file_print(file_t *file)
{
        printf("        File '%s', '%s'\n", file->id, file->name);
}

json_object_t file_get_metadata(file_t *file)
{
        // TODO: return a clone?
        return file->metadata;
}

/**************************************************************/

struct _fileset_t {
        scan_t *scan;
        char *id;
        list_t *files;
        int num_files;
};

static int fileset_unload(fileset_t *fileset);
static int fileset_load(fileset_t *fileset, json_object_t obj);

fileset_t *new_fileset(scan_t *scan, const char *id)
{
        fileset_t *fileset = new_obj(fileset_t);
        if (fileset == NULL) return NULL;
        
        fileset->scan = scan;
        fileset->id = mem_strdup(id);
        fileset->files = NULL;
        fileset->num_files = 0;
        
        if (fileset->id == NULL) {
                delete_obj(fileset);
                return NULL;
        }
        
        return fileset;
}

void delete_fileset(fileset_t *fileset)
{
        if (fileset) {
                if (fileset->id) mem_free(fileset->id);
                delete_obj(fileset);
        }
}

static int fileset_add_file(fileset_t *fileset, file_t *file)
{
        fileset->num_files++;
        fileset->files = list_prepend(fileset->files, file);
        return fileset->files == NULL;
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

static int fileset_store_file(fileset_t *fileset, file_t *file, FILE* fp)
{
        //log_debug("fileset_store_file @1");
        if (file->name == NULL) {
                //log_debug("fileset_store_file @1.1");
                log_warn("File '%s' has no file name. Skipping.", file->id);
        } else {
                //log_debug("fileset_store_file @1.2");
                fprintf(fp, "{\"file\": \"%s\", \"id\": \"%s\"}", file->name, file->id);
        }
        //log_debug("fileset_store_file @2");
        return 0;
}

static int fileset_store_files(fileset_t *fileset, FILE* fp)
{
        //log_debug("fileset_store_files @1");
        fprintf(fp, "{\"id\": \"%s\", \"files\": [", fileset->id);

        list_t *files = fileset->files;
        while (files) {
                //log_debug("fileset_store_files @2");
                file_t *file = list_get(files, file_t);;
                fileset_store_file(fileset, file, fp);
                files = list_next(files);
                if (files) fprintf(fp, ", ");
        } 
        fprintf(fp, "]}");
        //log_debug("fileset_store_files @3");
        return 0;
}

static int fileset_store_metadata(fileset_t *fileset)
{
        int err = 0;
        for (list_t *l = fileset->files; l != NULL; l = list_next(l)) {
                file_t *file = list_get(l, file_t);
                if (file_store_metadata(file) != 0)
                        err--;
        } 
        return err;
}

static int fileset_load(fileset_t *fileset, json_object_t obj)
{
        if (fileset_unload(fileset) != 0) {
                log_warn("fileset_load: Unload failed");
                return -1;
        }
        
        json_object_t files = json_object_get(obj, "files");
        if (!json_isarray(files)) {
                log_warn("Failed to load fileset '%s': "
                         "the files field is supposed to be a JSON array",
                         fileset->id);
                return -1;
        }
        for (int i = 0; i < json_array_length(files); i++) {
                json_object_t f = json_array_get(files, i);
                const char *id = json_object_getstr(f, "id");
                if (id == NULL) {
                        log_warn("While loading fileset '%s/%s': "
                                 "file %d has no id!",
                                 scan_id(fileset->scan), fileset->id, i);
                        continue;
                }
                const char *name = json_object_getstr(f, "file");
                if (name == NULL) {
                        log_warn("While loading fileset '%s/%s': file %d has no name.",
                                 scan_id(fileset->scan), fileset->id, i);
                }
                const char *mimetype = json_object_getstr(f, "mimetype");
                if (name == NULL) {
                        log_warn("While loading fileset '%s/%s': file %d has no mimetype.",
                                 scan_id(fileset->scan), fileset->id, i);
                }
                file_t *file = new_file(fileset, id, name, mimetype);
                if (file == NULL)
                        return -1;
                if (file_load(file, f) != 0) {
                        delete_file(file);
                } else if (fileset_add_file(fileset, file) != 0) {
                        delete_file(file);
                        return -1;
                }
        }
        return 0;
}

scan_t *fileset_get_scan(fileset_t *fileset)
{
        return fileset->scan;
}

file_t *fileset_new_file(fileset_t *fileset)
{
        char id[32];
        
        //log_debug("fileset_new_file @1");
        while (1) {
                //log_debug("fileset_new_file @2");
                snprintf(id, 32, "%05d", fileset->num_files);
                id[31] = 0;
                if (fileset_get_file(fileset, id) == NULL)
                        break;
                fileset->num_files++;
        }
        
        //log_debug("fileset_new_file @3");
        file_t *file = new_file(fileset, id, NULL, NULL);
        if (file == NULL) return NULL;
        //log_debug("fileset_new_file @4");
        fileset_add_file(fileset, file);
        //log_debug("fileset_new_file @5");
        fileset->num_files++;
        //log_debug("fileset_new_file @6");
        return file;
}

const char *fileset_id(fileset_t *fileset)
{
        return fileset->id;
}

int fileset_count_files(fileset_t *fileset)
{
        //log_debug("fileset_count_files: list_size=%d", list_size(fileset->files));
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

file_t *fileset_get_file_by_name(fileset_t *fileset, const char *name)
{
        for (list_t *l = fileset->files; l != NULL; l = list_next(l)) {
                file_t *file = list_get(l, file_t);
                if (file->name != NULL
                    && rstreq(file->name, name))
                        return file;
        }
        return NULL;
}

void fileset_print(fileset_t *fileset)
{
        printf("    Fileset '%s'\n", fileset->id);
        for (list_t *l = fileset->files; l != NULL; l = list_next(l)) {
                file_t *file = list_get(l, file_t);
                file_print(file);
        }
}

/**************************************************************/

struct _scan_t {
        database_t *db;
        char *id;
        list_t *filesets;
};

static int scan_unload(scan_t *scan);

scan_t *new_scan(database_t *db, const char *id)
{
        scan_t *scan = new_obj(scan_t);
        if (scan == NULL) return NULL;

        scan->db = db;
        scan->id = mem_strdup(id);
        scan->filesets = NULL;
        
        if (scan->id == NULL) {
                delete_obj(scan);
                return NULL;
        }
        
        return scan;
}

void delete_scan(scan_t *scan)
{
        if (scan) {
                scan_unload(scan);
                if (scan->id)
                        mem_free(scan->id);
                delete_obj(scan);
        }
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

static int scan_store_metadata(scan_t *scan)
{
        int err = 0;
        for (list_t *l = scan->filesets; l != NULL; l = list_next(l)) {
                fileset_t *fileset = list_get(l, fileset_t);
                if (fileset_store_metadata(fileset) != 0)
                        err--;
        }
        return err;
}

static int scan_store_filesets(scan_t *scan)
{
        char path[1024];
	
        //log_debug("scan_store_filesets @1");
	if (scan_get_fileset_listing(scan, path, sizeof(path)) != 0)
                return -1;

        log_info("scan_store: Storing filesets to '%s'", path);
        
        //log_debug("scan_store_filesets @2");
        FILE* fp = fopen(path, "w");
        if (fp == NULL) {
                log_warn("Failed to open file '%s'", path);
                return -1;
        }
        
        //log_debug("scan_store_filesets @3");
        fprintf(fp, "{\"filesets\": [");
        for (list_t *l = scan->filesets; l != NULL; l = list_next(l)) {
                //log_debug("scan_store_filesets @3.1");
                fileset_t *fileset = list_get(l, fileset_t);
                fileset_store_files(fileset, fp);
                if (l) fprintf(fp, ", ");
        }
        
        fprintf(fp, "]}");
        fclose(fp);
        //log_debug("scan_store_filesets @4");
        return 0;
}

int scan_store(scan_t *scan)
{
        int err = 0;
        //log_debug("scan_store @1");
        if (scan_store_filesets(scan) != 0)
                err--;
        //log_debug("scan_store @2");
        if (scan_store_metadata(scan) != 0)
                err--;
        //log_debug("scan_store @3");
        return 0;
}

static int scan_load(scan_t *scan)
{
        if (scan_unload(scan) != 0) {
                log_warn("scan_load: Unload failed");
                return -1;
        }

        log_info("Loading scan '%s'", scan->id);

        char buffer[1024];
        scan_get_fileset_listing(scan, buffer, sizeof(buffer));

        int err;
        char errmsg[128];
        json_object_t obj = json_load(buffer, &err, errmsg, sizeof(errmsg));
        if (json_falsy(obj)) {
                log_warn("Failed to load scan '%s'", scan->id);
                return -1;
        }

        json_object_t filesets = json_object_get(obj, "filesets");
        if (!json_isarray(filesets)) {
                log_warn("Failed to load scan '%s': "
                         "the filesets field is supposed to be a JSON array",
                         scan->id);
                return -1;
        }

        for (int i = 0; i < json_array_length(filesets); i++) {
                json_object_t fs = json_array_get(filesets, i);
                const char *id = json_object_getstr(fs, "id");
                if (id == NULL) {
                        log_warn("While loading scan '%s': "
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
                /*         log_debug("%s", file_id(file));; */
                /* } */
        }
        
        json_unref(obj);
        return 0;
}

void scan_print(scan_t *scan)
{
        printf("Scan '%s'\n", scan->id);
        for (list_t *l = scan->filesets; l != NULL; l = list_next(l)) {
                fileset_t *fileset = list_get(l, fileset_t);
                fileset_print(fileset);
        }
}

/**************************************************************/

struct _database_t {
        char *path;
        list_t *scans;
};

database_t *new_database(const char *path)
{
        database_t *db = new_obj(database_t);
        if (db == NULL) return NULL;

        db->scans = NULL;
        db->path = mem_strdup(path);
        if (db->path == NULL) {
                delete_obj(db);
                return NULL;
        }
        
        return db;
}

void delete_database(database_t *db)
{
        if (db) {
                database_unload(db);
                if (db->path)
                        mem_free(db->path);
                delete_obj(db);
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
        //log_debug("database_unload @1");
        if (db->scans) {
                //log_debug("database_unload @2");
                for (list_t *l = db->scans; l != NULL; l = list_next(l)) {
                        //log_debug("database_unload @2.0");
                        scan_t *scan = list_get(l, scan_t);
                        //log_debug("database_unload @2.1");
                        delete_scan(scan);
                }
                //log_debug("database_unload @2.2");
                delete_list(db->scans);
                db->scans = NULL;
        }
        //log_debug("database_unload @3");
        return 0;
}
        
int database_load(database_t *db)
{
        int err = 0;
        if (database_unload(db) != 0) {
                log_warn("database_unload: Unload failed");
                return -1;
        }

        list_t *ids = dir_list(db->path);
        for (list_t *l = ids; l != NULL; l = list_next(l)) {
                char *id = list_get(l, char);
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
                mem_free(list_get(l, char));

        return err;
}

int database_create_scan_dir(database_t *db, const char *id)
{
        char path[1024];
        
        snprintf(path, 1024, "%s/%s", db->path, id);
        path[1023] = 0;
	if (mkdir(path, 0777) == -1) {
                log_err("Failed to create directory '%s': %s.",
                        path, strerror(errno));
                return -1;
	}

        log_info("new scan '%s' in directory '%s'", id, path);

        snprintf(path, 1024, "%s/%s/metadata", db->path, id);
        path[1023] = 0;
	if (mkdir(path, 0777) == -1) {
                log_err("Failed to create directory '%s': %s.",
                        path, strerror(errno));
                return -1;
	}
        return 0;
}

scan_t *database_new_scan(database_t *db)
{
        char id[256];
        clock_datetime(id, sizeof(id), '-', '_', '-');
        if (database_create_scan_dir(db, id) != 0)
                return NULL;
        scan_t *scan = new_scan(db, id);
	if (scan == NULL) return NULL;
	
	if (database_add_scan(db, scan) != 0) {
                delete_scan(scan);
                return NULL;
	}
        return scan;
}

int database_create_fileset_dir(database_t *db, scan_t *scan, const char *id)
{
        char path[1024];
        
        snprintf(path, 1024, "%s/%s/%s", db->path, scan->id, id);
        path[1023] = 0;
	if (mkdir(path, 0777) == -1) {
                log_err("Failed to create directory '%s': %s.",
                        path, strerror(errno));
                return -1;
	}

        snprintf(path, 1024, "%s/%s/metadata/%s", db->path, scan->id, id);
        path[1023] = 0;
	if (mkdir(path, 0777) == -1) {
                log_err("Failed to create directory '%s': %s.",
                        path, strerror(errno));
                return -1;
	}
        
        return 0;
}

int database_get_file_path(database_t *db,
                           scan_t *scan,
                           fileset_t *fileset,
                           file_t *file,
                           char *buffer, int len)
{
        snprintf(buffer, len, "%s/%s/%s/%s", db->path, scan->id, fileset->id, file->name);
        buffer[len-1] = 0;
        return 0;
}

int database_get_file_metadata_path(database_t *db,
                                    scan_t *scan,
                                    fileset_t *fileset,
                                    file_t *file,
                                    char *buffer, int len)
{
        snprintf(buffer, len, "%s/%s/metadata/%s/%s.json",
                 db->path, scan->id, fileset->id, file->id);
        buffer[len-1] = 0;
        return 0;
}

int database_get_scan_directory(database_t *db,
                                scan_t *scan,
                                char *buffer, int len)
{
        snprintf(buffer, len, "%s/%s", db->path, scan->id);
        buffer[len-1] = 0;
        return 0;
}

int database_get_scan_json_file(database_t *db,
                                scan_t *scan,
                                char *buffer, int len)
{
        snprintf(buffer, len, "%s/%s/files.json", db->path, scan->id);
        buffer[len-1] = 0;
        return 0;
}

void database_print(database_t *db)
{
        for (list_t *l = db->scans; l != NULL; l = list_next(l)) {
                scan_t *scan = list_get(l, scan_t);
                scan_print(scan);
        }
}
