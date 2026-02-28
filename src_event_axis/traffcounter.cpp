#include "traffcounter.hpp"
#include <syslog.h>
#include <sys/time.h>
#include <sys/statvfs.h>
#include <unistd.h>   
#include <fcntl.h>
#include <cstdio>
#include <cstring>

/**
 * Конструктор: инициализация таймеров и резервирование RAM.
 */
TraffCounter::TraffCounter() 
    : totalObjects(0), currentScore(0.0), fileCounter(0) {
    lastSyncTime = getCurrentMillis();
    
    // Резервируем память для истории (учитываем лимиты MIPS)
    frameHistory.reserve(MAX_HISTORY_SIZE);
}

/**
 * Вспомогательный метод для времени.
 */
long TraffCounter::getCurrentMillis() {
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}

/**
 * Синхронизация: забираем Snapshot настроек из AppContext.
 * Теперь работаем с GlobalConfig (список сенсоров).
 */
void TraffCounter::updateSettings(const GlobalConfig& cfg) {
    // Копируем всю структуру со всеми сенсорами
    this->internalConfig = cfg;
}

/**
 * Основная обработка: итерируемся по всем активным сенсорам.
 */
void TraffCounter::processFrame(const Frame& frame) {
    // 1. Если кадра нет или сервер еще не прислал сенсоры — выходим
    if (frame.empty() || internalConfig.sensors.empty()) {
        return;
    }

    // 2. Проходим по каждому сенсору в списке
    for (const auto& sensor : internalConfig.sensors) {
        
        // ВАЖНО: Здесь будет ваша математика по зоне (sensor.zone)
        // Для примера — берем только первую точку четырехугольника
        int x = sensor.zone.p1.x;
        int y = sensor.zone.p1.y;

        // Безопасная проверка границ кадра
        if (x >= frame.width() || y >= frame.height()) continue;

        uchar pixelVal = frame[y][x];

        // Используем параметры конкретно этого сенсора
        if (pixelVal > sensor.params.binarizationThreshold) {
            totalObjects++;
            currentScore = (double)pixelVal / 255.0;

            // Наполнение истории для отладки
            if (frameHistory.size() < MAX_HISTORY_SIZE) {
                frameHistory.push_back(pixelVal);
            }
        }
    }

    // 3. Триггер сохранения истории (общий на все сенсоры)
    if (totalObjects > 0 && (totalObjects % 500 == 0)) {
        saveHistoryToCSV();
    }
}

/**
 * Редкая синхронизация (раз в 10 секунд) с результатами в AppContext.
 */
void TraffCounter::syncResultsIfNeeded(MathResults& globalResults) {
    long now = getCurrentMillis();
    
    if (now - lastSyncTime >= 10000) { // Используем 10000мс напрямую
        pthread_mutex_lock(&globalResults.lock);
        globalResults.objects_detected = totalObjects;
        globalResults.last_score = currentScore;
        pthread_mutex_unlock(&globalResults.lock);

        syslog(LOG_INFO, "[TraffCounter] Sync: Total=%d, ActiveSensors=%lu", 
               totalObjects, (unsigned long)internalConfig.sensors.size());

        lastSyncTime = now;
    }
}

/**
 * Сохранение в CSV с ротацией файлов и проверкой места.
 */
void TraffCounter::saveHistoryToCSV() {
    if (frameHistory.empty()) return;

    const char* targetPath = "/tmp";
    const int MAX_FILES = 5;

    // 1. Ротация (удаление старых)
    if (fileCounter >= MAX_FILES) {
        char oldFileName[64];
        snprintf(oldFileName, sizeof(oldFileName), "%s/traff_report_%d.csv", 
                 targetPath, fileCounter - MAX_FILES);
        unlink(oldFileName);
    }

    // 2. Проверка места на диске
    struct statvfs vfs;
    if (statvfs(targetPath, &vfs) == 0) {
        unsigned long long freeSpace = (unsigned long long)vfs.f_bsize * vfs.f_bavail;
        if (freeSpace < 256 * 1024) return;
    }

    // 3. Запись файла через POSIX (экономим RAM)
    char fileName[64];
    snprintf(fileName, sizeof(fileName), "%s/traff_report_%d.csv", targetPath, fileCounter++);
    
    int fd = open(fileName, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd >= 0) {
        const char* header = "ID,Value\n";
        write(fd, header, strlen(header));

        char lineBuf[32];
        for (size_t i = 0; i < frameHistory.size(); ++i) {
            int len = snprintf(lineBuf, sizeof(lineBuf), "%lu,%d\n", 
                               (unsigned long)i, (int)frameHistory[i]);
            write(fd, lineBuf, len);
        }
        close(fd);
    }

    frameHistory.clear();
}
