#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <limits.h>

#include <romi.h>

#include "interface.h"

typedef struct _script_t script_t;

static int interface_initialized = 0;
static list_t *scripts = NULL;
static char *server_dir = NULL;
static char *script_path = NULL;
static int idle = 1;
static int recording = 0;
static int progress = 0;
static const char *status = "idle";
static script_t *current_script = NULL;
static double position = 0.0;
static mutex_t *mutex;
thread_t *thread = NULL;

service_t *get_service_interface();
messagelink_t *get_messagelink_motorcontroller();
messagelink_t *get_messagelink_weeder();
messagelink_t *get_messagelink_camera_recorder();
messagelink_t *get_messagelink_navigation();
messagehub_t *get_messagehub_log_interface();

////////////////////////////////////////////////////////

static int status_error(json_object_t reply, membuf_t *message)
{
        int ret = -1;
        const char *status = json_object_getstr(reply, "status");
        if (status == NULL) {
                log_err("invalid status");
                membuf_printf(message, "invalid status");
                
        } else if (rstreq(status, "ok")) {
                ret = 0;
                
        } else if (rstreq(status, "error")) {
                const char *s = json_object_getstr(reply, "message");
                log_err("message error: %s", s);
                membuf_printf(message, "%s", s);
        }
        json_unref(reply);
        return ret;
}

static int homing(membuf_t *message)
{
        messagelink_t *motors = get_messagelink_motorcontroller();
        json_object_t reply;
        int err;
        
        log_debug("Sending homing");

        if (app_standalone()) {
                clock_sleep(1);
                return 0;
        }
        
        reply = messagelink_send_command_f(motors, "{'command':'homing'}");
        return status_error(reply, message);
}

static int moveat(membuf_t *message, int speed)
{
        messagelink_t *motors = get_messagelink_motorcontroller();
        json_object_t reply;

        log_debug("Sending moveat %d", speed);

        if (app_standalone()) {
                clock_sleep(1);
                return 0;
        }
        
        reply = messagelink_send_command_f(motors, "{'command':'moveat','speed':%d}",
                                           speed);
        return status_error(reply, message);
}

static int move(membuf_t *message, double distance, double speed)
{
        messagelink_t *navigation = get_messagelink_navigation();
        json_object_t reply;

        log_debug("Sending move %f", distance);

        if (app_standalone()) {
                clock_sleep(1);
                return 0;
        }
        
        reply = messagelink_send_command_f(navigation,
                                  "{'command':'move',"
                                  "'distance':%f,"
                                  "'speed':%f}",
                                  distance, speed);
        return status_error(reply, message);
}

static int hoe(membuf_t *message, const char *method)
{
        messagelink_t *weeder = get_messagelink_weeder();
        json_object_t reply;

        log_debug("Sending hoe %s", method);

        if (app_standalone()) {
                clock_sleep(1);
                return 0;
        }
        
        reply = messagelink_send_command_f(weeder, "{'command':'hoe','method':'%s'}",
                                           method);
        return status_error(reply, message);
}

static int start_recording(membuf_t *message)
{
        messagelink_t *recorder = get_messagelink_camera_recorder();
        json_object_t reply;
        
        log_debug("Sending start_recording");

        if (app_standalone()) {
                clock_sleep(1);
                return 0;
        }
        
        reply = messagelink_send_command_f(recorder, "{'command':'start'}");

        int err = status_error(reply, message);
        if (err = 0) recording = 1;
        
        return err;
}
                  
static int stop_recording(membuf_t *message)
{
        messagelink_t *recorder = get_messagelink_camera_recorder();
        json_object_t reply;
        
        log_debug("Sending stop_recording");

        if (app_standalone()) {
                clock_sleep(1);
                return 0;
        }
        
        reply = messagelink_send_command_f(recorder, "{'command':'stop'}");
        int err = status_error(reply, message);
        if (err = 0) recording = 0;
        
        return err;
}

/* static int shutdown_rover(membuf_t *message) */
/* { */
/*         messagelink_t *link = get_messagelink_shutdown(); */
/*         json_object_t reply; */

/*         log_debug("Sending shutdown"); */

/*         if (app_standalone()) { */
/*                 clock_sleep(1); */
/*                 return 0; */
/*         } */
        
/*         reply = messagelink_send_command_f(_shutdown, "{'command':'shutdown'}"); */
/*         log_debug("shutdown replied: %.*s", message_len(reply), message_data(reply)); */
/*         return 0; */
/* } */

////////////////////////////////////////////////////////

