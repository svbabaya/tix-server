#pragma once

#include "app_context.hpp"
#include "rowMatClass.hpp"
#include <vector>
#include <sys/time.h>

class TraffCounter {
private:
    // Внутренние настройки (Snapshot)
    VideoSettings internalSettings;

    // Накопленные данные
    int totalObjects = 0;
    double currentScore = 0.0;
    
    // Имитация истории для анализа трендов (наполнение объекта)
    // Ограничим размер, чтобы не исчерпать RAM на встроенной системе
    std::vector<uchar> frameHistory;
    const size_t MAX_HISTORY_SIZE = 1000; 

    // Таймер синхронизации
    long lastSyncTime;
    const long syncIntervalMs = 1000; // 1 секунда

    // Вспомогательная функция времени
    long getCurrentMillis();

    const int SAVE_TRIGGER_COUNT = 500; // Сбрасывать в файл каждые 500 событий
    int fileCounter = 0;
    void saveHistoryToCSV(); // Приватный метод сохранения

public:
    TraffCounter();
    ~TraffCounter() = default;

    // Копирование настроек из контекста во внутреннее состояние
    void updateSettings(const VideoSettings& cfg);

    // Основная математика над кадром (наполнение данными)
    void processFrame(const Frame& frame);

    // Сброс результатов в AppContext по таймеру с логированием
    void syncResultsIfNeeded(MathResults& globalResults);

    // Геттеры для внутреннего использования (если понадобятся)
    int getTotalCount() const { return totalObjects; }
};
