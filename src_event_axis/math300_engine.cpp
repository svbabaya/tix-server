#include "math300_engine.hpp"
#include <capture.h>
#include <cmath>
#include <unistd.h>
#include <syslog.h>

void* MathEngine::run_thread(void* arg) {
    AppContext* ctx = (AppContext*)arg;
    processing_loop(ctx);
    return NULL;
}

void MathEngine::processing_loop(AppContext* ctx) {
    // 1. Инициализация захвата (один раз при старте потока)
    // Используем минимальное разрешение для экономии ресурсов CPU
    capture_session_t* session = capture_open_stream("320x240", "yuv");
    if (!session) {
        syslog(LOG_CRIT, "MathEngine: Cannot open capture stream!");
        return;
    }

    syslog(LOG_INFO, "MathEngine: Video stream 320x240 (YUV) started");

    while (true) {
        // 2. Получение кадра (Блокирующий вызов - заменяет usleep)
        capture_frame_t* frame = capture_get_frame(session);
        
        if (frame) {
            // 3. Копируем настройки под мьютексом
            VideoSettings cfg; 
            pthread_mutex_lock(&ctx->settings.lock);
            cfg.threshold = ctx->settings.threshold;
            cfg.sensitivity = ctx->settings.sensitivity;
            pthread_mutex_unlock(&ctx->settings.lock);

            // 4. Выполняем анализ реального кадра
            int result = analyze(frame, cfg);

            // 5. Записываем результат под мьютексом
            pthread_mutex_lock(&ctx->results.lock);
            ctx->results.objects_detected = result;
            // Можно обновить score на основе данных кадра
            ctx->results.last_score = 0.85; 
            pthread_mutex_unlock(&ctx->results.lock);

            // 6. ОБЯЗАТЕЛЬНО освобождаем кадр (иначе память Axis переполнится)
            capture_frame_free(frame);
        } else {
            // Если кадр не получен (например, сбой сенсора), делаем небольшую паузу
            usleep(100000);
        }
        
        // usleep(100000) здесь больше не нужен, так как частоту цикла 
        // теперь диктует частота кадров камеры (FPS).
    }

    capture_close_stream(session);
}

int MathEngine::analyze(capture_frame_t* frame, const VideoSettings& current_cfg) {
    // Получаем доступ к данным кадра (канал яркости Y)
    unsigned char* data = (unsigned char*)capture_frame_data(frame);
    size_t width = capture_frame_width(frame);
    size_t height = capture_frame_height(frame);

    // --- Имитация высокой нагрузки (синхронно с анализом) ---
    double dummy_work = 0.0;
    for (int i = 0; i < 10000; ++i) {
        dummy_work += std::sin(i) * std::cos(i) + std::sqrt(i + current_cfg.threshold);
    }

    // --- Простейшая логика: считаем яркость центрального пикселя ---
    // (Или любой другой алгоритм на основе данных 'data')
    int center_pixel_val = data[(width * height) / 2];

    if (dummy_work < 0) return -1; // Чтобы компилятор не удалил цикл

    // Логика: если яркость в центре выше порога
    if (center_pixel_val > current_cfg.threshold) {
        return 1;
    }
    return 0;
}