typedef struct _script_t {
        char *name;
        char *display_name;
        json_object_t actions;
} script_t;

static void delete_script(script_t *script);

static script_t *new_script(const char *name,
                            const char *display_name,
                            json_object_t actions)
{
        script_t *script = new_obj(script_t);
        script->actions = json_null();
        script->name = mem_strdup(name);
        script->display_name = mem_strdup(display_name);
        if (script->name == NULL || script->display_name == NULL) {
                delete_script(script);
                return NULL;
        }
        script->actions = actions;
        json_ref(actions);
        return script;
}

static void delete_script(script_t *script)
{
        if (script) {
                if (script->name) mem_free(script->name);
                if (script->display_name) mem_free(script->display_name);
                json_unref(script->actions);
                delete_obj(script);
        }
}

static void delete_script_list(list_t *list)
{
        list_t *l = list;
        script_t *script;
        while (l) {
                script = list_get(l, script_t);
                delete_script(script);
                l = list_next(l);
        }
        delete_list(list);
}

static script_t *find_script(list_t *list, const char *name)
{
        while (list) {
                script_t *script = list_get(list, script_t);
                if (rstreq(name, script->name))
                        return script;
                list = list_next(list);
        }
        return NULL;
}

static int do_action(const char *name, json_object_t obj, membuf_t *message)
{
        if (rstreq(name, "homing")) 
                return homing(message);

        if (rstreq(name, "start_recording"))
                return start_recording(message);

        if (rstreq(name, "stop_recording"))
                return stop_recording(message);

        /* if (rstreq(name, "shutdown")) */
        /*         return shutdown_rover(message); */

        if (rstreq(name, "move")) {
                double distance = json_object_getnum(obj, "distance");
                if (isnan(distance)) {
                        log_err("Action 'move': invalid distance");
                        return -1;
                }
                double speed = 100.0;
                if (json_object_has(obj, "speed")) {
                        speed = json_object_getnum(obj, "speed");
                        if (isnan(speed) || speed < -1000 || speed > 1000) {
                                log_err("Action 'move': invalid distance");
                                return -1;
                        }
                }
                return move(message, distance, speed);
        }

        if (rstreq(name, "moveat")) {
                double speed = json_object_getnum(obj, "speed");
                if (isnan(speed) || speed < -1000 || speed > 1000) {
                        log_err("Action 'moveat': invalid speed");
                        return -1;
                }
                return moveat(message, (int) speed);
        }

        if (rstreq(name, "hoe")) {
                const char *method = json_object_getstr(obj, "method");
                if (method == NULL) {
                        log_err("Action 'hoe': invalid method");
                        return -1;
                }
                if (!rstreq(method, "quincunx") && !rstreq(method, "boustrophedon")) {
                        log_err("Action 'hoe': invalid method");
                        return -1;
                }
                return hoe(message, method);
        }

        log_err("Unknown action: '%s'", name);
        membuf_printf(message, "Unknown action: '%s'", name);
        return -1;
}

void _run_script(script_t *script)
{
        log_debug("_run_script");

        membuf_t *message = new_membuf();
        
        mutex_lock(mutex);
        progress = 0;
        status = "idle";
        current_script = script;
        mutex_unlock(mutex);
        
        for (int i = 0; i < json_array_length(script->actions); i++) {
                json_object_t obj = json_array_get(script->actions, i);
                const char *name = json_object_getstr(obj, "action");
                if (name == NULL) {
                        log_err("Invalid action! Script '%s', action %d", name, i);
                        moveat(message, 0); // FIXME: raise alert to robot!
                        break;
                }
                status = name; 
                log_debug("_run_script: %s", name);
                do_action(name, obj, message);
                progress = 100 * (i+1) / json_array_length(script->actions);
        }

        delete_membuf(message);
                
        mutex_lock(mutex);
        idle = 1;
        progress = 100;
        status = "idle";
        current_script = NULL;
        delete_thread(thread);
        thread = NULL;
        mutex_unlock(mutex);
}

int run_script(script_t *script)
{
        int err = 0;
        log_debug("run_script");
        mutex_lock(mutex);
        if (thread == NULL) {
                log_debug("run_script: creating new thread");
                thread = new_thread((thread_run_t) _run_script, script, 0, 0);
                if (thread == NULL) 
                        err = -1;
        } else {
                err = -2;
        }
        mutex_unlock(mutex);
        return err;
}

