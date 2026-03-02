#include <pthread.h>
#include <syslog.h>
#include <event2/event.h>
#include <signal.h>

#include "app_context.hpp"
#include "network_server.hpp"
#include "math_engine.hpp"
#include "system_constants.hpp"

// Глобальный указатель для доступа из обработчика сигналов
static AppContext* g_ctx = nullptr;

void signal_handler(int sig) {
    if (g_ctx) {
        syslog(LOG_NOTICE, "Signal %d received: stopping...", sig);
        g_ctx->stop(); // Флаг для MathEngine
        
        if (g_ctx->base) {
            // Останавливаем сетевой цикл libevent
            event_base_loopbreak(g_ctx->base); 
        }
    }
}

int main() {
    openlog("TiXerver", LOG_PID | LOG_CONS, LOG_USER);
    syslog(LOG_NOTICE, "TiXerver version 1.0 starting...");

    // 1. Инициализация libevent
    struct event_base* base = event_base_new();
    if (!base) {
        syslog(LOG_CRIT, "Failed to initialize libevent!");
        return 1;
    }

    // 2. Создание контекста и СВЯЗЬ с глобальным указателем
    AppContext app(base);
    g_ctx = &app;

    // 3. Настройка сигналов
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    // 4. Запуск сетевых служб
    if (!NetworkServer::start(&app, app.netParams.data_port, app.netParams.api_port)) {
        syslog(LOG_CRIT, "Failed to start NetworkServer on ports %d, %d!", 
               app.netParams.data_port, app.netParams.api_port);
        event_base_free(base);
        return 1;
    }

    // 5. Запуск математического потока
    pthread_t math_thread_id;
    if (pthread_create(&math_thread_id, NULL, MathEngine::run_thread, (void*)&app) != 0) {
        syslog(LOG_CRIT, "Failed to create MathEngine thread!");
        event_base_free(base);
        return 1;
    }

    syslog(LOG_INFO, "TiXerver running. Data Port: %d, API Port: %d", 
           app.netParams.data_port, app.netParams.api_port);

    // 6. Вход в сетевой цикл (БЛОКИРУЮЩИЙ ВЫЗОВ)
    // Основной поток "живет" здесь, пока не придет сигнал
    event_base_dispatch(base);

    // 7. ОСТАНОВКА: Сюда попадем только после signal_handler -> loopbreak
    syslog(LOG_NOTICE, "Waiting for MathEngine to finish...");
    pthread_join(math_thread_id, NULL); // Теперь это в правильном месте

    syslog(LOG_NOTICE, "TiXerver stopped gracefully.");
    event_base_free(base);
    closelog();

    return 0;
}
