#include "traffcounter.hpp"
#include <syslog.h>

TraffCounter::TraffCounter() {
    lastSyncTime = getCurrentMillis();
}

// Переносим настройки алгоритма GlobalConfig cfg в internalConfig
// ToDo Из internalConfig сделать TraffSensor, TraffAvgParams для oldEngine
void TraffCounter::updateSettings(const GlobalConfig& cfg) {
    /*** Debug */
    // Если количество сенсоров изменилось или это первая загрузка
    if (this->internalConfig.sensors.size() != cfg.sensors.size()) {
        syslog(LOG_NOTICE, "[TraffCounter] Local config updated: %lu -> %lu sensors", 
               (unsigned long)this->internalConfig.sensors.size(), 
               (unsigned long)cfg.sensors.size());
    }
    /*** end Debug */
    this->internalConfig = cfg;

    /**** ToDo sth as traffiXInit(TraffCounter &traffixCounter) in old main.cpp */

    // Определяем поля TraffAvgParams объекта oldEngine из internalConfig
    oldEngine.avg.bigAvgInterval = internalConfig.mainIntervalMsec;
    oldEngine.avg.smallAvgInterval = internalConfig.nestedIntervalMsec;

    // Создать TraffSensor объекта oldEngine

    // oldEngine.initTraffSensors

    // Перенести данные из internalConfig в TraffSensor 
}

/**
 * Преобразование новых параметров в структуру старого ПО.
 */
// old::TraffAlgParams TraffCounter::mapToOldParams(const SensorParams& p) {
//     old::TraffAlgParams oldP;
//     // Маппим только то, что есть в вашем новом конфиге
//     oldP.BINARIZATION_THRESHOLD = p.binarizationThreshold;
//     // Остальные параметры старого ПО останутся по умолчанию (из конструктора oldP)
//     // или добавьте их в ваш algo_params.hpp
//     return oldP;
// }

// Переделываем
void TraffCounter::processFrame(const Frame& frame) {
    if (frame.empty() || internalConfig.sensors.empty()) {
        return;
    }

    // ВЫЗОВ СТАРОГО АЛГОРИТМА
    // Метод processImage внутри итерируется по sensorList и делает всю математику
    //bool isAveragingDone = oldEngine.processImage(frame);

}





// Переделываем - достаем результаты из старого TraffCounter
void TraffCounter::syncResultsIfNeeded(MathResults& globalResults) {
    // long now = getCurrentMillis();
    
    // if (now - lastSyncTime >= syncIntervalMs) {
    //     std::vector<old::TraffStat> stats;
    //     oldEngine.toTraffStat(stats, true);

         pthread_mutex_lock(&globalResults.lock);
        
    //     // Агрегируем данные из всех сенсоров старого ПО в ваши глобальные результаты
    //     int total = 0;
    //     for (const auto& s : stats) {
    //         total += s.totalCounter;
    //     }
    //     globalResults.objects_detected = total;
        
         pthread_mutex_unlock(&globalResults.lock);

    //     lastSyncTime = now;
    // }
}





// Оставляем как есть, на старый алгоритм не влияет
long TraffCounter::getCurrentMillis() {
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}