static list_t *load_script_file(const char *filename)
{
        int err;
        char errmsg[256];
        json_object_t scripts;
        list_t *list = NULL;

        log_debug("load_script_file %s", filename);
        
        scripts = json_load(filename, &err, errmsg, sizeof(errmsg));
        if (err != 0) {
                log_err("Failed to load scripts file: %s", errmsg);
                log_err("File: '%s'", filename);
                json_unref(scripts);
                return NULL;
        }
        if (!json_isarray(scripts)) {
                log_err("Scripts file doesn't contain an array. File: '%s'", filename);
                json_unref(scripts);
                return NULL;
        }

        for (int i = 0; i < json_array_length(scripts); i++) {
                const char* name;
                const char* display_name;
                json_object_t script;
                json_object_t actions;

                script = json_array_get(scripts, i);
                if (!json_isobject(script)) {
                        log_err("Script is not an object. File: '%s'", filename);
                        json_unref(scripts);
                        delete_script_list(list);
                        return NULL;
                }

                name = json_object_getstr(script, "name");
                display_name = json_object_getstr(script, "display_name");
                actions = json_object_get(script, "script");
                if (name == NULL
                    || display_name == NULL
                    || !json_isarray(actions)) {
                        if (name == NULL) 
                                log_err("Script %i has no name. File: '%s'", i, filename);
                        else if (display_name == NULL) 
                                log_err("Script %i has no disply name. File: '%s'",
                                        i, filename);
                        else log_err("Script %i: script filed is not an array. File: '%s'",
                                     i, filename);
                        json_unref(scripts);
                        delete_script_list(list);
                        return NULL;
                }
                log_debug("Adding script '%s'", name);
                script_t *s = new_script(name, display_name, actions);
                list = list_append(list, s);
        }
        
        json_unref(scripts);
        return list;
}

////////////////////////////////////////////////////////

static int set_server_dir(const char *path)
{
        char initial_path[PATH_MAX];
        char resolved_path[PATH_MAX];
        struct stat statbuf;

        log_debug("set_server_dir: %s", path);

        // FIXME!
        if (path[0] != '/') {
                char cwd[PATH_MAX];
                if (getcwd(cwd, PATH_MAX) == NULL) {
                        char reason[200];
                        strerror_r(errno, reason, 200);
                        log_err("getcwd failed: %s", reason);
                        return -1;
                }
                snprintf(initial_path, PATH_MAX, "%s/%s", cwd, path);
                initial_path[PATH_MAX-1] = 0;
        } else {
                snprintf(initial_path, PATH_MAX, "%s", path);
                initial_path[PATH_MAX-1] = 0;
        }

        if (realpath(initial_path, resolved_path) == NULL) {
                char reason[200];
                strerror_r(errno, reason, 200);
                log_err("realpath failed: %s", reason);
                log_err("path: %s", initial_path);
                return -1;
        }
        if (stat(resolved_path, &statbuf) != 0) {
                char reason[200];
                strerror_r(errno, reason, 200);
                log_err("stat failed: %s", reason);
                return -1;
        }
        if ((statbuf.st_mode & S_IFMT) != S_IFDIR) {
                log_err("Not a directory: %s", resolved_path);
                return -1;
        }
        
        server_dir = mem_strdup(resolved_path);
        log_info("Serving files from directory %s", server_dir);
        return 0;
}

static int set_script_path(const char *path)
{
        if (script_path != NULL) {
                mem_free(script_path);
                script_path = NULL;
        }
        script_path = mem_strdup(path);
        if (script_path == NULL)
                return -1;

        log_info("Script file: %s", path);
        return 0;
}

static int get_configuration()
{
        if (server_dir != NULL && script_path != NULL)
                return 0;
        
        log_debug("trying to configure the interface");
        
        json_object_t config = client_get("configuration", "interface");

        if (!json_isobject(config)) {
                log_err("failed to load the configuration");
                json_unref(config);
                return -1;
        }
        
        const char *html = json_object_getstr(config, "html");
        const char *script_file = json_object_getstr(config, "scripts");
        if (html == NULL || script_file == NULL) {
                log_err("invalid configuration");
                json_unref(config);
                return -1;
        }

        if (set_server_dir(html) != 0) {
                json_unref(config);
                return -1;
        }

        if (set_script_path(script_file) != 0) {
                json_unref(config);
                return -1;
        }
                        
        json_unref(config);
        return 0;
}

static int init()
{
        if (interface_initialized)
                return 0;
        
        if (get_configuration() != 0)
                return -1;
        
        scripts = load_script_file(script_path);
        if (scripts == NULL)
                return -1;
        
        interface_initialized = 1;
        return 0;
}

