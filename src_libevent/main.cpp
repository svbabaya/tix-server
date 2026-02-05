#include <event2/event.h>
#include <event2/http.h>
#include <event2/buffer.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

// Обработчик текстового API
void request_handler(struct evhttp_request *req, void *arg) {
    struct evbuffer *buf = evbuffer_new();
    if (!buf) return;

    // Ответ на запрос (например, "OK" для текстового API)
    evbuffer_add_printf(buf, "Hello! Camera API is active on thread %lu\n", (unsigned long)pthread_self());
    evhttp_send_reply(req, HTTP_OK, "OK", buf);
    evbuffer_free(buf);
}

// Функция, запускаемая в каждом потоке
void *start_worker(void *arg) {
    struct event_base *base = event_base_new();
    struct evhttp *http = evhttp_new(base);

    // Привязка к порту 8080 с поддержкой SO_REUSEPORT (через низкоуровневый bind)
    // Для простоты здесь используем стандартный bind, но в реальности лучше evhttp_bind_socket_with_handle
    evhttp_bind_socket(http, "0.0.0.0", 8080);
    
    // Установка общего колбэка для всех путей
    evhttp_set_gencb(http, request_handler, NULL);

    printf("Thread started...\n");
    event_base_dispatch(base); // Запуск цикла событий

    evhttp_free(http);
    event_base_free(base);
    return NULL;
}

int main() {
    int num_threads = 2; // Например, 2 потока для 2-ядерного SoC камеры
    pthread_t threads[num_threads];

    for (int i = 0; i < num_threads; i++) {
        pthread_create(&threads[i], NULL, start_worker, NULL);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
