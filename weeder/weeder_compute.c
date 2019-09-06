#include <stdlib.h>
#include <getopt.h>
#include <romi.h>
#include "weeding.h"

static workspace_t workspace;
static double quincunx_threshold = 0.0;

static int init_workspace(const char *config_file)
{
        int err;
        char errmsg[200];
                
        json_object_t config = json_load(config_file, &err, errmsg, sizeof(errmsg));
        if (err != 0) {
                fprintf(stderr, "Failed to load the config file\n");
                fprintf(stderr, "File: %s\n", config_file);
                fprintf(stderr, "Error: %s\n", errmsg);
                return -1;
        }
        
        json_object_t weeder = json_object_get(config, "weeder");
        if (json_falsy(weeder)) {
                fprintf(stderr, "Couldn't find 'weeder' in configuration file\n");
                json_unref(config);
                return -1;
        }

        json_object_t w = json_object_get(weeder, "workspace");
        if (workspace_parse(&workspace, w) != 0) {
                json_unref(config);
                return -1;
        }

        double threshold = json_object_getnum(weeder, "quincunx_threshold");
        if (isnan(threshold) || threshold < 0 || threshold > 1000.0) {
                r_err("Invalid value for 'quincunx_threshold'");
                json_unref(config);
                return -1;
        }
        quincunx_threshold = threshold;
        
        json_unref(config);
        return 0;
}

int main(int argc, char **argv)
{
        const char *db_path = ".";
        const char *session_id = NULL;
        const char *fileset_id = NULL;
        const char *input_file = NULL;
        const char *config_file = NULL;
        double distance_plants = 0.0;
        double distance_rows = 0.0;
        double radius_zones = 0.10;
        double diameter_tool = 0.05;
        int do_print_usage = 0;
        
        int option_index;
        static char *optchars = "d:s:f:C:p:r:z:t:";
        static struct option long_options[] = {
                {"db", required_argument, 0, 'd'},
                {"session", required_argument, 0, 's'},
                {"fileset", required_argument, 0, 'f'},
                {"config", required_argument, 0, 'C'},
                {"plants", required_argument, 0, 'p'},
                {"rows", required_argument, 0, 'r'},
                {"zone-radius", required_argument, 0, 'z'},
                {"tool-diameter", required_argument, 0, 't'},
                {0, 0, 0, 0}
        };

        while (1) {
                int c = getopt_long(argc, argv, optchars, long_options, &option_index);
                if (c == -1) break;
                switch (c) {
                case 'd': db_path = optarg; break;
                case 's': session_id = optarg; break;
                case 'f': fileset_id = optarg; break;
                case 'C': config_file = optarg; break;
                case 'p': distance_plants = atof(optarg); break;
                case 'r': distance_rows = atof(optarg); break;
                case 'z': radius_zones = atof(optarg); break;
                case 't': diameter_tool = atof(optarg); break;
                }
        }

        if (optind < argc) 
                input_file = argv[optind];;

        if (input_file == NULL) {
                printf("Missing input file\n");                
                do_print_usage = 1;
        }
        if (config_file == NULL) {
                printf("Missing config file\n");                
                do_print_usage = 1;
        }
        if (distance_plants == 0.0) {
                printf("Missing plant distance\n");                
                do_print_usage = 1;
        }
        if (distance_plants == 0.0) {
                printf("Missing plant distance\n");                
                do_print_usage = 1;
        }

        if (do_print_usage) {
                printf("Usage: weeder_compute [options]  camera-image\n");
                printf("Options:\n");
                printf("--db=path, -d path: location of the database\n");
                printf("--session=ID, -s ID: the scan where the fileset is stored\n");
                printf("--fileset=ID, -f ID: the ID of the output fileset\n");
                printf("--config=path, -C path: the config file\n");
                printf("--plants=distance, -p value: the distance between plants (m)\n");
                printf("--rows=distance, -r value: the distance between rows (m)\n");
                printf("--zone-radius=value, -z value: the radius of the zones around the plants (m)\n");
                printf("--tool-diameter=value, -t value: the diameter of the weeding tool (m)\n");
                return -1;
        }

        if (distance_plants < 0.01 || distance_plants > 2) {
                printf("Invalid distance\n");
                return -1;
        }

        if (diameter_tool < 0.01 || diameter_tool > 1) {
                printf("Invalid tool diameter\n");
                return -1;
        }

        if (radius_zones < 0.01 || radius_zones > 1) {
                printf("Invalid radius zones\n");
                return -1;
        }

        if (distance_rows == 0)
                distance_rows = distance_plants;

        if (init_workspace(config_file) != 0)
                return 1;
        
        database_t *db = new_database(db_path);
        if (db == NULL)
                return 1;
        
        if (database_load(db) != 0)
                return 1;
        
        scan_t *session = NULL;
        if (session_id != NULL)
                session = database_get_scan(db, session_id);
        if (session == NULL)
                session = database_new_scan(db, session_id);
        if (session == NULL)
                return 1;
        
        image_t *camera = image_load(input_file);
        if (camera == NULL) {
                fprintf(stderr, "Failed to load the image %s", input_file);
                return 1;
        }

        float confidence;
        list_t *path = compute_path(session, fileset_id, camera, &workspace,
                                    distance_plants, distance_rows,
                                    radius_zones, diameter_tool,
                                    &confidence);

        delete_database(db);
        return 0;
}
