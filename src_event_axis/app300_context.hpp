#ifndef APP_CONTEXT_HPP
#define APP_CONTEXT_HPP

#include "camera_info.hpp"
#include "command300_processor.hpp"

#include <pthread.h>
#include <string>

// Настройки для математического модуля (меняются через POST)
struct VideoSettings {
    int threshold;
    int sensitivity;
    pthread_mutex_t lock;

    VideoSettings() : threshold(10), sensitivity(50) {
        pthread_mutex_init(&lock, NULL);
    }
    ~VideoSettings() { pthread_mutex_destroy(&lock); }
};

// Результаты обработки видео (отдаются через TCP)
struct MathResults {
    int objects_detected;
    double last_score;
    pthread_mutex_t lock;

    MathResults() : objects_detected(0), last_score(0.0) {
        pthread_mutex_init(&lock, NULL);
    }
    ~MathResults() { pthread_mutex_destroy(&lock); }
};

// Глобальный контекст
struct AppContext {
    struct event_base* base;
    VideoSettings settings;
    MathResults results;
    CameraInfo info;
    CommandProcessor processor;

    AppContext(struct event_base* b) : base(b) {}
};

#endif
