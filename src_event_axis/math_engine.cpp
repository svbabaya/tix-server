#include "math_engine.hpp"

#include <cmath>
#include <unistd.h>
#include <syslog.h>
#include <sys/time.h>

void* MathEngine::run_thread(void* arg) {
    AppContext* ctx = (AppContext*)arg;
    processing_loop(ctx);
    return NULL;
}

void MathEngine::processing_loop(AppContext* ctx) {
    // Настройка потока: 640x480, 25 FPS, формат Y800 (яркость)
    media_stream* stream = capture_open_stream(IMAGE_UNCOMPRESSED, "resolution=640x480&sdk_format=Y800&fps=25");
    
    if (stream == NULL) {
        syslog(LOG_CRIT, "MathEngine: Failed to open stream");
        return;
    }

    syslog(LOG_INFO, "MathEngine: Video stream 640x480 Y800 started");

    // Переменные для расчета FPS
    int frame_count = 0;
    struct timeval tv_start, tv_end;
    gettimeofday(&tv_start, NULL);

    while (true) {
        media_frame* frame = capture_get_frame(stream);
        
        if (frame != NULL) {
            // 1. Читаем настройки
            VideoSettings cfg; 
            pthread_mutex_lock(&ctx->settings.lock);
            cfg.threshold = ctx->settings.threshold;
            cfg.sensitivity = ctx->settings.sensitivity;
            pthread_mutex_unlock(&ctx->settings.lock);

            // 2. Анализируем кадр
            int result = analyze(frame, cfg);

            // 3. Записываем результат
            pthread_mutex_lock(&ctx->results.lock);
            ctx->results.objects_detected = result;
            ctx->results.last_score = 0.77; 
            pthread_mutex_unlock(&ctx->results.lock);

            // 4. Освобождаем память кадра (обязательно для Axis SDK)
            capture_frame_free(frame);

            // --- Статистика FPS ---
            frame_count++;
            if (frame_count >= 100) {
                gettimeofday(&tv_end, NULL);
                
                // Считаем время в секундах
                double seconds = (tv_end.tv_sec - tv_start.tv_sec) + 
                                 (tv_end.tv_usec - tv_start.tv_usec) / 1000000.0;
                double fps = (seconds > 0) ? (frame_count / seconds) : 0;

                syslog(LOG_INFO, "TiXerver Stats: 100 frames in %.2f sec (Real FPS: %.2f)", seconds, fps);
                
                // Сброс для следующего замера
                frame_count = 0;
                gettimeofday(&tv_start, NULL);
            }
        } else {
            // В случае ошибки захвата (например, перегрузка драйвера)
            syslog(LOG_ERR, "MathEngine: Capture failed, sleeping...");
            usleep(100000); // 0.1 sec
        }
    }
    capture_close_stream(stream);
}

int MathEngine::analyze(media_frame* frame, const VideoSettings& current_cfg) {
    unsigned char* data = (unsigned char*)capture_frame_data(frame);
    
    // --- Эффективная целочисленная нагрузка для MIPS ---
    volatile long dummy_work = 0; 
    // Начни с 500 000 итераций (это примерно эквивалент твоих 15к с синусами)
    for (int i = 0; i < 500000; ++i) {
        dummy_work += (i * 3) / ( (i % 5) + 1 );
        dummy_work ^= (i << 2); // Битовые операции очень хороши для тестов
    }

    // --- Реальная логика (Центральный пиксель) ---
    if (data) {
        int width = capture_frame_width(frame);
        int height = capture_frame_height(frame);
        int val = data[(width * height) / 2];
        return (val > current_cfg.threshold) ? 1 : 0;
    }
    return 0;
}
