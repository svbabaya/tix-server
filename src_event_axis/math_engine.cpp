#include "math_engine.hpp"
#include "capturehandler.hpp"
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
    TraffCounter traffixCounter; 

    if (!capturer.open(FRAME_WIDTH, FRAME_HEIGHT)) {
        syslog(LOG_ERR, "MathEngine: Capture open failed!");
        return;
    }

    syslog(LOG_NOTICE, "MathEngine: Processing started (SyncInterval: 1s)");

    while (true) {
        Frame frame = capturer.handle();
        
        if (!frame.empty()) {
            // ШАГ 1: Забираем актуальный контекст настроек (быстро)
            pthread_mutex_lock(&ctx->settings.lock);
            traffixCounter.updateSettings(ctx->settings);
            pthread_mutex_unlock(&ctx->settings.lock);

            // ШАГ 2: Обработка кадра и наполнение внутреннего состояния
            // Здесь происходит вся математика над RowMat
            traffixCounter.processFrame(frame);
            
            // ШАГ 3: Периодический сброс в глобальный контекст и логирование
            traffixCounter.syncResultsIfNeeded(ctx->results);

        } else {
            // Если кадр пустой — даем процессору MIPS "подышать"
            usleep(20000); 
        }
    }
}
