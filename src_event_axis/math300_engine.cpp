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
    // В SDK 2: argv[1] это тип ("uncompressed"), argv[2] это свойства
    // Для ARTPEC-3 лучше использовать Y800 (серый) или YUV
    media_stream* stream = capture_open_stream("uncompressed", "resolution=320x240&sdk_format=Y800&fps=10");
    
    if (stream == NULL) {
        syslog(LOG_CRIT, "MathEngine: Failed to open stream");
        return;
    }

    syslog(LOG_INFO, "MathEngine: Video stream 320x240 Y800 started");




    // gettimeofday(&tv_start, NULL);

    // /* print intital information */
    // frame = capture_get_frame(stream);

    // LOGINFO("Getting %d frames. resolution: %dx%d framesize: %d\n",
    //     numframes,
    //     capture_frame_width(frame),
    //     capture_frame_height(frame),
    //     capture_frame_size(frame));

    // capture_frame_free(frame);




    while (true) {
        media_frame* frame = capture_get_frame(stream);
        
        if (frame != NULL) {
            VideoSettings cfg; 
            pthread_mutex_lock(&ctx->settings.lock);
            cfg.threshold = ctx->settings.threshold;
            cfg.sensitivity = ctx->settings.sensitivity;
            pthread_mutex_unlock(&ctx->settings.lock);

            int result = analyze(frame, cfg);

            pthread_mutex_lock(&ctx->results.lock);
            ctx->results.objects_detected = result;
            ctx->results.last_score = 0.77; 
            pthread_mutex_unlock(&ctx->results.lock);

            capture_frame_free(frame);
        } else {
            syslog(LOG_ERR, "MathEngine: Capture failed");
            usleep(100000); // 0.1 sec
        }
    }
    capture_close_stream(stream);
}

int MathEngine::analyze(media_frame* frame, const VideoSettings& current_cfg) {
    // Получаем данные кадра через функции SDK 2
    unsigned char* data = (unsigned char*)capture_frame_data(frame);
    int width = capture_frame_width(frame);
    int height = capture_frame_height(frame);

    // Имитация нагрузки
    double dummy_work = 0.0;
    for (int i = 0; i < 15000; ++i) {
        dummy_work += std::sin(i) * std::cos(i) + std::sqrt(i + current_cfg.threshold);
    }

    if (dummy_work < 0) return -1; 

    // Анализ яркости центрального пикселя
    if (data && width > 0 && height > 0) {
        int val = data[(width * height) / 2];
        return (val > current_cfg.threshold) ? 1 : 0;
    }

    return 0;
}
