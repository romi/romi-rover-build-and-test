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
#include <stdlib.h>
#include <unistd.h>
#include "webproxy.h"

// For MAX_PATH
#include <linux/limits.h>

// For errno...
#include <errno.h>

static int interface_initialized = 0;
static char *server_dir = NULL;


void broadcast_log(void *userdata, const char* s)
{
        messagelink_t *get_messagelink_logger();
        messagelink_t *log = get_messagelink_logger();
        if (log)
                messagelink_send_str(log, s);
}

/***************************************************************************/

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
                rprintf(initial_path, PATH_MAX, "%s/%s", cwd, path);
                initial_path[PATH_MAX-1] = 0;
        } else {
                rprintf(initial_path, PATH_MAX, "%s", path);
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

static int get_configuration()
{
        if (server_dir != NULL)
                return 0;

        r_debug("trying to configure the interface");

        json_object_t global_env = client_get("configuration", "");
        if (json_falsy(global_env)) {
                r_err("failed to load the configuration");
                return -1;
        }

        json_object_t config = json_object_get(global_env, "proxy");

        if (!json_isobject(config)) {
                r_err("failed to load the proxy configuration");
                json_unref(global_env);
                return -1;
        }

        const char *html = json_object_getstr(config, "html");
        if (html == NULL) {
                r_err("invalid configuration");
                json_unref(global_env);
                return -1;
        }

        if (set_server_dir(html) != 0) {
                json_unref(global_env);
                return -1;
        }

        json_unref(global_env);

        return 0;
}

static int init()
{
        if (interface_initialized)
                return 0;

        if (get_configuration() != 0)
                return -1;

        interface_initialized = 1;
        return 0;
}

static int check_path(const char *filename, char *path, int len, response_t *response)
{
        char requested_path[PATH_MAX];
        char resolved_path[PATH_MAX];
        struct stat statbuf;

        snprintf(requested_path, PATH_MAX, "%s/%s", server_dir, filename);
        requested_path[PATH_MAX-1] = 0;

        if (realpath(requested_path, resolved_path) == NULL) {
                char reason[200];
                strerror_r(errno, reason, 200);
                if (errno == EACCES)
                        response_set_status(response, HTTP_Status_Forbidden);
                else if (errno == ENOENT || errno == ENOTDIR)
                        response_set_status(response, HTTP_Status_Not_Found);
                else
                        response_set_status(response, HTTP_Status_Internal_Server_Error);
                r_err("realpath failed: %s", reason);
                r_err("path: %s", requested_path);
                return -1;
        }
        if (strncmp(server_dir, resolved_path, strlen(server_dir)) != 0) {
                r_err("File not in server path: %s", resolved_path);
                response_set_status(response, HTTP_Status_Forbidden);
                return -1;
        }
        if (stat(resolved_path, &statbuf) != 0) {
                char reason[200];
                strerror_r(errno, reason, 200);
                if (errno == EACCES)
                        response_set_status(response, HTTP_Status_Forbidden);
                else if (errno == ENOENT || errno == ENOTDIR)
                        response_set_status(response, HTTP_Status_Not_Found);
                else
                        response_set_status(response, HTTP_Status_Internal_Server_Error);
                r_err("stat failed: %s", reason);
                return -1;
        }
        if ((statbuf.st_mode & S_IFMT) != S_IFREG) {
                r_err("Not a regular file: %s", resolved_path);
                response_set_status(response, HTTP_Status_Forbidden);
                return -1;
        }
        snprintf(path, len, "%s", resolved_path);
        return 0;
}

static void send_file(const char *path, const char *mimetype,
                      request_t *request, response_t *response)
{
        FILE *fp = fopen(path, "r");
        if (fp == NULL) {
                r_err("Failed to open %s", path);
                response_set_status(response, HTTP_Status_Not_Found);
                return;
        }

        response_set_mimetype(response, mimetype);

        while (1) {
                size_t num;
                char buffer[256];
                num = fread(buffer, 1, 256, fp);
                response_append(response, buffer, num);
                if (feof(fp)) break;
                if (ferror(fp)) {
                        r_err("Failed to read %s", path);
                        fclose(fp);
                        response_set_status(response, HTTP_Status_Internal_Server_Error);
                        return;
                }
        }
        fclose(fp);
}

/***************************************************************************/

int webproxy_init(int argc, char **argv)
{
        r_log_set_writer(broadcast_log, NULL);

        if (argc == 2) {
                if (set_server_dir(argv[1]) != 0)
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

void webproxy_cleanup()
{
        if (server_dir)
                r_free(server_dir);
}

/***************************************************************************/

void send_local_file(const char *filename, request_t *request, response_t *response)
{
        char path[PATH_MAX];

        r_info("send_local_file: %s", filename);

        const char *mimetype = filename_to_mimetype(filename);

        if (check_path(filename, path, PATH_MAX, response) == 0)
                send_file(path, mimetype, request, response);
}

void webproxy_send_index(void *data, request_t *request, response_t *response)
{
        if (init() != 0) {
                response_set_status(response, HTTP_Status_Internal_Server_Error);
                return;
        }
        send_local_file("index.html", request, response);
}

void webproxy_send_file(void *data, request_t *request, response_t *response)
{
        if (init() != 0) {
                response_set_status(response, HTTP_Status_Internal_Server_Error);
                return;
        }

	const char *filename = request_uri(request);
        r_debug("uri: %s", request_uri(request));

        if (rstreq(filename, "/"))
                filename = "index.html";
        else if (filename[0] == '/')
                filename++;

        send_local_file(filename, request, response);
}

static const char *list_nth_string(list_t *list, int n)
{
        list_t* l = list_nth(list, n);
        return (l == NULL)? NULL : list_get(l, const char);
}

static int make_uri(char *buffer, int len, request_t *request, list_t* elements)
{
        char path[1024];
        int err = path_glue(elements, 1, path, sizeof(path));

        if (err == 0) {
                if (request_args(request) != NULL)
                        rprintf(buffer, len, "%s?%s", path, request_args(request));
                else
                        rprintf(buffer, len, "%s", path);
        }
        return err;
}

void webproxy_redirect(void *data, request_t *request, response_t *response)
{
        list_t* elements = path_break(request_uri(request));

        // Expecting the following pattern: /<type>/<topic>/<dir1>/.../<file>
        // The elements list should contain the following strings:
        //  "/"->"<type>"->"<topic>"->"<dir1>"->...->"<file>"

        if (list_size(elements) >= 4) {

                char resource[1024];
                const char *slash = list_nth_string(elements, 0);
                const char *type = list_nth_string(elements, 1);
                const char *topic = list_nth_string(elements, 2);
                int err = make_uri(resource, sizeof(resource),
                                   request, list_nth(elements, 3));

                if ((slash == NULL)
                    || !rstreq(slash, "/")
                    || (type == NULL)
                    || !rstreq(type, "service")
                    || (topic == NULL)
                    || (err != 0)
                    || (strlen(resource) == 0)) {
                        response_set_status(response, HTTP_Status_Bad_Request);
                } else {
                        client_get_data(topic, resource, &response);
                }

        } else {
                response_set_status(response, HTTP_Status_Bad_Request);
        }

        path_delete(elements);
}

/***************************************************************************/

/* static int *webproxy_onpart(void *userdata, */
/*                             const unsigned char *data, int len, */
/*                             const char *mimetype, */
/*                             double timestamp) */
/* { */
/*         return 0; */
/* } */

/* int webproxy_stream_onresponse(void *userdata, response_t *response) */
/* { */
/*         return 0; */
/* } */

/* int webproxy_stream_ondata(multipart_parser_t *parser, response_t *response, */
/*                            const char *buf, int len) */
/* { */
/*         multipart_parser_process(parser, response, buf, len); */
/*         return 0; */
/* } */

/* void webproxy_get_stream(request_t *request, */
/*                          const char *topic, */
/*                          const char *resource, */
/*                          response_t *response) */
/* { */
/*         multipart_parser_t *parser = NULL; */

/*         parser = new_multipart_parser(NULL, NULL, */
/*                                       (multipart_onpart_t) webproxy_onpart); */

/*         streamerlink_t *link = NULL; */
/*         link = registry_open_streamerlink("webproxy", */
/*                                           topic, */
/*                                           webproxy_stream_ondata, */
/*                                           webproxy_stream_onresponse, */
/*                                           parser, */
/*                                           1); */
/*         if (link == NULL) { */
/*                 response_set_status(response, HTTP_Status_Internal_Server_Error); */
/*                 return; */
/*         } */

/* } */

/***************************************************************************/

void webproxy_onrequest(void *data,
                        request_t *request,
                        response_t *response)
{
	const char *path = request_uri(request);
        if (strncmp(path, "/service/", 9) == 0)
                webproxy_redirect(data, request, response);
        else
                webproxy_send_file(data, request, response);
}

/***************************************************************************/

static void copy_message_to_client(messagelink_t *link_from_client,
                                   messagelink_t *link_to_hub,
                                   json_object_t message)
{
        if (link_from_client) {
                {
                        char buffer[1024];
                        json_tostring(message, buffer, sizeof(buffer));
                        r_debug("copy_message_to_client: %s", buffer);
                }
                messagelink_send_obj(link_from_client, message);
        }
}

static void copy_message_to_hub(messagelink_t *link_to_hub,
                                messagelink_t *link_from_client,
                                json_object_t message)
{
        if (link_to_hub) {
                {
                        char buffer[1024];
                        json_tostring(message, buffer, sizeof(buffer));
                        r_debug("copy_message_to_hub: %s", buffer);
                }
                messagelink_send_obj(link_to_hub, message);
        }
}

static void link_from_client_closed(messagelink_t *link_to_hub,
                                    messagelink_t *link_from_client)
{
        r_debug("link_from_client_closed");
        registry_close_messagelink(link_to_hub);
        messagelink_set_userdata(link_from_client, NULL);
}

static int webproxy_redirect_websocket(messagelink_t *link_from_client, const char *topic)
{
        messagelink_t *link_to_hub = NULL;
        link_to_hub = registry_open_messagelink("proxy-to-hub",
                                                topic,
                                                (messagelink_onmessage_t) copy_message_to_client,
                                                link_from_client);
        if (link_to_hub != NULL) {
                messagelink_set_userdata(link_from_client, link_to_hub);
                messagelink_set_onmessage(link_from_client,
                                          (messagelink_onmessage_t) copy_message_to_hub);
                messagelink_set_onclose(link_from_client,
                                        (messagelink_onclose_t) link_from_client_closed);
        }

        return (link_to_hub == NULL)? -1 : 0;
}

int webproxy_onconnect(void *userdata, messagehub_t *hub,
                       request_t* request, messagelink_t *link_from_client)
{
        list_t* elements = path_break(request_uri(request));
        int err = -1;

        // Expecting the following pattern: /messagehub/<topic>
        // The elements list should contain the following strings:
        //  "/" -> "messagehub" -> "<topic>"

        if (list_size(elements) == 3) {
                const char *slash = list_nth_string(elements, 0);
                const char *type = list_nth_string(elements, 1);
                const char *topic = list_nth_string(elements, 2);

                if ((slash != NULL)
                    && rstreq(slash, "/")
                    && (type != NULL)
                    && rstreq(type, "messagehub")
                    && (topic != NULL)
                    && (strlen(topic) > 0)) {

                        err = webproxy_redirect_websocket(link_from_client, topic);

                } else {
                        r_debug("webproxy_onconnect: Invalid path: %s",
                                request_uri(request));
                }

        } else {
                r_debug("webproxy_onconnect: Wrong path length: %s",
                        request_uri(request));
        }

        path_delete(elements);
        return err;
}
