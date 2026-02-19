#include <event2/event.h>
#include <event2/http.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <string>

#include "camera_info.hpp"

// Server context
struct ServerContext {
    struct event_base *base;
    int total_requests;
};

// --- Module 1: TCP SOCKET (text protocol) ---

void tcp_read_cb(struct bufferevent *bev, void *ctx) {
    struct evbuffer *input = bufferevent_get_input(bev);
    char *line;
    size_t n;

    while ((line = evbuffer_readln(input, &n, EVBUFFER_EOL_ANY))) {
        if (strcmp(line, "GET_INFO") == 0) {
            CameraInfo info = InfoCollector::collect();
            std::string response = info.toJson() + "\n";
            bufferevent_write(bev, response.c_str(), response.length());
        } else {
            bufferevent_write(bev, "UNKNOWN_CMD\n", 12);
        }
        free(line);
    }
}

void tcp_event_cb(struct bufferevent *bev, short events, void *ctx) {
    if (events & (BEV_EVENT_EOF | BEV_EVENT_ERROR)) {
        bufferevent_free(bev);
    }
}

void tcp_accept_cb(struct evconnlistener *listener, evutil_socket_t fd,
                   struct sockaddr *address, int socklen, void *ctx) {
    struct event_base *base = (struct event_base *)ctx;
    struct bufferevent *bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
    bufferevent_setcb(bev, tcp_read_cb, NULL, tcp_event_cb, NULL);
    bufferevent_enable(bev, EV_READ | EV_WRITE);
}

// --- MODULE 2: HTTP API ---

// Handler /api/info
void http_info_api_cb(struct evhttp_request *req, void *arg) {
    ServerContext *ctx = (ServerContext *)arg;
    ctx->total_requests++;

    CameraInfo info = InfoCollector::collect();
    std::string json = info.toJson();
    
    struct evbuffer *buf = evbuffer_new();
    evbuffer_add(buf, json.c_str(), json.length());

    evhttp_add_header(evhttp_request_get_output_headers(req), "Content-Type", "application/json");
    evhttp_send_reply(req, HTTP_OK, "OK", buf);
    evbuffer_free(buf);
}

// Common handler (404 and others)
void http_generic_cb(struct evhttp_request *req, void *arg) {
    evhttp_send_error(req, HTTP_NOTFOUND, "Not Found");
}

int main() {
    openlog("TiXerver", LOG_PID | LOG_CONS, LOG_USER);

    struct event_base *base = event_base_new();
    ServerContext ctx = { base, 0 };

    // 1. Настройка HTTP сервера
    struct evhttp *http = evhttp_new(base);
    // Привязываем конкретный обработчик к пути
    evhttp_set_cb(http, "/api/info", http_info_api_cb, &ctx);
    // Все остальное пойдет в generic (например, 404)
    evhttp_set_gencb(http, http_generic_cb, &ctx);

    if (evhttp_bind_socket(http, "0.0.0.0", 8085) != 0) {
        syslog(LOG_CRIT, "Failed to bind HTTP port 8085");
        return 1;
    }

    // 2. Настройка TCP сервера (чистый сокет)
    struct sockaddr_in sin = {};
    sin.sin_family = AF_INET;
    sin.sin_port = htons(8888); // Порт для текстовых команд

    struct evconnlistener *listener = evconnlistener_new_bind(
        base, tcp_accept_cb, base,
        LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, -1,
        (struct sockaddr*)&sin, sizeof(sin));

    if (!listener) {
        syslog(LOG_CRIT, "Failed to bind TCP port 8888");
        return 1;
    }

    syslog(LOG_INFO, "TiXerver started. HTTP: 8085, TCP: 8888");
    event_base_dispatch(base);

    evconnlistener_free(listener);
    evhttp_free(http);
    event_base_free(base);
    closelog();
    return 0;
}
