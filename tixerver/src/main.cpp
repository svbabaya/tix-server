#include "app_context.hpp"
#include "network_server.hpp"
#include "math_engine.hpp"

#include <thread>   // C++11
#include <atomic>   // C++11
#include <syslog.h>
#include <event2/event.h>
#include <signal.h>

static AppContext* g_ctx = nullptr;

void signal_handler(int sig) {
    if (g_ctx) {
        syslog(LOG_NOTICE, "Signal %d received: stopping...", sig);
        g_ctx->stop(); // Атомарный флаг для MathEngine
        if (g_ctx->base) {
            // Останавливаем сетевой цикл libevent
            event_base_loopbreak(g_ctx->base); 
        }
    }
}

int main() {
    openlog("TiXerver", LOG_PID | LOG_CONS, LOG_USER);
    
    struct event_base* base = event_base_new();
    if (!base) return 1;

    AppContext app(base);
    g_ctx = &app;

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    if (!NetworkServer::start(&app, app.netParams.api_port, app.netParams.data_port)) {
        event_base_free(base);
        return 1;
    }

    // 5. Запуск потока через std::thread (C++11)
    // Передаем ссылку на объект app
    std::thread math_thread(MathEngine::run_thread, &app);

    syslog(LOG_NOTICE, "Server running with C++11 threads...");

    // 6. Сетевой цикл
    event_base_dispatch(base);

    // 7. Остановка
    if (math_thread.joinable()) {
        math_thread.join(); // Ждем завершения расчетов
    }

    event_base_free(base);
    closelog();
    return 0;
}
