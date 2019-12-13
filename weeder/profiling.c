#include "profiling.h"

fileset_t *create_fileset(scan_t *session, const char *fileset_id)
{
        if (session == NULL) {
                r_debug("create_fileset: session is null");
                return NULL;
        }
        
        fileset_t *fileset = NULL;
        const char *s = fileset_id;
        char id[64];

        if (fileset_id == NULL) {
                int num = 0;
                while (num < 100000) {
                        rprintf(id, sizeof(id), "weeding-%05d", num);
                        fileset = scan_get_fileset(session, id);
                        if (fileset == NULL)
                                break;
                        num++;
                }
                s = id;
        }

        r_info("Storing weeding files in fileset '%s'", s);

        fileset = scan_get_fileset(session, s);

        if (fileset == NULL)
                fileset = scan_new_fileset(session, s);
        if (fileset == NULL)
                return NULL;
        
        return fileset;
}

void store_jpg(fileset_t *fileset, const char *name, image_t *image)
{
        if (fileset == NULL)
                return;
        fileset_store_image(fileset, name, image, "jpg");
}

void store_png(fileset_t *fileset, const char *name, image_t *image)
{
        if (fileset == NULL)
                return;
        fileset_store_image(fileset, name, image, "png");
}

file_t *create_file(fileset_t *fileset, const char *name)
{
        if (fileset == NULL)
                return NULL;
        file_t *file = fileset_new_file(fileset);
        if (file == NULL)
                return NULL;
        file_set_timestamp(file, clock_time());
        file_set_metadata_str(file, "name", name);
        return file;
}

void store_text(fileset_t *fileset, const char *name, membuf_t *text)
{
        if (fileset == NULL)
                return;
        
        file_t *file = create_file(fileset, name);
        if (file == NULL)
                return;
        
        file_import_data(file, membuf_data(text), membuf_len(text),
                         "text/plain", "txt");
}
