#pragma once
#include <vector>

struct Point { int x = 0; int y = 0; };

struct Quadrilateral {
    Point p1, p2, p3, p4;
};

struct MathCoreParams {
    int binarizationThreshold = 0;
    int shadowMaxUpperThreshold = 0;
    int shadowDarkThreshold = 0;
    int shadowRange = 0;
    int lightBeamMaxThreshold = 0;
    int shadowMaxWrongPoints = 0;
    int lightBeamMaxWrongPoints = 0;
    int objectConfirmationPoints = 0;
    int objectEnterPointsCount = 0;
    int objectExitPointsCount = 0;
    int jamCarInZoneTimeSec = 0;
    int increasedObjectEnterPointsCount = 0;
    int increasedObjectExitPointsCount = 0;
    int carExitEnterIntervalMsec = 0;
    int catchSlowMotionPointsCount = 0;
    int catchMotionPointsCount = 0;
    int catchFrameMotionRepeatCount = 0;
    int catchInterZoneFrameDifferenceJumpTimeMsec = 0;
    int catchNoMotionPointsCount = 0;
    int catchCheckJamMaxTimeIntervalMsec = 0;
    int catchInterZoneNoMotionWasFoundTimeMsec = 0;
    int catchBackgroundDifferencePointsCount = 0;
    int catchFrameStableRepeatCount = 0;
    int catchTimeToFindStableIntervalSec = 0;
    int sensorParallelWork = 0;
    int tracker = 0;
    int smDataWakeup = 0;
    int sensorMaxLockTimeSec = 0;
};

struct SensorConfig {
    int id = -1; // -1 означает невалидный ID
    Quadrilateral zone;
    MathCoreParams params;
};

struct GlobalConfig {
    int mainIntervalMsec = 0;
    int nestedIntervalMsec = 0;
    std::vector<SensorConfig> sensors;

    // Полезно вызвать перед заполнением новыми данными с сервера
    void clear() {
        sensors.clear();
        mainIntervalMsec = 0;
        nestedIntervalMsec = 0;
    }
};
