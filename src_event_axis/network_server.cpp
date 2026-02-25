#include "network_server.hpp"

extern "C" {
    #include "cJSON.h"
}

#include <event2/event.h>
#include <event2/http.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <arpa/inet.h>
#include <syslog.h>
#include <stdlib.h>
#include <cstring>

// --- HTTP Обработчики ---

// GET /api/info -> CameraInfo
void http_get_info_cb(struct evhttp_request* req, void* arg) {
    AppContext* ctx = (AppContext*)arg;

    ctx->info = InfoCollector::collect();

    // pthread_mutex_lock(&ctx->info.lock); 
    std::string json = ctx->info.toJson();
    // pthread_mutex_unlock(&ctx->info.lock);

    
    struct evbuffer* buf = evbuffer_new();
    evbuffer_add(buf, json.c_str(), (int)json.length());
    evhttp_add_header(evhttp_request_get_output_headers(req), "Content-Type", "application/json");
    evhttp_send_reply(req, HTTP_OK, "OK", buf);
    evbuffer_free(buf);
}

// POST /api/settings -> Обновление MathSettings
void http_post_settings_cb(struct evhttp_request* req, void* arg) {
    AppContext* ctx = (AppContext*)arg;
    if (evhttp_request_get_command(req) != EVHTTP_REQ_POST) {
        evhttp_send_error(req, HTTP_BADMETHOD, "POST only");
        return;
    }

    struct evbuffer* in_buf = evhttp_request_get_input_buffer(req);
    size_t len = evbuffer_get_length(in_buf);
    if (len > 0) {
        char* raw = (char*)malloc(len + 1);
        evbuffer_remove(in_buf, raw, len);
        raw[len] = '\0';

        cJSON* root = cJSON_Parse(raw);
        if (root) {
            pthread_mutex_lock(&ctx->settings.lock);
            cJSON* thr = cJSON_GetObjectItemCaseSensitive(root, "threshold");
            if (cJSON_IsNumber(thr)) ctx->settings.threshold = thr->valueint;
            
            cJSON* sen = cJSON_GetObjectItemCaseSensitive(root, "sensitivity");
            if (cJSON_IsNumber(sen)) ctx->settings.sensitivity = sen->valueint;
            pthread_mutex_unlock(&ctx->settings.lock);

            syslog(LOG_INFO, "Settings updated via HTTP");
            cJSON_Delete(root);
        }
        free(raw);
    }
    evhttp_send_reply(req, HTTP_OK, "OK", NULL);
}

// --- TCP Обработчики ---

void tcp_read_cb(struct bufferevent* bev, void* arg) {
    AppContext* ctx = (AppContext*)arg;
    struct evbuffer* input = bufferevent_get_input(bev);
    char* line;
    size_t n;

    while ((line = evbuffer_readln(input, &n, EVBUFFER_EOL_CRLF))) {
        std::string command(line);
        free(line); // Освобождаем сразу после копирования в string

        // Диспетчер находит нужную функцию и возвращает ответ
        std::string response = ctx->processor.execute(command, ctx);
        
        bufferevent_write(bev, response.c_str(), response.length());
    }
}

void tcp_accept_cb(struct evconnlistener* l, evutil_socket_t fd, struct sockaddr* s, int sl, void* arg) {
    (void)l; (void)s; (void)sl;
    AppContext* ctx = (AppContext*)arg;
    struct bufferevent* bev = bufferevent_socket_new(ctx->base, fd, BEV_OPT_CLOSE_ON_FREE);
    bufferevent_setcb(bev, tcp_read_cb, NULL, NULL, ctx);
    bufferevent_enable(bev, EV_READ | EV_WRITE);
}

// --- Инициализация сервера ---

bool NetworkServer::start(AppContext* ctx, int http_port, int tcp_port) {
    // Настройка HTTP
    struct evhttp* http = evhttp_new(ctx->base);
    evhttp_set_cb(http, "/api/info", http_get_info_cb, ctx);
    evhttp_set_cb(http, "/api/settings", http_post_settings_cb, ctx);
    if (evhttp_bind_socket(http, "0.0.0.0", (uint16_t)http_port) != 0) return false;

    // Настройка TCP
    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons((uint16_t)tcp_port);
    
    struct evconnlistener* listener = evconnlistener_new_bind(
        ctx->base, tcp_accept_cb, ctx,
        LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, -1,
        (struct sockaddr*)&sin, (int)sizeof(sin));

    return (listener != NULL);
}
