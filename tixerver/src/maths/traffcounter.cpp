#include "traffcounter.hpp"
#include <syslog.h>

TraffCounter::TraffCounter() {
    lastSyncTime = getCurrentMillis();
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

void TraffCounter::updateSettings(const GlobalConfig& cfg) {
    // Если конфигурация изменилась, переинициализируем старый движок
    // if (this->internalConfig.sensors.size() != cfg.sensors.size()) {
        
    //     std::vector<old::TraffSensor> oldSensors;
    //     old::TraffAvgParams avgParams; // Можно настроить интервалы усреднения тут

    //     for (const auto& s : cfg.sensors) {
    //         old::TraffSensor sensor;
    //         // s.id -> sensor.setId(...)
    //         // s.zone -> передаем координаты в сенсор
    //         sensor.setParams(mapToOldParams(s.params));
    //         oldSensors.push_back(sensor);
    //     }

    //     // Инициализируем старый движок списком подготовленных сенсоров
    //     oldEngine.initTraffSensors(oldSensors, avgParams, true, 1);
        
    //     syslog(LOG_NOTICE, "[TraffCounter] Old engine re-initialized with %zu sensors", oldSensors.size());
    // }
    this->internalConfig = cfg;
}





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
