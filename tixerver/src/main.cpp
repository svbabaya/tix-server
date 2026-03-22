#include "app_context.hpp"
#include "network_server.hpp"
#include "math_engine.hpp"
#include "get_serial_number.hpp"

#include <thread>
#include <atomic>
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
    syslog(LOG_NOTICE, "%s version %s starting...", APP_NAME_STR, APP_VER_STR); // Данные из макросов makefile

    /* Check serial number */
    std::string expected_sn = SERIAL_NUMBER; // SERIAL_NUMBER может быть "free" или 12-значной строкой
    if (expected_sn != "free") {
        std::string sn = getCameraSerialNumber();
        // syslog(LOG_NOTICE, "Expected serial number: %s", expected_sn.c_str());
        if (sn.empty()) {
            syslog(LOG_WARNING, "The app %s can't read the camera's serial number and will be stopped!", APP_NAME_STR);
            return 0;
        }
        // syslog(LOG_NOTICE, "Actual serial number: %s", sn.c_str());
        if (sn != expected_sn) {
            syslog(LOG_WARNING, "The app %s is not intended for this camera and will be stopped!", APP_NAME_STR);
            return 0;
        }
    }
    /* end Check serial number */

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
    if (!NetworkServer::start(&app, app.netParams.api_port, app.netParams.data_port)) {
        syslog(LOG_CRIT, "Failed to start NetworkServer on ports %d, %d!", 
               app.netParams.api_port, app.netParams.data_port);
        event_base_free(base);
        return 1;
    }

    // 5. Запуск потока через std::thread (C++11)
    // Передаем ссылку на объект app
    std::thread math_thread(MathEngine::run_thread, &app);

    syslog(LOG_NOTICE, "%s running. API Port: %d, Data Port: %d", 
        APP_NAME_STR, app.netParams.api_port, app.netParams.data_port);

    // 6. Вход в сетевой цикл (БЛОКИРУЮЩИЙ ВЫЗОВ)
    // Основной поток "живет" здесь, пока не придет сигнал
    event_base_dispatch(base);

    // 7. Остановка
    if (math_thread.joinable()) {
        math_thread.join(); // Ждем завершения расчетов
    }

    syslog(LOG_NOTICE, "%s stopped gracefully.", APP_NAME_STR);
    
    event_base_free(base);
    closelog();
    return 0;
}
