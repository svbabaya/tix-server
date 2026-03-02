#include <event2/event.h>
#include <event2/http.h>
#include <event2/buffer.h>
#include <stdio.h>
#include <stdlib.h>

struct server_config {
    char *server_name;
    int total_requests;
};

void request_handler(struct evhttp_request *req, void *arg) {
    struct server_config *config = (struct server_config *)arg;
    config->total_requests++;

    struct evbuffer *buf = evbuffer_new();
    if (!buf) return;

    evbuffer_add_printf(buf, "Server: %s\n", config->server_name);
    evbuffer_add_printf(buf, "Total requests: %d\n", config->total_requests);
    evbuffer_add_printf(buf, "URI: %s\n", evhttp_request_get_uri(req));

    evhttp_send_reply(req, HTTP_OK, "OK", buf);
    evbuffer_free(buf);
}

int main() {
    struct event_base *base = event_base_new();
    struct evhttp *http = evhttp_new(base);

    struct server_config config = { .server_name = "My_Libevent_Server", .total_requests = 0 };

    evhttp_set_gencb(http, request_handler, &config);

    evhttp_bind_socket_with_handle(http, "0.0.0.0", 8085);
    printf("Server started on port 8085...\n");
    
    event_base_dispatch(base);

    evhttp_free(http);
    event_base_free(base);
    return 0;
}
