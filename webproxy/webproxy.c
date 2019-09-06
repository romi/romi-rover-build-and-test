#include <stdlib.h>
#include <unistd.h>
#include "webproxy.h"

int webproxy_init(int argc, char **argv)
{
        return 0;
}

void webproxy_cleanup()
{
}

list_t* chop_path(const char *path)
{
        list_t* elements = NULL;
        membuf_t *buf = new_membuf();
        if (buf == NULL)
                return NULL;

        for (int i = 0; path[i] != 0; i++) {
                char c = path[i];
                if (c == '/') {
                        if (membuf_len(buf) > 0) {
                                membuf_append_zero(buf);
                                char *s = r_strdup(membuf_data(buf));
                                r_debug("chop_path: s=%s", s);
                                elements = list_append(elements, s);
                                membuf_clear(buf);
                        } 
                } else {
                        membuf_append(buf, &c, 1);
                }
        }
        if (membuf_len(buf) > 0) {
                membuf_append_zero(buf);
                char *s = r_strdup(membuf_data(buf));
                elements = list_append(elements, s);
        } 
        return elements;
}

char *glue_path(list_t* elements)
{
        membuf_t *buf = new_membuf();
        for (list_t* l = elements; l != NULL; l = list_next(l)) {
                const char *s = list_get(l, char);
                membuf_append(buf, "/", 1);
                membuf_append_str(buf, s);
        }
        membuf_append_zero(buf);
        return r_strdup(membuf_data(buf));
}

void delete_path(list_t* list)
{
        for (list_t* l = list; l != NULL; l = list_next(l)) {
                char *s = list_get(l, char);
                if (s) r_free(s);
        }
        delete_list(list);
}

int webproxy_get_service(request_t *request, const char *topic, const char *resource)
{
        r_debug("webproxy_get_service: topic %s, resource %s", topic, resource);
        return client_get_data(topic, resource, request_reply_buffer(request));
}

int webproxy_get(void *data, request_t *request)
{
	const char *path = request_uri(request);
        r_debug("webproxy_get: uri=%s", path);
        
        list_t* elements = chop_path(path);

        for (list_t* l = elements; l != NULL; l = list_next(l)) {
                const char *s = list_get(l, char);
                r_debug("%s", s);
        }

        list_t* l_type = elements;
        if (l_type == NULL) {
                request_set_status(request, HTTP_Status_Bad_Request);
                return -1;
        }
        char *type = list_get(l_type, char);
        r_debug("webproxy_get: type %s", type);
        
        list_t* l_topic = list_next(l_type);
        if (l_topic == NULL) {
                request_set_status(request, HTTP_Status_Bad_Request);
                return -1;
        }
        char *topic = list_get(l_topic, char);
        r_debug("webproxy_get: topic %s", topic);
        
        list_t* l_resource = list_next(l_topic);
        if (l_resource == NULL) {
                request_set_status(request, HTTP_Status_Bad_Request);
                return -1;
        }
        char *resource = glue_path(l_resource);
        r_debug("webproxy_get: resource %s", resource);

        int r = 0;
        if (rstreq(type, "service")) {
                r = webproxy_get_service(request, topic, resource);

        } else if (rstreq(type, "kettle")) {
                request_set_status(request, HTTP_Status_Im_A_Teapot);

        } else {
                request_set_status(request, HTTP_Status_Bad_Request);
                r = -1;
        }
        
        delete_path(elements);
        r_free(resource);
        
        return r;
}

