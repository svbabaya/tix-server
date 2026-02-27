#include "traffcounter.hpp"
#include <syslog.h>
#include <sys/time.h>
#include <sys/statvfs.h>
#include <fstream>
#include <iomanip>
#include <string>

/**
 * Конструктор: инициализация таймеров и резервирование RAM.
 */
TraffCounter::TraffCounter() 
    : totalObjects(0), currentScore(0.0), fileCounter(0) {
    lastSyncTime = getCurrentMillis();
    
    // Резервируем память заранее для предотвращения фрагментации на MIPS
    frameHistory.reserve(MAX_HISTORY_SIZE);
}

/**
 * Вспомогательный метод для получения времени в мс.
 */
long TraffCounter::getCurrentMillis() {
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}

/**
 * Snapshot настроек: копирование параметров из контекста во внутреннее состояние.
 */
void TraffCounter::updateSettings(const VideoSettings& cfg) {
    internalSettings.threshold = cfg.threshold;
    internalSettings.sensitivity = cfg.sensitivity;
}

/**
 * Основная обработка: прямой доступ к RowMat и накопление данных.
 */
void TraffCounter::processFrame(const Frame& frame) {
    if (frame.empty()) return;

    // Прямой доступ через RowMat: frame[y][x]
    int h = frame.height();
    int w = frame.width();
    uchar pixelVal = frame[h / 2][w / 2];

    // Логика обнаружения (имитация)
    if (pixelVal > internalSettings.threshold) {
        totalObjects++;
        currentScore = (pixelVal / 255.0) * (internalSettings.sensitivity / 100.0);

        // Наполнение внутренней истории
        if (frameHistory.size() < MAX_HISTORY_SIZE) {
            frameHistory.push_back(pixelVal);
        }

        // Триггер сохранения: каждые 500 событий
        if (totalObjects > 0 && (totalObjects % 500 == 0)) {
            saveHistoryToCSV();
        }
    }
}

/**
 * Редкая синхронизация (раз в секунду) с глобальным контекстом.
 */
void TraffCounter::syncResultsIfNeeded(MathResults& globalResults) {
    long now = getCurrentMillis();
    
    if (now - lastSyncTime >= syncIntervalMs) {
        // Захват мьютекса результатов
        pthread_mutex_lock(&globalResults.lock);
        globalResults.objects_detected = totalObjects;
        globalResults.last_score = currentScore;
        pthread_mutex_unlock(&globalResults.lock);

        syslog(LOG_INFO, "[TraffCounter] Sync: Total=%d, SamplesInMem=%zu", 
               totalObjects, frameHistory.size());

        lastSyncTime = now;
    }
}

/**
 * Сохранение в CSV с проверкой свободного места в /tmp/
 */
void TraffCounter::saveHistoryToCSV() {
    if (frameHistory.empty()) return;

    const char* targetPath = "/tmp";
    
    // Проверка свободного места через statvfs
    struct statvfs vfs;
    if (statvfs(targetPath, &vfs) == 0) {
        unsigned long long freeSpace = (unsigned long long)vfs.f_bsize * vfs.f_bavail;
        // Если места меньше 500Кб — отменяем запись во избежание Kernel Panic/ошибок FS
        if (freeSpace < 512 * 1024) {
            syslog(LOG_WARNING, "[TraffCounter] Disk Full on %s: %llu bytes left. Skipping CSV.", 
                   targetPath, freeSpace);
            return;
        }
    }

    std::string fileName = std::string(targetPath) + "/traff_report_" + std::to_string(fileCounter++) + ".csv";
    
    std::ofstream fs(fileName);
    if (!fs.is_open()) {
        syslog(LOG_ERR, "[TraffCounter] Cannot open CSV: %s", fileName.c_str());
        return;
    }

    fs << "ID,Value,Threshold\n";
    for (size_t i = 0; i < frameHistory.size(); ++i) {
        fs << i << "," << (int)frameHistory[i] << "," << internalSettings.threshold << "\n";
    }

    fs.close();
    
    syslog(LOG_NOTICE, "[TraffCounter] CSV Saved: %s (%zu samples)", 
           fileName.c_str(), frameHistory.size());

    // Очистка истории после успешного сброса на "диск"
    frameHistory.clear();
}
