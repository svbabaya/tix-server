#include "math300_engine.hpp"

#include <cmath>
#include <unistd.h>


void* MathEngine::run_thread(void* arg) {
    AppContext* ctx = (AppContext*)arg;
    processing_loop(ctx);
    return NULL;
}

void MathEngine::processing_loop(AppContext* ctx) {
    while (true) {
        // 1. Копируем настройки под мьютексом
        VideoSettings cfg; 
        pthread_mutex_lock(&ctx->settings.lock);
        cfg.threshold = ctx->settings.threshold;
        cfg.sensitivity = ctx->settings.sensitivity;
        pthread_mutex_unlock(&ctx->settings.lock);

        // 2. Выполняем анализ (вызываем наш метод)
        int result = analyze(cfg);

        // 3. Записываем результат под мьютексом
        pthread_mutex_lock(&ctx->results.lock);
        ctx->results.objects_detected = result;
        pthread_mutex_unlock(&ctx->results.lock);

        usleep(100000); // 0.1 sec
    }
}

int MathEngine::analyze(const VideoSettings& current_cfg) {

    // --- Имитация высокой нагрузки ---
    double dummy_work = 0.0;
    // Число итераций подберите экспериментально. 
    // 1 000 000 обычно дает заметную нагрузку на слабых CPU камер.
    for (int i = 0; i < 10000; ++i) {
        dummy_work += std::sin(i) * std::cos(i) + std::sqrt(i + current_cfg.threshold);

    }
    // Используем переменную, чтобы оптимизатор компилятора не выкинул цикл
    if (dummy_work < 0) return -1; 
    // ---------------------------------

    // Вся ваша математика здесь
    if (current_cfg.threshold > 50) {
        return 1;
    }
    return 0;
}
