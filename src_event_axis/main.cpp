#include <event2/event.h>
#include <event2/http.h>
#include <event2/buffer.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>

#include "camera_info.hpp"

struct server_config {
    const char *server_name;
    int total_requests;
};

void request_handler(struct evhttp_request *req, void *arg) {
    struct server_config *config = (struct server_config *)arg;
    config->total_requests++;

    // 1. Собираем данные
    CameraInfo info = InfoCollector::collect();
    std::string jsonResponse = info.toJson() + "\n";
    struct evbuffer *buf = evbuffer_new();
    if (!buf) return;

    // 2. Добавляем JSON в буфер ответа
    evbuffer_add(buf, jsonResponse.c_str(), jsonResponse.length());

    // 3. Устанавливаем заголовок ответа (Content-Type: application/json)
    evhttp_add_header(evhttp_request_get_output_headers(req), 
                      "Content-Type", "application/json");

    evhttp_send_reply(req, HTTP_OK, "OK", buf);
    evbuffer_free(buf);
    
    syslog(LOG_DEBUG, "Handled request #%d for URI: %s", 
           config->total_requests, evhttp_request_get_uri(req));
}

int main() {
    openlog("TiXerver", LOG_PID | LOG_CONS, LOG_USER);

    struct event_base *base = event_base_new();
    if (!base) {
        syslog(LOG_CRIT, "Could not initialize libevent!");
        return 1;
    }

    struct evhttp *http = evhttp_new(base);
    
    // Инициализация структуры (в C++ стиле)
    server_config config;
    config.server_name = "TiXerver";
    config.total_requests = 0;

    evhttp_set_gencb(http, request_handler, &config);

    if (evhttp_bind_socket(http, "0.0.0.0", 8085) != 0) {
        syslog(LOG_CRIT, "Failed to bind port 8085!");
        return 1;
    }

    syslog(LOG_INFO, "TiXerver started on port 8085");
    
    event_base_dispatch(base);

    evhttp_free(http);
    event_base_free(base);
    closelog();
    return 0;
}
