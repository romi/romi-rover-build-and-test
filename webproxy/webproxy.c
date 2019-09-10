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

/***************************************************************************/

void webproxy_get_service(request_t *request,
                          const char *topic,
                          const char *resource,
                          response_t *response)
{
        int err;
        r_debug("webproxy_get_service: topic %s, resource %s", topic, resource);
        if (request_args(request) != NULL) {
                char buffer[1024]; 
                rprintf(buffer, sizeof(buffer), "%s?%s", resource, request_args(request));
                r_debug("   resource %s", buffer);
                err = client_get_data(topic, buffer, &response);
                
        } else
                err = client_get_data(topic, resource, &response);
        
        if (err != 0)
                response_set_status(response, HTTP_Status_Internal_Server_Error);
}

/***************************************************************************/

static int *webproxy_onpart(void *userdata,
                            const unsigned char *data, int len,
                            const char *mimetype,
                            double timestamp)
{
        return 0;
}

int webproxy_stream_onresponse(void *userdata, response_t *response)
{
        return 0;
}

int webproxy_stream_ondata(multipart_parser_t *parser, const char *buf, int len)
{
        multipart_parser_process(parser, buf, len);
        return 0;
}

void webproxy_get_stream(request_t *request,
                         const char *topic,
                         const char *resource,
                         response_t *response)
{
        r_debug("webproxy_get_stream: topic %s, resource %s", topic, resource);

        /* multipart_parser_t *parser = NULL; */

        /* parser = new_multipart_parser(NULL, NULL, */
        /*                               (multipart_onpart_t) webproxy_onpart); */

        /* streamerlink_t *link = NULL; */
        /* link = registry_open_streamerlink("webproxy", */
        /*                                   topic, */
        /*                                   webproxy_stream_ondata,  */
        /*                                   webproxy_stream_onresponse, */
        /*                                   parser, */
        /*                                   1); */
        /* if (link == NULL) { */
        /*         response_set_status(response, HTTP_Status_Internal_Server_Error); */
        /*         return; */
        /* } */

}

/***************************************************************************/

void webproxy_onrequest(void *data,
                        request_t *request,
                        response_t *response)
{
	const char *path = request_uri(request);
        r_debug("webproxy_onrequest: uri=%s", path);
        
        list_t* elements = path_break(path);
        list_t* start = elements;

        char *s = list_get(start, char);
        if (rstreq(s, "/"))
            start = list_next(start);

        list_t* l_type = start;
        if (l_type == NULL) {
                response_set_status(response, HTTP_Status_Bad_Request);
                return;
        }
        
        char *type = list_get(l_type, char);
        r_debug("webproxy_onrequest: type %s", type);
        
        if (rstreq(type, "coffee")) {
                response_set_status(response, HTTP_Status_Im_A_Teapot);
                return;
        }
        
        list_t* l_topic = list_next(l_type);
        if (l_topic == NULL) {
                response_set_status(response, HTTP_Status_Bad_Request);
                return;
        }
        
        char *topic = list_get(l_topic, char);
        r_debug("webproxy_onrequest: topic %s", topic);
        
        list_t* l_resource = list_next(l_topic);
        if (l_resource == NULL) {
                response_set_status(response, HTTP_Status_Bad_Request);
                return;
        }

        char resource[1024];
        int err = path_glue(l_resource, 1, resource, sizeof(resource));
        r_debug("webproxy_onrequest: resource %s", resource);

        int r = 0;
        if (rstreq(type, "service")) {
                webproxy_get_service(request, topic, resource, response);

        } else {
                r_debug("webproxy_onrequest: Bad resquest: Unknown type '%s'", type);
                response_set_status(response, HTTP_Status_Bad_Request);
        }
        
        path_delete(elements);
}

/***************************************************************************/

static void copy_message_to_client(messagelink_t *link_from_client,
                                   messagelink_t *link_to_hub,
                                   json_object_t message)
{
        r_debug("copy_message_to_client");
        if (link_from_client)
                messagelink_send_obj(link_from_client, message);
}

static void copy_message_to_hub(messagelink_t *link_to_hub,
                                messagelink_t *link_from_client,
                                json_object_t message)
{
        r_debug("copy_message_to_hub");
        if (link_to_hub)
                messagelink_send_obj(link_to_hub, message);
}

static void link_from_client_closed(messagelink_t *link_to_hub,
                                    messagelink_t *link_from_client)
{
        r_debug("link_from_client_closed");
        registry_close_messagelink(link_to_hub);
        messagelink_set_userdata(link_from_client, NULL);
}

int webproxy_onconnect(void *userdata, messagehub_t *hub,
                       request_t* request, messagelink_t *link_from_client)
{
        const char *path = request_uri(request);
        r_debug("webproxy_onconnect: uri '%s'", path);
        
        list_t* elements = path_break(path);
        list_t* start = elements;

        char *s = list_get(start, char);
        if (rstreq(s, "/"))
            start = list_next(start);

        list_t* l_type = start;
        if (l_type == NULL)
                return -1;
        
        char *type = list_get(l_type, char);
        r_debug("webproxy_onconnect: type %s", type);

        if (!rstreq(type, "messagehub"))
                return -1;
        
        list_t* l_topic = list_next(l_type);
        if (l_topic == NULL)
                return -1;
        
        char *topic = list_get(l_topic, char);
        r_debug("webproxy_onconnect: topic %s", topic);
        
        messagelink_t *link_to_hub = NULL;
        link_to_hub = registry_open_messagelink("webproxy",
                                                topic,
                                                (messagelink_onmessage_t) copy_message_to_client,
                                                link_from_client);
        if (link_to_hub == NULL) {
                path_delete(elements);
                return -1;
        }
        
        messagelink_set_userdata(link_from_client, link_to_hub);
        messagelink_set_onmessage(link_from_client,
                                  (messagelink_onmessage_t) copy_message_to_hub);
        messagelink_set_onclose(link_from_client,
                                (messagelink_onclose_t) link_from_client_closed);

        path_delete(elements);
        return 0;
}

