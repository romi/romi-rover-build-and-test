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

json_object_t global_env;

////////////////////////////////////////////////////////

static int status_error(json_object_t reply, membuf_t *message)
{
        int ret = -1;
        const char *status = json_object_getstr(reply, "status");
        if (status == NULL) {
                r_err("invalid status");
                membuf_printf(message, "invalid status");
                
        } else if (rstreq(status, "ok")) {
                ret = 0;
                
        } else if (rstreq(status, "error")) {
                const char *s = json_object_getstr(reply, "message");
                r_err("message error: %s", s);
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
        
        r_debug("Sending homing");

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

        r_debug("Sending moveat %d", speed);

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

        r_debug("Sending move %f", distance);

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

        r_debug("Sending hoe %s", method);

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
        
        r_debug("Sending start_recording");

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
        
        r_debug("Sending stop_recording");

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

/*         r_debug("Sending shutdown"); */

/*         if (app_standalone()) { */
/*                 clock_sleep(1); */
/*                 return 0; */
/*         } */
        
/*         reply = messagelink_send_command_f(_shutdown, "{'command':'shutdown'}"); */
/*         r_debug("shutdown replied: %.*s", message_len(reply), message_data(reply)); */
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
        script_t *script = r_new(script_t);
        script->actions = json_null();
        script->name = r_strdup(name);
        script->display_name = r_strdup(display_name);
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
                if (script->name) r_free(script->name);
                if (script->display_name) r_free(script->display_name);
                json_unref(script->actions);
                r_delete(script);
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

//////////////////////////////////////////////////////

static json_object_t eval(json_object_t obj, list_t *environments);
static json_object_t eval_expr(const char *expr, list_t* environments);
static json_object_t eval_string(json_object_t str, list_t* environments);
static json_object_t eval_object(json_object_t obj, list_t *environments);
static json_object_t eval_array(json_object_t obj, list_t *environments);

static json_object_t eval_expr(const char *expr, list_t* environments)
{
        for (list_t *l = environments; l != NULL; l = list_next(l)) {
                json_object_t env = list_get(l, base_t);
                json_object_t value = json_object_get(env, expr);
                if (!json_falsy(value))
                        return value;
        }
        r_err("Failed to evaluate the expression '%s'", expr);
        return json_null();
}

// Check out https://github.com/codeplea/tinyexpr
static json_object_t eval_string(json_object_t str, list_t* environments)
{
        const char *s = json_string_value(str);
        
        if (strchr(s, '$') == NULL)
                return json_string_create(s);

        int len = strlen(s);
        if (s[0] == '$' && s[1] == '(' && s[len-1] == ')') {
                char *expr = r_alloc(len-2);
                if (expr == NULL) return json_null();
                memcpy(expr, s+2, len-3);
                expr[len-3] = '\0';
                json_object_t value = eval_expr(expr, environments);
                json_ref(value);
                r_free(expr);
                return value;
                
        } else {
                return json_string_create(s);
        }
}

static int32 eval_object_field(const char* key, json_object_t value, void* data)
{
        list_t* stuff = (list_t*) data;
        json_object_t obj = list_get(stuff, base_t);
        list_t* environments = list_next(stuff);
        json_object_t new_value = eval(value, environments);
        if (json_isundefined(new_value))
                return -1;
        json_object_set(obj, key, new_value);
        json_unref(new_value);
        return 0;
}

static json_object_t eval_object(json_object_t obj, list_t *environments)
{
        json_object_t o = json_object_create();
        list_t *stuff = list_prepend(environments, o);
        int err = json_object_foreach(obj, eval_object_field, stuff);
        if (err != 0) {
                json_unref(o);
                delete1_list(stuff);
                return json_null();
        }
        delete1_list(stuff);
        return o;
}

static json_object_t eval_array(json_object_t obj, list_t *environments)
{
        json_object_t a = json_array_create();
        for (int i = 0; i < json_array_length(obj); i++) {
                json_object_t e = json_array_get(obj, i);
                e = eval(e, environments);
                if (json_isnull(e)) {
                        json_unref(a);
                        return json_null();
                }
                json_array_push(a, e);
                json_unref(e);
        }
        return a;
}

static json_object_t eval(json_object_t obj, list_t *environments)
{
        if (json_isnumber(obj)) {
                return json_number_create(json_number_value(obj));

        } else if (json_isstring(obj)) {
                return eval_string(obj, environments);
                
        } else if (json_isarray(obj)) {
                return eval_array(obj, environments);
                
        } else if (json_isobject(obj)) {
                return eval_object(obj, environments);
                
        } else {
                return obj;
        }
}

//////////////////////////////////////////////////////

static int do_action(const char *name, json_object_t action,
                     list_t *environments, membuf_t *message)
{
        int err;
        /* json_object_t args; */
        
        /* args = eval(action, environments); */
        /* if (json_isnull(args)) { */
        /*         return -1; */
        /* } */
        
        if (rstreq(name, "homing")) {
                err = homing(message);

        } else if (rstreq(name, "start_recording")) {
                err = start_recording(message);

        } else if (rstreq(name, "stop_recording")) {
                err = stop_recording(message);

        /* } else if (rstreq(name, "shutdown")) */
        /*         return shutdown_rover(message); */

        } else if (rstreq(name, "move")) {
                double distance = json_object_getnum(action, "distance");
                if (isnan(distance)) {
                        r_err("Action 'move': invalid distance");
                        return -1;
                }
                double speed = 100.0;
                if (json_object_has(action, "speed")) {
                        speed = json_object_getnum(action, "speed");
                        if (isnan(speed) || speed < -1000 || speed > 1000) {
                                r_err("Action 'move': invalid distance");
                                return -1;
                        }
                }
                err = move(message, distance, speed);
                
        } else if (rstreq(name, "moveat")) {
                double speed = json_object_getnum(action, "speed");
                if (isnan(speed) || speed < -1000 || speed > 1000) {
                        r_err("Action 'moveat': invalid speed");
                        return -1;
                }
                err = moveat(message, (int) speed);
                
        } else if (rstreq(name, "hoe")) {
                const char *method = json_object_getstr(action, "method");
                if (method == NULL) {
                        r_err("Action 'hoe': invalid method");
                        return -1;
                }
                if (!rstreq(method, "quincunx") && !rstreq(method, "boustrophedon")) {
                        r_err("Action 'hoe': invalid method");
                        return -1;
                }
                err = hoe(message, method);
        } else {
                r_err("Unknown action: '%s'", name);
                membuf_printf(message, "Unknown action: '%s'", name);
                err = -1;
        }

        /* json_unref(action); */
        return err;
}

static int execute(json_object_t actions, list_t *environments, membuf_t *message)
{
        int err = 0;

        for (int i = 0; i < json_array_length(actions); i++) {
                
                json_object_t action = json_array_get(actions, i);
                
                const char *action_name = json_object_getstr(action, "action");
                if (action_name == NULL) {
                        membuf_printf(message, "script step has no action field");
                        err = -1;
                        break;
                }
                
                status = action_name; 
                r_debug("action: %s", action_name);
                
                do_action(action_name, action, environments, message);
        }

        return err;
}

void _run_script(script_t *script)
{
        r_debug("_run_script");

        membuf_t *message = new_membuf();
        
        mutex_lock(mutex);
        progress = 0;
        status = "idle";
        current_script = script;
        mutex_unlock(mutex);

        list_t *environments = list_prepend(NULL, global_env);

        if (execute(script->actions, environments, message) != 0) {
                membuf_append_zero(message);
                r_err("Script '%s' returned an error: %s",
                        script->name, membuf_data(message));
                moveat(message, 0); // FIXME: raise alert to robot!
        }

        delete_list(environments);
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
        r_debug("run_script");
        mutex_lock(mutex);
        if (thread == NULL) {
                r_debug("run_script: creating new thread");
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

        r_debug("load_script_file %s", filename);
        
        scripts = json_load(filename, &err, errmsg, sizeof(errmsg));
        if (err != 0) {
                r_err("Failed to load scripts file: %s", errmsg);
                r_err("File: '%s'", filename);
                json_unref(scripts);
                return NULL;
        }
        if (!json_isarray(scripts)) {
                r_err("Scripts file doesn't contain an array. File: '%s'", filename);
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
                        r_err("Script is not an object. File: '%s'", filename);
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
                                r_err("Script %i has no name. File: '%s'", i, filename);
                        else if (display_name == NULL) 
                                r_err("Script %i has no disply name. File: '%s'",
                                        i, filename);
                        else r_err("Script %i: script filed is not an array. File: '%s'",
                                     i, filename);
                        json_unref(scripts);
                        delete_script_list(list);
                        return NULL;
                }
                r_debug("Adding script '%s'", name);
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

        r_debug("set_server_dir: %s", path);

        // FIXME!
        if (path[0] != '/') {
                char cwd[PATH_MAX];
                if (getcwd(cwd, PATH_MAX) == NULL) {
                        char reason[200];
                        strerror_r(errno, reason, 200);
                        r_err("getcwd failed: %s", reason);
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
                r_err("realpath failed: %s", reason);
                r_err("path: %s", initial_path);
                return -1;
        }
        if (stat(resolved_path, &statbuf) != 0) {
                char reason[200];
                strerror_r(errno, reason, 200);
                r_err("stat failed: %s", reason);
                return -1;
        }
        if ((statbuf.st_mode & S_IFMT) != S_IFDIR) {
                r_err("Not a directory: %s", resolved_path);
                return -1;
        }
        
        server_dir = r_strdup(resolved_path);
        r_info("Serving files from directory %s", server_dir);
        return 0;
}

static int set_script_path(const char *path)
{
        if (script_path != NULL) {
                r_free(script_path);
                script_path = NULL;
        }
        script_path = r_strdup(path);
        if (script_path == NULL)
                return -1;

        r_info("Script file: %s", path);
        return 0;
}

static int get_configuration()
{
        if (server_dir != NULL && script_path != NULL)
                return 0;
        
        r_debug("trying to configure the interface");

        global_env = client_get("configuration", "");
        if (json_falsy(global_env)) {
                r_err("failed to load the configuration");
                return -1;
        }
        
        json_object_t config = json_object_get(global_env, "interface");

        if (!json_isobject(config)) {
                r_err("failed to load the interface configuration");
                json_unref(config);
                return -1;
        }
        
        const char *html = json_object_getstr(config, "html");
        const char *script_file = json_object_getstr(config, "scripts");
        if (html == NULL || script_file == NULL) {
                r_err("invalid configuration");
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
                        r_log_set_writer((log_writer_t) log_writer,
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
        r_log_set_writer((log_writer_t) log_writer, NULL);
        
        if (argc == 3) {
                if (set_server_dir(argv[1]) != 0)
                        return -1;
                if (set_script_path(argv[2]) != 0)
                        return -1;
        }
        
        for (int i = 0; i < 10; i++) {
                if (init() == 0)
                        return 0;
                r_err("init failed: attempt %d/10", i);
                clock_sleep(0.2);
        }

        r_err("failed to initialize the interface");        
        return -1;
}

void interface_cleanup()
{
        if (server_dir)
                r_free(server_dir);
        if (script_path)
                r_free(script_path);
        if (scripts) {
                for (list_t *l = scripts; l != NULL; l = list_next(l)) {
                        script_t *script = list_get(l, script_t);
                        delete_script(script);
                }
                delete_list(scripts);
        }
        if (mutex != NULL)
                delete_mutex(mutex);
        json_unref(global_env);
}

/////////////////////////////////////////////////////

static int send_file(const char *path, const char *mimetype, request_t *request)
{
        FILE *fp = fopen(path, "r");
        if (fp == NULL) {
                r_err("Failed to open %s", path);
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
                        r_err("Failed to read %s", path);
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
                r_err("realpath failed: %s", reason);
                r_err("path: %s", requested_path);
                return -1;
        }
        if (strncmp(server_dir, resolved_path, strlen(server_dir)) != 0) {
                r_err("File not in server path: %s", resolved_path);
                return -1;
        }
        if (stat(resolved_path, &statbuf) != 0) {
                char reason[200];
                strerror_r(errno, reason, 200);
                r_err("stat failed: %s", reason);
                return -1;
        }
        if ((statbuf.st_mode & S_IFMT) != S_IFREG) {
                r_err("Not a regular file: %s", resolved_path);
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
                request_set_status(request, HTTP_Status_Internal_Server_Error);
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
                request_set_status(request, HTTP_Status_Internal_Server_Error);
                return -1;
        }
        return send_local_file("index.html", request);
}

int interface_scripts(void *data, request_t *request)
{
        if (init() != 0) {
                request_set_status(request, HTTP_Status_Internal_Server_Error);
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
                request_set_status(request, HTTP_Status_Internal_Server_Error);
                return -1;
        }

        const char *arg = request_args(request);
        if (arg == NULL) {
                request_set_status(request, HTTP_Status_Bad_Request);
                return -1;
        }

        r_debug("checking for script '%s'", arg);

        script_t *script = find_script(scripts, arg);
        if (script == NULL) {
                request_set_status(request, HTTP_Status_Not_Found);
                return -1;
        }

        int r = run_script(script);
        if (r == -1) {
                request_set_status(request, HTTP_Status_Internal_Server_Error);
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
                             "      <div id=\"nodes\">\n"
                             "      Didn't load the information from the registry, yet.\n"
                             "      </div>\n"
                             "    </div>\n"
                             "    <div id=\"main\" class=\"container\">\n"
                             "      <ul class=\"nav nav-pills green\" id=\"viewer-nav\">\n"
                             "      </ul>\n"
                             "      <div class=\"tab-content clearfix\" id=\"viewers\">\n"
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

int interface_db(void *data, request_t *request)
{
        addr_t *addr;
        char b[52];
        char c[52];

        addr = registry_get_service("db");
        addr_string(addr, b, 52);
        delete_addr(addr);
        
        addr = registry_get_messagehub("db");
        addr_string(addr, c, 52);
        delete_addr(addr);

        request_reply_printf(request,
"<!DOCTYPE html>\n"
"<html lang=\"en\">\n"
"  <head>\n"
"    <meta charset=\"utf-8\">\n"
"    <title>Database</title>\n"
"    <link rel=\"stylesheet\" href=\"css/bootstrap.min.css\">\n"
"    <link rel=\"stylesheet\" href=\"css/db.css\">\n"
"  </head>\n"
"  <body>\n"
"    <div id=\"main\" class=\"container\">\n"
"      <ul class=\"nav nav-pills\" id=\"browser\" role=\"tablist\">\n"
"        <li class=\"nav-item\">\n"
"          <a class=\"nav-link active\" id=\"db-tab\" data-toggle=\"tab\" href=\"#db\"\n"
"             role=\"tab\" aria-controls=\"db\" aria-selected=\"true\">DB</a>\n"
"        </li>\n"
"        <li class=\"nav-item\">\n"
"          <a class=\"nav-link\" id=\"scan-tab\" data-toggle=\"tab\" href=\"#scan\"\n"
"             role=\"tab\" aria-controls=\"scan\" aria-selected=\"false\">-</a>\n"
"        </li>\n"
"        <li class=\"nav-item\">\n"
"          <a class=\"nav-link\" id=\"fileset-tab\" data-toggle=\"tab\" href=\"#fileset\"\n"
"             role=\"tab\" aria-controls=\"fileset\" aria-selected=\"false\">-</a>\n"
"        </li>\n"
"        <li class=\"nav-item\">\n"
"          <a class=\"nav-link\" id=\"file-tab\" data-toggle=\"tab\" href=\"#file\"\n"
"             role=\"tab\" aria-controls=\"file\" aria-selected=\"false\">-</a>\n"
"        </li>\n"
"      </ul>\n"
"      <div class=\"tab-content\" id=\"myTabContent\">\n"
"        <div class=\"tab-pane show active\" id=\"db\" role=\"tabpanel\" aria-labelledby=\"db-tab\">\n"
"          Database has not been loaded, yet.</div>\n"
"        <div class=\"tab-pane\" id=\"scan\" role=\"tabpanel\" aria-labelledby=\"scan-tab\">\n"
"          Select a scan in the DB tab.</div>\n"
"        <div class=\"tab-pane\" id=\"fileset\" role=\"tabpanel\" aria-labelledby=\"fileset-tab\">\n"
"          Select a fileset in the scan tab.</div>\n"
"        <div class=\"tab-pane\" id=\"file\" role=\"tabpanel\" aria-labelledby=\"file-tab\">\n"
"          Select a file in the fileset tab.</div>\n"
"      </div>\n"
"    </div>\n"
"    <script src=\"js/jquery.min.js\"></script>\n"
"    <script src=\"js/bootstrap.min.js\"></script>\n"
"    <script src=\"js/db.js\"></script>\n"
"    <script src=\"js/svg.min.js\"></script>\n"
"    <script>\n"
"      $(document).ready(function() { showDB(\"http://%s\", \"ws://%s\"); });\n"
"    </script>\n"
"  </body>\n"
"</html>\n",
                             b, c);
        return 0;
}

/* int interface_listen(void *data, request_t *request) */
/* { */
/*         if (request_args(request) == NULL) { */
/*                 return -1; */
/*         } */

/*         request_reply_printf(request, */
/*                              "<!DOCTYPE html>\n" */
/*                              "<html lang=\"en\">\n" */
/*                              "  <head>\n" */
/*                              "    <meta charset=\"utf-8\">\n" */
/*                              "    <title>Registry</title>\n" */
/*                              "    <link rel=\"stylesheet\" href=\"index.css\">\n" */
/*                              "    <script src=\"js/jquery.min.js\"></script>\n" */
/*                              "    <script src=\"js/bootstrap.min.js\"></script>\n" */
/*                              "    <script src=\"js/listen.js\"></script>\n" */
/*                              "  </head>\n" */
/*                              "  <body>\n" */
/*                              "    <div id=\"registry\"></div>\n" */
/*                              "    <script>\n" */
/*                              "      $(document).ready(function() { listenTo(\"ws://%s\"); });\n" */
/*                              "    </script>\n" */
/*                              "  </body>\n" */
/*                              "</html>\n", */
/*                              request_args(request));         */
/*         return 0; */
/* } */
