#include "math_engine.hpp"
#include "traffcounter.hpp"
#include "capture_factory.hpp" 
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

    TraffCounter traffCounter;

    auto capturer = CaptureFactory::create();

    // Сработает, если фабрика вернула nullptr
    // Проверка на nullptr обязательна, если камера не определена в сборке
    if (!capturer || !capturer->open(ctx->capParams)) {
        syslog(LOG_ERR, "MathEngine: Capture initialization failed (No implementation or open error)!");
        return;
    }
    
    syslog(LOG_NOTICE, "MathEngine: Processing started (Multi-Sensor Mode)");

    while (ctx->running.load()) {
        Frame frame = capturer->handle();
        
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

    // СЮДА поток попадет ТОЛЬКО после получения сигнала
    syslog(LOG_NOTICE, "MathEngine: Thread exiting gracefully.");

    // Обращение через -> (хотя unique_ptr сам закроет при выходе из функции)
    capturer->close(); // Явное закрытие ресурсов видео
}
