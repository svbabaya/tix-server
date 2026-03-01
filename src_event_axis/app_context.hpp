#pragma once

#include "camera_info.hpp"
#include "command_processor.hpp"
#include "algo_params.hpp" // Вынесли структуры параметров сюда

#include <pthread.h>
#include <vector>
#include <syslog.h>

/**
 * Глобальный контейнер для параметров алгоритма.
 * Защищен мьютексом, так как обновляется из сетевого потока (CommandProcessor),
 * а читается из потока обработки видео (TraffCounter).
 */
struct AlgoSettings {
    GlobalConfig config;      // Наша структура с вектором сенсоров и MathCoreParams
    pthread_mutex_t lock;

    AlgoSettings() {
        pthread_mutex_init(&lock, NULL);
    }
    ~AlgoSettings() {
        pthread_mutex_destroy(&lock);
    }

    // Вспомогательный метод для безопасного копирования "снимка" настроек
    GlobalConfig getSnapshot() {
        pthread_mutex_lock(&lock);
        GlobalConfig copy = config;
        pthread_mutex_unlock(&lock);
        return copy;
    }

    // Безопасное обновление данных (вызывается при получении команды с сервера)
    void update(const GlobalConfig& newConfig) {
        pthread_mutex_lock(&lock);
        config = newConfig;

        /*** Debug */
        syslog(LOG_NOTICE, "[AppContext] Config updated. New sensors: %lu", 
           (unsigned long)config.sensors.size());
        /*** end Debug */

        pthread_mutex_unlock(&lock);
    }
};

/**
 * Результаты обработки (то, что уходит клиенту по TCP)
 */
struct MathResults {
    int objects_detected;
    double last_score;
    pthread_mutex_t lock;

    MathResults() : objects_detected(0), last_score(0.0) {
        pthread_mutex_init(&lock, NULL);
    }
    ~MathResults() {
        pthread_mutex_destroy(&lock);
    }
};

/**
 * Глобальный контекст приложения
 */
struct AppContext {
    struct event_base* base;    // Для libevent
    AlgoSettings algoSettings;  // Настройки алгоритма
    MathResults results;        // Текущая статистика
    CameraInfo info;            // Данные о камере Axis
    CommandProcessor processor; // Обработчик команд

    AppContext(struct event_base* b) : base(b), processor() {}
};
