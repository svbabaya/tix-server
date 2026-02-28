#pragma once

#include "app_context.hpp"
#include "rowMatClass.hpp"
#include "algo_params.hpp" // Наша новая структура с MathCoreParams и SensorConfig
#include <vector>

class TraffCounter {
private:
    // Внутренняя копия настроек (Snapshot)
    // Хранит список сенсоров, их координаты и индивидуальные параметры
    GlobalConfig internalConfig;

    // Накопленные данные для синхронизации
    int totalObjects = 0;
    double currentScore = 0.0;
    
    // История для отладки (сохраняется в /tmp)
    std::vector<uchar> frameHistory;
    const size_t MAX_HISTORY_SIZE = 1000; 

    // Таймер синхронизации с AppContext
    long lastSyncTime;
    const long syncIntervalMs = 1000; // 1 секунда

    // Вспомогательный метод времени
    long getCurrentMillis();

    // Параметры файлового лога
    int fileCounter = 0;
    void saveHistoryToCSV(); // Приватный метод записи через POSIX write

public:
    TraffCounter();
    ~TraffCounter() = default;

    /**
     * Копирование настроек из контекста во внутреннее состояние.
     * Вызывается перед обработкой кадра, если в AppContext обновились данные.
     */
    void updateSettings(const GlobalConfig& cfg);

    /**
     * Основная обработка: итерируется по всем сенсорам в internalConfig.
     */
    void processFrame(const Frame& frame);

    /**
     * Безопасный сброс результатов в MathResults (под мьютексом).
     */
    void syncResultsIfNeeded(MathResults& globalResults);

    // Геттеры
    int getTotalCount() const { return totalObjects; }
    size_t getActiveSensorsCount() const { return internalConfig.sensors.size(); }
};
