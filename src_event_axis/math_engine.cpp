#include "math_engine.hpp"
#include "traffcounter.hpp"
#include "capturehandler.hpp"
#include "algo_params.hpp"

#include <unistd.h>
#include <syslog.h>
#include <sys/time.h>
#include <cstring>

void* MathEngine::run_thread(void* arg) {
    AppContext* ctx = static_cast<AppContext*>(arg);
    processing_loop(ctx);
    return nullptr;
}

void MathEngine::processing_loop(AppContext* ctx) {
    CaptureY800 capturer;
    TraffCounter traffCounter; 

    // Используем константы разрешения из камеры Axis
    if (!capturer.open(FRAME_WIDTH, FRAME_HEIGHT)) {
        syslog(LOG_ERR, "MathEngine: Capture open failed!");
        return;
    }

    syslog(LOG_NOTICE, "MathEngine: Processing started (Multi-Sensor Mode)");

    while (true) {
        Frame frame = capturer.handle();
        
        if (!frame.empty()) {
            // ШАГ 1: Забираем актуальный снимок настроек (Snapshot)
            // Метод getSnapshot() сам внутри захватывает и отпускает мьютекс
            GlobalConfig currentCfg = ctx->algoSettings.getSnapshot();
            
            // ШАГ 2: Передаем весь конфиг (со списком сенсоров) в TraffCounter
            traffCounter.updateSettings(currentCfg);

            // ШАГ 3: Обработка кадра (теперь итерируется по всем сенсорам внутри)
            traffCounter.processFrame(frame);
            
            // ШАГ 4: Синхронизация результатов в AppContext под мьютексом результатов
            traffCounter.syncResultsIfNeeded(ctx->results);

        } else {
            // Разгрузка CPU MIPS при отсутствии кадров
            usleep(20000); 
        }
    }
}
