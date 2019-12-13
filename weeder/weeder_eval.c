#include <getopt.h>
#include <romi.h>
#include "weeding.h"

static workspace_t workspace;

file_t *get_file(fileset_t *fileset, const char *name)
{
        int num = fileset_count_files(fileset);
        for (int i = 0; i < num; i++) {
                file_t *file = fileset_get_file_at(fileset, i);
                const char *s = file_get_metadata_str(file, "name");
                if (rstreq(name, s))
                        return file;
        }
        return NULL;
}

image_t *load_image(fileset_t *fileset, const char *name)
{
        file_t *file = get_file(fileset, name);
        if (file == NULL) {
                fprintf(stderr, "Could not find the image '%s'\n", name);
                return NULL;
        }

        char path[1024];
        if (file_path(file, path, sizeof(path)) != 0)
                return NULL;

        image_t *image = image_load(path);
        if (image == NULL)
                return NULL;

        return image;
}

image_t *load_computed_mask(fileset_t *fileset)
{
        return load_image(fileset, "mask");
}

static file_t *create_file(fileset_t *fileset, const char *name)
{
        file_t *file = fileset_new_file(fileset);
        if (file == NULL)
                return NULL;
        file_set_timestamp(file, clock_time());
        file_set_metadata_str(file, "name", name);
        return file;
}

image_t *import_manual_mask(fileset_t *fileset, const char *filename)
{
        image_t *image = image_load(filename);
        if (image == NULL)
                return NULL;
        
        file_t *file = get_file(fileset, "mask-manual");
        if (file == NULL)
                file = create_file(fileset, "mask-manual");
        if (file == NULL)
                return image;
        
        if (file != NULL) {
                membuf_t *buffer = new_membuf();
                if (buffer != NULL) {
                        if (image_store_to_mem(image, buffer, "png") == 0)
                                file_import_png(file,
                                                membuf_data(buffer),
                                                membuf_len(buffer));
                        delete_membuf(buffer);
                }
        }

        return image;
}

image_t *get_manual_mask(fileset_t *fileset, const char *filename)
{
        if (filename != NULL)
                return import_manual_mask(fileset, filename);
        else 
                return load_image(fileset, "mask-manual");
}


int main(int argc, char **argv)
{
        const char *db_path = ".";
        const char *session_id = NULL;
        const char *fileset_id = NULL;
        const char *mask_file = NULL;
        int option_index;
        static char *optchars = "d:s:f:";
        static struct option long_options[] = {
                {"db", required_argument, 0, 'd'},
                {"session", required_argument, 0, 's'},
                {"fileset", required_argument, 0, 'f'},
                {0, 0, 0, 0}
        };

        while (1) {
                int c = getopt_long(argc, argv, optchars, long_options, &option_index);
                if (c == -1) break;
                switch (c) {
                case 'd': db_path = optarg; break;
                case 's': session_id = optarg; break;
                case 'f': fileset_id = optarg; break;
                }
        }

        if (optind < argc) 
                mask_file = argv[optind];;
        
        if (session_id == NULL
            || fileset_id == NULL) {
                printf("Usage: weeder_compute [options] mask-image\n");
                printf("Options:\n");
                printf("--session=ID, -s ID: the scan where the fileset is stored\n");
                printf("--fileset=ID, -f ID: the ID of the weeder fileset\n");
                printf("--db=path, -d path: location of the database (default: local dir)\n");
                return -1;
        }
        
        database_t *db = new_database(db_path);
        if (db == NULL)
                return 1;
        
        if (database_load(db) != 0)
                return 1;
        
        scan_t *session = database_get_scan(db, session_id);
        if (session == NULL) {
                fprintf(stderr, "Could not find scan/session with ID '%s'\n", session_id);
                return 1;
        }
        
        fileset_t *fileset = scan_get_fileset(session, fileset_id);
        if (fileset == NULL) {
                fprintf(stderr, "Could not find fileset with ID '%s'\n", fileset_id);
                return 1;
        }

        image_t *computed_mask = load_computed_mask(fileset);
        if (computed_mask == NULL)
                return 1;

        image_t *manual_mask = get_manual_mask(fileset, mask_file);
        if (manual_mask == NULL)
                return 1;

        if (manual_mask->width != computed_mask->width
            || manual_mask->height != computed_mask->height) {
                fprintf(stderr, "The two mask have different sizes: %dx%d vs %dx%d\n",
                        manual_mask->width, manual_mask->height,
                        computed_mask->width, computed_mask->height);
                delete_image(manual_mask);
                delete_image(computed_mask);
                delete_database(db);
                return 1;
        }
        
        int x, y;
        int true_positives = 0;
        int true_negatives = 0;
        int false_positives = 0;
        int false_negatives = 0;
        int total_pixels = manual_mask->width * manual_mask->height;
        
        image_t *results = new_image_rgb(manual_mask->width, manual_mask->height);
        image_clear(results);

        
        for (y = 0; y < manual_mask->height; y++) {
                for (x = 0; x < manual_mask->width; x++) {
                        int manual_classified = 
                                (image_get(manual_mask, x, y, 0) > 0.0f
                                 || image_get(manual_mask, x, y, 1) > 0.0f
                                 || image_get(manual_mask, x, y, 2) > 0.0f);
                        int weeder_classified = 
                                (image_get(computed_mask, x, y, 0) > 0.0f);
                        if (manual_classified && weeder_classified) {
                                true_positives++;
                        } else if (!manual_classified && !weeder_classified)
                                true_negatives++;
                        else if (!manual_classified && weeder_classified) {
                                false_positives++;
                                image_set(results, x, y, 1, 1.0f);
                        } else if (manual_classified && !weeder_classified) {
                                false_negatives++;
                                image_set(results, x, y, 0, 1.0f);
                        }
                }
        }
        
        file_t *file = get_file(fileset, "mask-evaluation");
        if (file != NULL)
                file_import_image(file, results, "png");
        else
                fileset_store_image(fileset, "mask-evaluation", results, "png");
        
        double iou_crop = ((double) true_positives
                           / (true_positives + false_positives + false_negatives));
        double iou_soil = ((double) true_negatives
                           / (true_negatives + false_negatives + false_positives));
        
        json_object_t obj = json_object_create();
        json_object_setnum(obj, "true-positives", true_positives);
        json_object_setnum(obj, "true-negatives", true_negatives);
        json_object_setnum(obj, "false-positives", false_positives);
        json_object_setnum(obj, "false-negatives", false_negatives);
        json_object_setnum(obj, "pixel-count", total_pixels);
        json_object_setnum(obj, "iou-crop", iou_crop);
        json_object_setnum(obj, "iou-soil", iou_soil);
        json_object_setnum(obj, "iou", 0.5 * (iou_soil + iou_crop));
        json_object_setnum(obj, "intersection-over-union", 0.5 * (iou_soil + iou_crop));
        fileset_set_metadata(fileset, "mask-evaluation", obj);
        
        printf("True positives:          %d\n", true_positives);
        printf("True negatives:          %d\n", true_negatives);
        printf("False positives:         %d\n", false_positives);
        printf("False negatives:         %d\n", false_negatives);
        printf("Number of pixels:        %d\n", total_pixels);
        printf("IoU(crop):               %0.3f\n", iou_crop);
        printf("IoU(soil):               %0.3f\n", iou_soil);
        printf("IoU:                     %0.3f\n", 0.5 * (iou_soil + iou_crop));
        delete_database(db);
        return 0;
}