static void log_writer(messagehub_t *hub, const char* line)
{
        static int _inside_log_writer = 0;
        
        // FIXME
        if (hub == NULL) {
                if (get_messagehub_log_interface() != NULL)
                        set_log_writer((log_writer_t) log_writer,
                                       get_messagehub_log_interface());
                return;
        }
        if (_inside_log_writer)
                return;
        
        _inside_log_writer = 1;
        messagehub_broadcast_str(hub, NULL, line);
        _inside_log_writer = 0;
}

int interface_init(int argc, char **argv)
{
        mutex = new_mutex();
        if (mutex == NULL)
                return -1;

        // FIXME: the messagehub has not been created yet when
        // interface_init is called.
        set_log_writer((log_writer_t) log_writer, NULL);
        
        if (argc == 3) {
                if (set_server_dir(argv[1]) != 0)
                        return -1;
                if (set_script_path(argv[2]) != 0)
                        return -1;
        }
        
        for (int i = 0; i < 10; i++) {
                if (init() == 0)
                        return 0;
                log_err("init failed: attempt %d/10", i);
                clock_sleep(0.2);
        }

        log_err("failed to initialize the interface");        
        return -1;
}

void interface_cleanup()
{
        if (server_dir)
                mem_free(server_dir);
        if (script_path)
                mem_free(script_path);
        if (scripts) {
                for (list_t *l = scripts; l != NULL; l = list_next(l)) {
                        script_t *script = list_get(l, script_t);
                        delete_script(script);
                }
                delete_list(scripts);
        }
        if (mutex != NULL)
                delete_mutex(mutex);
}

/////////////////////////////////////////////////////

static int send_file(const char *path, const char *mimetype, request_t *request)
{
        FILE *fp = fopen(path, "r");
        if (fp == NULL) {
                log_err("Failed to open %s", path);
                return -1;
        }

        request_set_mimetype(request, mimetype);

        while (1) {
                size_t num;
                char buffer[256];
                num = fread(buffer, 1, 256, fp);
                request_reply_append(request, buffer, num);
                if (feof(fp)) break;
                if (ferror(fp)) {
                        log_err("Failed to read %s", path);
                        fclose(fp);
                        return -1;
                }
        }
        fclose(fp);
        return 0;
}

static int check_path(const char *filename, char *path, int len)
{
        char requested_path[PATH_MAX];
        char resolved_path[PATH_MAX];
        struct stat statbuf;
        
        snprintf(requested_path, PATH_MAX, "%s/%s", server_dir, filename);
        requested_path[PATH_MAX-1] = 0;

        if (realpath(requested_path, resolved_path) == NULL) {
                char reason[200];
                strerror_r(errno, reason, 200);
                log_err("realpath failed: %s", reason);
                log_err("path: %s", requested_path);
                return -1;
        }
        if (strncmp(server_dir, resolved_path, strlen(server_dir)) != 0) {
                log_err("File not in server path: %s", resolved_path);
                return -1;
        }
        if (stat(resolved_path, &statbuf) != 0) {
                char reason[200];
                strerror_r(errno, reason, 200);
                log_err("stat failed: %s", reason);
                return -1;
        }
        if ((statbuf.st_mode & S_IFMT) != S_IFREG) {
                log_err("Not a regular file: %s", resolved_path);
                return -1;
        }
        snprintf(path, len, "%s", resolved_path);
        return 0;
}


int send_local_file(const char *filename, request_t *request)
{
        if (init() != 0)
                return -1;

        char path[PATH_MAX];
        
        const char *mimetype = filename_to_mimetype(filename);
        if (mimetype == NULL)
                return -1;
        
        if (check_path(filename, path, PATH_MAX) != 0)
                return -1;
        
        return send_file(path, mimetype, request);
}

/////////////////////////////////////////////////////

int interface_local_file(void *data, request_t *request)
{
        if (init() != 0) {
                request_set_status(request, 500);
                return -1;
        }

	const char *filename = request_uri(request);
        
        if (rstreq(filename, "/"))
                filename = "index.html";
        else if (filename[0] == '/')
                filename++;
        
        return send_local_file(filename, request);
}

int interface_index(void *data, request_t *request)
{
        if (init() != 0) {
                request_set_status(request, 500);
                return -1;
        }
        return send_local_file("index.html", request);
}

