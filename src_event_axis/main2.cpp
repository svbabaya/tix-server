#include <event2/event.h>
#include <event2/http.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
// #include <string.h>
#include <syslog.h>
#include <string>

#include "camera_info.hpp"

// Структура контекста сервера
struct ServerContext {
    struct event_base *base;
    int total_requests;
};

// --- МОДУЛЬ 1: TCP СОКЕТ (Чистые текстовые команды) ---

void tcp_read_cb(struct bufferevent *bev, void *ctx) {
    (void)ctx; // Убираем ошибку unused-parameter
    struct evbuffer *input = bufferevent_get_input(bev);
    char *line;
    size_t n;

    // Читаем построчно до символа \n
    while ((line = evbuffer_readln(input, &n, EVBUFFER_EOL_ANY))) {
        if (strcmp(line, "GET_INFO") == 0) {
            CameraInfo info = InfoCollector::collect();
            std::string response = info.toJson() + "\n";
            bufferevent_write(bev, response.c_str(), (int)response.length());
        } else {
            bufferevent_write(bev, "UNKNOWN_CMD\n", 12);
        }
        free(line);
    }
}

void tcp_event_cb(struct bufferevent *bev, short events, void *ctx) {
    (void)ctx;
    if (events & (BEV_EVENT_EOF | BEV_EVENT_ERROR)) {
        bufferevent_free(bev);
    }
}

void tcp_accept_cb(struct evconnlistener *listener, evutil_socket_t fd,
                   struct sockaddr *address, int socklen, void *ctx) {
    (void)listener; (void)address; (void)socklen;
    struct event_base *base = (struct event_base *)ctx;
    
    struct bufferevent *bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
    if (bev) {
        bufferevent_setcb(bev, tcp_read_cb, NULL, tcp_event_cb, NULL);
        bufferevent_enable(bev, EV_READ | EV_WRITE);
    }
}

// --- МОДУЛЬ 2: HTTP API (JSON настройки и инфо) ---

// Обработчик GET /api/info
void http_info_api_cb(struct evhttp_request *req, void *arg) {
    ServerContext *ctx = (ServerContext *)arg;
    ctx->total_requests++;

    CameraInfo info = InfoCollector::collect();
    std::string json = info.toJson();
    
    struct evbuffer *buf = evbuffer_new();
    if (!buf) return;

    evbuffer_add(buf, json.c_str(), (int)json.length());
    evhttp_add_header(evhttp_request_get_output_headers(req), "Content-Type", "application/json");
    evhttp_send_reply(req, HTTP_OK, "OK", buf);
    evbuffer_free(buf);
}

// Обработчик POST /api/settings
void http_settings_api_cb(struct evhttp_request *req, void *arg) {
    (void)arg;
    if (evhttp_request_get_command(req) != EVHTTP_REQ_POST) {
        evhttp_send_error(req, HTTP_BADMETHOD, "Only POST allowed");
        return;
    }

    struct evbuffer *in_buf = evhttp_request_get_input_buffer(req);
    size_t len = evbuffer_get_length(in_buf);
    
    if (len > 0) {
        char *json_data = (char *)malloc(len + 1);
        if (json_data) {
            evbuffer_remove(in_buf, json_data, len);
            json_data[len] = '\0';
            syslog(LOG_INFO, "New settings received: %s", json_data);
            // Здесь добавьте вызов парсера JSON
            free(json_data);
        }
    }

    struct evbuffer *out_buf = evbuffer_new();
    evbuffer_add_printf(out_buf, "{\"status\":\"accepted\"}");
    evhttp_add_header(evhttp_request_get_output_headers(req), "Content-Type", "application/json");
    evhttp_send_reply(req, HTTP_OK, "OK", out_buf);
    evbuffer_free(out_buf);
}

void http_generic_cb(struct evhttp_request *req, void *arg) {
    (void)arg;
    evhttp_send_error(req, HTTP_NOTFOUND, "Not Found");
}

// --- MAIN ---

int main() {
    openlog("TiXerver", LOG_PID | LOG_CONS, LOG_USER);

    struct event_base *base = event_base_new();
    if (!base) {
        syslog(LOG_CRIT, "Failed to init libevent");
        return 1;
    }

    ServerContext ctx;
    ctx.base = base;
    ctx.total_requests = 0;

    // Инициализация HTTP
    struct evhttp *http = evhttp_new(base);
    evhttp_set_cb(http, "/api/info", http_info_api_cb, (void *)&ctx);
    evhttp_set_cb(http, "/api/settings", http_settings_api_cb, (void *)&ctx);
    evhttp_set_gencb(http, http_generic_cb, (void *)&ctx);

    if (evhttp_bind_socket(http, "0.0.0.0", 8085) != 0) {
        syslog(LOG_CRIT, "Failed to bind HTTP port 8085");
        return 1;
    }

    // Инициализация TCP Listener
    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = htons(8888);

    struct evconnlistener *listener = evconnlistener_new_bind(
        base, tcp_accept_cb, (void *)base,
        LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, -1,
        (struct sockaddr*)&sin, (int)sizeof(sin));

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
