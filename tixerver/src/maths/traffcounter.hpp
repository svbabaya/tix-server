#ifndef TRAFFCOUNTER_HPP
#define TRAFFCOUNTER_HPP

#include "app_context.hpp"
#include "rowMatClass.hpp"
#include "algo_params.hpp"

// Подключаем старое ПО
#include "old_traffcounter.h" 

#include <vector>

class TraffCounter {
public:
    TraffCounter();
    ~TraffCounter() = default;

    // Переносим настройки алгоритма GlobalConfig cfg в internalConfig (новая архитектура)
    // ToDo Из internalConfig сделать TraffSensor, TraffAvgParams для oldEngine
    void updateSettings(const GlobalConfig& cfg);

    // Переделать!!!
    void processFrame(const Frame& frame);

    // Переделать!!! Сбрасываем результаты расчета в контекст
    void syncResultsIfNeeded(MathResults& globalResults);

private:
    GlobalConfig internalConfig;
    
    // Экземпляр старого алгоритмического движка
    old::TraffCounter oldEngine; 

    // Метод для конвертации SensorConfig в старый TraffAlgParams
    // Переделать!!!!!!!
    // old::TraffAlgParams mapToOldParams(const SensorParams& newParams);


    /*** Расчет и хранение времени чтобы syncResultsIfNeeded() сбрасывала в контекст
     *   результаты через заданное время (syncIntervalMs), а не каждый цикл 
     */
    long lastSyncTime;
    const long syncIntervalMs = 10000; 
    long getCurrentMillis();

};

#endif
