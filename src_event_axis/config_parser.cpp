#include <cJSON.h>
#include "algo_params.hpp"

GlobalConfig parseJsonToConfig(const char* jsonString) {
    GlobalConfig cfg;
    cJSON* root = cJSON_Parse(jsonString);
    if (!root) return cfg;

    // 1. Парсим глобальные интервалы
    cJSON* mainInt = cJSON_GetObjectItemCaseSensitive(root, "mainIntervalMsec");
    if (cJSON_IsNumber(mainInt)) cfg.mainIntervalMsec = mainInt->valueint;

    cJSON* nestInt = cJSON_GetObjectItemCaseSensitive(root, "nestedIntervalMsec");
    if (cJSON_IsNumber(nestInt)) cfg.nestedIntervalMsec = nestInt->valueint;

    // 2. Парсим массив сенсоров
    cJSON* sensorsArr = cJSON_GetObjectItemCaseSensitive(root, "sensors");
    if (cJSON_IsArray(sensorsArr)) {
        int arraySize = cJSON_GetArraySize(sensorsArr);
        cfg.sensors.reserve(arraySize); // Резервируем память один раз

        for (int i = 0; i < arraySize; i++) {
            cJSON* sObj = cJSON_GetArrayItem(sensorsArr, i);
            SensorConfig sensor;

            // ID
            cJSON* id = cJSON_GetObjectItemCaseSensitive(sObj, "id");
            if (cJSON_IsNumber(id)) sensor.id = id->valueint;

            // Зона (Координаты)
            cJSON* zone = cJSON_GetObjectItemCaseSensitive(sObj, "zone");
            if (zone) {
                auto parsePoint = [&](const char* name, Point& p) {
                    cJSON* pt = cJSON_GetObjectItemCaseSensitive(zone, name);
                    if (pt) {
                        cJSON* x = cJSON_GetObjectItemCaseSensitive(pt, "x");
                        cJSON* y = cJSON_GetObjectItemCaseSensitive(pt, "y");
                        if (cJSON_IsNumber(x)) p.x = x->valueint;
                        if (cJSON_IsNumber(y)) p.y = y->valueint;
                    }
                };
                parsePoint("p1", sensor.zone.p1);
                parsePoint("p2", sensor.zone.p2);
                parsePoint("p3", sensor.zone.p3);
                parsePoint("p4", sensor.zone.p4);
            }

            // Математические параметры (params)
            cJSON* prm = cJSON_GetObjectItemCaseSensitive(sObj, "params");
            if (prm) {
                // Макрос для сокращения кода (только внутри этой функции)
                #define GET_INT(name) { \
                    cJSON* item = cJSON_GetObjectItemCaseSensitive(prm, #name); \
                    if (cJSON_IsNumber(item)) sensor.params.name = item->valueint; \
                }
                GET_INT(binarizationThreshold);
                GET_INT(shadowMaxUpperThreshold);
                GET_INT(shadowDarkThreshold);
                GET_INT(shadowRange);
                GET_INT(lightBeamMaxThreshold);
                GET_INT(objectConfirmationPoints);
                GET_INT(objectEnterPointsCount);
                GET_INT(objectExitPointsCount);
                GET_INT(jamCarInZoneTimeSec);
                GET_INT(increasedObjectEnterPointsCount);
                GET_INT(increasedObjectExitPointsCount);
                GET_INT(carExitEnterIntervalMsec);
                GET_INT(catchSlowMotionPointsCount);
                GET_INT(catchMotionPointsCount);
                GET_INT(catchFrameMotionRepeatCount);
                GET_INT(catchInterZoneFrameDifferenceJumpTimeMsec);
                GET_INT(catchNoMotionPointsCount);
                GET_INT(catchCheckJamMaxTimeIntervalMsec);
                GET_INT(catchInterZoneNoMotionWasFoundTimeMsec);
                GET_INT(catchBackgroundDifferencePointsCount);
                GET_INT(catchFrameStableRepeatCount);
                GET_INT(catchTimeToFindStableIntervalSec);
                GET_INT(sensorParallelWork);
                GET_INT(tracker);
                GET_INT(smDataWakeup);
                GET_INT(sensorMaxLockTimeSec);
                #undef GET_INT
            }
            cfg.sensors.push_back(sensor);
        }
    }

    cJSON_Delete(root);
    return cfg;
}
