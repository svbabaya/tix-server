#include <pthread.h>
#include <syslog.h>
#include <event2/event.h>

#include "app_context.hpp"
#include "network_server.hpp"
#include "math_engine.hpp"

int main() {
    openlog("TiXerver", LOG_PID | LOG_CONS, LOG_USER);

    // daemon(nochdir, noclose)
    // 0, 0 означает: перейти в корень и закрыть stdout/stderr/stdin
    // if (daemon(0, 0) == -1) {
    //     syslog(LOG_ERR, "Failed to become a daemon");
    //     return 1;
    // }

    // 1. Инициализация базового цикла libevent
    struct event_base* base = event_base_new();
    if (!base) {
        syslog(LOG_CRIT, "Failed to initialize libevent base!");
        return 1;
    }

    // 2. Создание единого контекста данных (Shared Context)
    // Передаем указатель на base, чтобы сервер мог регистрировать события
    AppContext app(base);
    
    // Заполняем статичные данные о камере при старте или делаем это при каждом запросе, поскольку не все данные статичные
    // app.info = InfoCollector::collect(); 

    // 3. Запуск сетевого модуля (HTTP: 8085, TCP: 8095)
    // Сервер получает указатель на app, чтобы менять настройки и читать результаты
    if (!NetworkServer::start(&app, 8085, 8095)) {
        syslog(LOG_CRIT, "Failed to start network services!");
        event_base_free(base);
        return 1;
    }

    // 4. Запуск математического модуля в отдельном потоке
    pthread_t math_thread_id;
    // Передаем &app как аргумент, чтобы поток имел доступ к настройкам и буферу данных
    if (pthread_create(&math_thread_id, NULL, MathEngine::run_thread, (void*)&app) != 0) {
        syslog(LOG_CRIT, "Failed to create math processing thread!");
        event_base_free(base);
        return 1;
    }

    // Отсоединяем поток, если не планируем делать pthread_join в конце, 
    // либо оставляем так, чтобы завершить корректно при выходе из цикла.
    pthread_detach(math_thread_id);

    syslog(LOG_INFO, "TiXerver fully initialized. Network and Math modules are running.");

    // 5. Вход в основной цикл обработки сетевых событий
    // Этот вызов блокирует текущий (основной) поток.
    event_base_dispatch(base);

    // Точка выхода (сработает при остановке event_base)
    event_base_free(base);
    closelog();

    return 0;
}