int interface_scripts(void *data, request_t *request)
{
        if (init() != 0) {
                request_set_status(request, 500);
                return -1;
        }

        request_reply_printf(request, "[");
        
        for (list_t *l = scripts; l != NULL; l = list_next(l)) {
                script_t *script = list_get(l, script_t);
                request_reply_printf(request,
                                     "{\"name\":\"%s\",\"display_name\":\"%s\"}",
                                     script->name, script->display_name);
                ;
                if (list_next(l) != NULL) 
                        request_reply_printf(request, ",");
        }

        request_reply_printf(request, "]");
        return 0;
}

int interface_status(void *data, request_t *request)
{
        request_reply_printf(request,
                             "{\"status\": \"%s\", "
                             "\"position\": %f, "
                             "\"progress\": %d, "
                             "\"recording\": %d",
                             status, position, progress, recording);

        if (current_script != NULL) {
                request_reply_printf(request,
                                     ", "
                                     "\"script\": {\"name\":\"%s\", "
                                     "\"display_name\": \"%s\"}}",
                                     current_script->name,
                                     current_script->display_name);
        } else {
                request_reply_printf(request, "}");
        }

        return 0;
}

int interface_execute(void *data, request_t *request)
{
        if (init() != 0) {
                request_set_status(request, 500);
                return -1;
        }

        const char *arg = request_args(request);
        if (arg == NULL) {
                request_set_status(request, 400);
                return -1;
        }

        log_debug("checking for script '%s'", arg);

        script_t *script = find_script(scripts, arg);
        if (script == NULL) {
                request_set_status(request, 404);
                return -1;
        }

        int r = run_script(script);
        if (r == -1) {
                request_set_status(request, 500);
                return -1;
        }

        return send_local_file("script.html", request);
}

int interface_registry(void *data, request_t *request)
{
        request_reply_printf(request,
                             "<!DOCTYPE html>\n"
                             "<html lang=\"en\">\n"
                             "  <head>\n"
                             "    <meta charset=\"utf-8\">\n"
                             "    <title>Registry</title>\n"
                             "    <link rel=\"stylesheet\" href=\"css/bootstrap.min.css\">\n"
                             "    <link rel=\"stylesheet\" href=\"css/registry.css\">\n"
                             "  </head>\n"
                             "  <body>\n"
                             "    <div id=\"main\" class=\"container\">\n"
                             "      <ul class=\"nav nav-pills green\">\n"
                             "        <li class=\"nav-item\"><a class=\"nav-link active\" href=\"#nodes-tab\" data-toggle=\"tab\">Nodes</a></li>\n"
                             "        <li class=\"nav-item\"><a class=\"nav-link\" href=\"#topics-tab\" data-toggle=\"tab\">Topics</a></li>\n"
                             "      </ul>\n"
                             "      <div class=\"tab-content clearfix\">\n"
                             "        <div class=\"tab-pane active\" id=\"nodes-tab\">\n"
                             "          Didn't load the information from the registry, yet.\n"
                             "        </div>\n"
                             "        <div class=\"tab-pane\" id=\"topics-tab\">\n"
                             "          Didn't load the information from the registry, yet.\n"
                             "        </div>\n"
                             "      </div>\n"
                             "    </div>\n"
                             "    <script src=\"js/jquery.min.js\"></script>\n"
                             "    <script src=\"js/bootstrap.min.js\"></script>\n"
                             "    <script src=\"js/registry.js\"></script>\n"
                             "    <script src=\"js/svg.min.js\"></script>\n"
                             "    <script>\n"
                             "      $(document).ready(function() { showRegistry(\"ws://%s:%d\"); });\n"
                             "    </script>\n"
                             "  </body>\n"
                             "</html>\n",
                             get_registry_ip(), get_registry_port());
        return 0;
}

int interface_listen(void *data, request_t *request)
{
        if (request_args(request) == NULL) {
                return -1;
        }

        request_reply_printf(request,
                             "<!DOCTYPE html>\n"
                             "<html lang=\"en\">\n"
                             "  <head>\n"
                             "    <meta charset=\"utf-8\">\n"
                             "    <title>Registry</title>\n"
                             "    <link rel=\"stylesheet\" href=\"index.css\">\n"
                             "    <script src=\"js/jquery.min.js\"></script>\n"
                             "    <script src=\"js/bootstrap.min.js\"></script>\n"
                             "    <script src=\"js/listen.js\"></script>\n"
                             "  </head>\n"
                             "  <body>\n"
                             "    <div id=\"registry\"></div>\n"
                             "    <script>\n"
                             "      $(document).ready(function() { listenTo(\"ws://%s\"); });\n"
                             "    </script>\n"
                             "  </body>\n"
                             "</html>\n",
                             request_args(request));        
        return 0;
}
